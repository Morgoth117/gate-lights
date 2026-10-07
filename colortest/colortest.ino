#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS 100
#define OMIT_START 22
#define OMIT_END   1

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

int usableStart = OMIT_START;
int usableEnd   = NUMPIXELS - OMIT_END; // exclusive

void setup() {
  strip.begin();
  strip.setBrightness(80);
  strip.clear();
  strip.show();
}

void loop() {
  showColor(strip.Color(255, 0, 0), 3000);     // Should be RED
  showColor(strip.Color(0, 255, 0), 3000);     // Should be GREEN
  showColor(strip.Color(0, 0, 255), 3000);     // Should be BLUE
  showColor(strip.Color(255, 255, 0), 3000);   // Should be YELLOW
  showColor(strip.Color(255, 80, 0), 3000);    // Should be ORANGE/GOLD
  showColor(strip.Color(255, 255, 255), 3000); // Should be WHITE

  strip.clear();
  strip.show();
  delay(2000);
}

void showColor(uint32_t color, unsigned long holdMs) {
  strip.clear();

  for (int i = usableStart; i < usableEnd; i++) {
    strip.setPixelColor(i, color);
  }

  strip.show();
  delay(holdMs);
}