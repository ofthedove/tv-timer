
#include <Adafruit_NeoPixel.h>

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

static const DigitMask_t masks[10] = {
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
};

int hue = 0;
int currentDigit = 0;

void setup() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  Serial.begin(115200);
}

void setDigit(uint32_t color, uint8_t digit, DigitMask_t mask)
{
  uint8_t startingOffset = digit * 7;
  if(mask.a) { pixels.setPixelColor(startingOffset + indexOfSegmentA, color); }
  if(mask.b) { pixels.setPixelColor(startingOffset + indexOfSegmentB, color); }
  if(mask.c) { pixels.setPixelColor(startingOffset + indexOfSegmentC, color); }
  if(mask.d) { pixels.setPixelColor(startingOffset + indexOfSegmentD, color); }
  if(mask.e) { pixels.setPixelColor(startingOffset + indexOfSegmentE, color); }
  if(mask.f) { pixels.setPixelColor(startingOffset + indexOfSegmentF, color); }
  if(mask.g) { pixels.setPixelColor(startingOffset + indexOfSegmentG, color); }
}

void loop() {
  currentDigit += 1;
  currentDigit = currentDigit == 10 ? 0 : currentDigit;
  Serial.printf("%d  ", currentDigit);

  hue += 10;
  hue = hue > 255 ? hue - 255 : hue;
  Serial.printf("%d  ", hue);

  pixels.clear(); // Set all pixel colors to 'off'

  uint32_t colorDim = pixels.ColorHSV((hue << 8), 255, 100);
  uint32_t colorBright = pixels.ColorHSV((hue << 8), 255, 255);
  setDigit(colorDim, 0, masks[currentDigit]);
  setDigit(colorDim, 1, masks[currentDigit]);
  setDigit(colorBright, 2, masks[currentDigit]);
  setDigit(colorBright, 3, masks[currentDigit]);

  // Serial.print("   ");
  // Serial.printf("%d", (color >> 24) & 0xFF);
  // Serial.print("   ");
  // Serial.printf("%d", (color >> 16) & 0xFF);
  // Serial.print("   ");
  // Serial.printf("%d", (color >> 8) & 0xFF);
  // Serial.print("   ");
  // Serial.printf("%d", (color >> 0) & 0xFF);
  // Serial.print("   ");
  // Serial.println();

  pixels.show();   // Send the updated pixel colors to the hardware.

  delay(500); // Pause before next pass through loop

  // for(int i=0; i<5; i++) { // For each pixel...

  //   // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
  //   // Here we're using a moderately bright green color:
  //   pixels.setPixelColor(i, pixels.Color(0, 150, 0));

  //   pixels.show();   // Send the updated pixel colors to the hardware.

  //   delay(300); // Pause before next pass through loop
  // }
}
