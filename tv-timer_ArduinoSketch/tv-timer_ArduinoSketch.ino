
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "Shortening"
#define STAPSK "ThiaIsAwesome"
#endif

const char* ssid = STASSID;  // your network SSID (name)
const char* pass = STAPSK;   // your network password

unsigned int localPort = 2390;  // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
// IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP;  // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48;  // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(28, 2, NEO_GRB + NEO_KHZ800);

static const uint8_t indexOfSegmentA = 0;
static const uint8_t indexOfSegmentB = 5;
static const uint8_t indexOfSegmentC = 4;
static const uint8_t indexOfSegmentD = 3;
static const uint8_t indexOfSegmentE = 2;
static const uint8_t indexOfSegmentF = 1;
static const uint8_t indexOfSegmentG = 6;

typedef struct {
  bool a : 1;
  bool b : 1;
  bool c : 1;
  bool d : 1;
  bool e : 1;
  bool f : 1;
  bool g : 1;
} DigitMask_t;

enum
{
  BlankMask = 10,
};

static const DigitMask_t masks[11] = {
  //a  b  c  d  e  f  g
  { 1, 1, 1, 1, 1, 1, 0, }, // 0
  { 0, 1, 1, 0, 0, 0, 0, }, // 1
  { 1, 1, 0, 1, 1, 0, 1, }, // 2
  { 1, 1, 1, 1, 0, 0, 1, }, // 3
  { 0, 1, 1, 0, 0, 1, 1, }, // 4
  { 1, 0, 1, 1, 0, 1, 1, }, // 5
  { 1, 0, 1, 1, 1, 1, 1, }, // 6
  { 1, 1, 1, 0, 0, 0, 0, }, // 7
  { 1, 1, 1, 1, 1, 1, 1, }, // 8
  { 1, 1, 1, 1, 0, 1, 1, }, // 9
  { 0, 0, 0, 0, 0, 0, 0, }, // blank
};

void setup() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  Serial.begin(115200);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void setDigit(uint32_t color, uint8_t startingOffset, DigitMask_t mask)
{
  if(mask.a) { pixels.setPixelColor(startingOffset + indexOfSegmentA, color); }
  if(mask.b) { pixels.setPixelColor(startingOffset + indexOfSegmentB, color); }
  if(mask.c) { pixels.setPixelColor(startingOffset + indexOfSegmentC, color); }
  if(mask.d) { pixels.setPixelColor(startingOffset + indexOfSegmentD, color); }
  if(mask.e) { pixels.setPixelColor(startingOffset + indexOfSegmentE, color); }
  if(mask.f) { pixels.setPixelColor(startingOffset + indexOfSegmentF, color); }
  if(mask.g) { pixels.setPixelColor(startingOffset + indexOfSegmentG, color); }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123);  // NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void loop() {
  // Time code, reusing esp8266 NTP example code with added logic for parallel operation with display
  static uint32_t lastRun = 0;
  static bool requestSent = false;
  static bool responseReceived = false;
  uint32_t now = millis();

  static uint8_t hour;
  static uint8_t min;

  if(now - lastRun > 1000 & !requestSent)
  {
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);
    sendNTPpacket(timeServerIP);  // send an NTP packet to a time server
    Serial.println("Request Sent");
    requestSent = true;
  }
  else if(now - lastRun > 2000 & !responseReceived)
  {
    int cb = udp.parsePacket();
    if (!cb) {
      Serial.println("no packet yet");
    } else {
      responseReceived = true;
      Serial.print("packet received at ");
      Serial.print(now - lastRun);
      Serial.print(" length = ");
      Serial.println(cb);
      // We've received a packet, read the data from it
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer

      // the timestamp starts at byte 40 of the received packet and is four bytes,
      //  or two words, long. First, esxtract the two words:

      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      // combine the four bytes (two words) into a long integer
      // this is NTP time (seconds since Jan 1 1900):
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      Serial.print("Seconds since Jan 1 1900 = ");
      Serial.println(secsSince1900);

      // now convert NTP time into everyday time:
      Serial.print("Unix time = ");
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;
      // print Unix time:
      Serial.println(epoch);

      // Adjust for timezone UTC-4
      epoch -= 4 * 3600;

      // print the hour, minute and second:
      Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
      hour = (epoch % 86400L) / 3600;
      Serial.print(hour);  // print the hour (86400 equals secs per day)
      Serial.print(':');
      min = (epoch % 3600) / 60;
      if (min < 10) {
        // In the first 10 minutes of each hour, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.print(min);  // print the minute (3600 equals secs per minute)
      Serial.print(':');
      if ((epoch % 60) < 10) {
        // In the first 10 seconds of each minute, we'll want a leading '0'
        Serial.print('0');
      }
      Serial.println(epoch % 60);  // print the second
    }
  }
  
  if(now - lastRun > 10000)
  {
    lastRun = now;
    requestSent = false;
    responseReceived = false;
  }

  // Custom Display Code
  static int hue = 0;

  hue += 2;
  hue = hue > 255 ? hue - 255 : hue;
  // Serial.printf("%d  ", hue);

  pixels.clear(); // Set all pixel colors to 'off'

  uint32_t color = pixels.ColorHSV((hue << 8), 255, 255);

  // Convert 24 hour to 12 hour
  hour = hour > 12 ? hour - 12 : hour;
  hour = hour == 0 ? 12 : hour;

  // Serial.printf("Hour %d min %d\r\n", hour, min);
  uint8_t dig4 = hour / 10;
  uint8_t dig3 = hour % 10;
  uint8_t dig2 = min / 10;
  uint8_t dig1 = min % 10;

  // No leading 0 on hours
  dig4 = dig4 == 0 ? BlankMask : dig4;

  setDigit(color, 0, masks[dig1]);
  setDigit(color, 7, masks[dig2]);
  setDigit(color, 16 - 2, masks[dig3]);
  setDigit(color, 23 - 2, masks[dig4]);

  pixels.show();   // Send the updated pixel colors to the hardware.

  delay(100); // Pause before next pass through loop
}
