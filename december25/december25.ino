#include <Adafruit_NeoPixel.h>

#define PIN        6      // Pin where the NeoPixel strip is connected
#define NUMPIXELS  200    // Number of pixels in your NeoPixel strip
#define OMIT_START 28    // Number of LEDs to omit from the start
#define OMIT_END   37     // Number of LEDs to omit from the end

// IMPORTANT: using RGB order here as requested
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

unsigned long previousMillis = 0;       // Store the last time the effect was updated
unsigned long effectDuration = 300000;   // Duration of each effect in milliseconds (30 seconds)
unsigned int currentEffect = 0;         // To track the current effect

unsigned long wipePreviousMillis = 0;   // Store the last time the color wipe was updated
int wipeIndex = OMIT_START;            // Index to track the current LED in the color wipe
uint32_t wipeColors[3] = {
  // still RGB here
  strip.Color(255, 0, 0),   // Red
  strip.Color(0, 255, 0),   // Green
  strip.Color(0, 0, 255)    // Blue
};
int currentWipeColorIndex = 0;          // Index to track the current color in the wipe

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= effectDuration) {
    previousMillis = currentMillis;
    currentEffect = (currentEffect + 1) % 4; // Cycle through the effects
    wipeIndex = OMIT_START;                  // Reset wipe index when switching effects
    currentWipeColorIndex = 0;               // Reset wipe color index when switching effects
    wipePreviousMillis = currentMillis;      // Reset wipe timer to align with effect change
  }

  switch (currentEffect) {
    case 0:
      awesomeTwinkle(); // new accumulating twinkle
      break;
    case 1:
      //           color,                      pixel speed, hold time in ms
      colorWipe(wipeColors[currentWipeColorIndex], 50,      10000); // 6s hold
      break;
    case 2:
      // Candy cane: red + bluish white, slower crawl
      chasingEffect(
        strip.Color(255, 0, 0),       // Red (RGB)
        strip.Color(180, 220, 255),   // Bluish white
        400                           // Slow crawl
      );
      break;
    case 3:
      setStaticColors(); // Static Christmas colors
      break;
  }
}

// =====================
// ACCUMULATING AWESOME TWINKLE (NON-BLOCKING)
// =====================
void awesomeTwinkle() {
  static unsigned long lastSpawn = 0;
  static unsigned long lastFade  = 0;

  const unsigned long spawnInterval = 120; // time between new twinkle bursts
  const unsigned long fadeInterval  = 50;  // time between fade steps

  unsigned long now = millis();
  bool updated = false;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;

  // --- Spawn new twinkles periodically ---
  if (now - lastSpawn >= spawnInterval) {
    lastSpawn = now;

    int numNew = random(3, 8); // how many fresh twinkles per spawn
    for (int n = 0; n < numNew; n++) {
      int idx = random(usableStart, usableEnd);
      int randomColor = random(3);

      uint32_t c;
      switch (randomColor) {
        case 0:
          c = strip.Color(255, 0, 0);   // Red
          break;
        case 1:
          c = strip.Color(0, 255, 0);   // Green
          break;
        default:
          c = strip.Color(0, 0, 255);   // Blue
          break;
      }

      strip.setPixelColor(idx, c);
    }

    updated = true;
  }

  // --- Fade all lit pixels down a bit periodically ---
  if (now - lastFade >= fadeInterval) {
    lastFade = now;

    for (int i = usableStart; i < usableEnd; i++) {
      uint32_t c = strip.getPixelColor(i);
      uint8_t r = (c >> 16) & 0xFF;
      uint8_t g = (c >> 8)  & 0xFF;
      uint8_t b =  c        & 0xFF;

      if (r == 0 && g == 0 && b == 0) continue; // already off

      // Fade each channel
      r = (uint8_t)((r * 220) / 255);
      g = (uint8_t)((g * 220) / 255);
      b = (uint8_t)((b * 220) / 255);

      // Hard floor so they don't hang dim forever
      if (r < 3 && g < 3 && b < 3) {
        r = g = b = 0;
      }

      strip.setPixelColor(i, strip.Color(r, g, b));
    }

    updated = true;
  }

  if (updated) {
    strip.show();
  }
}

// =====================
// COLOR WIPE (unchanged logic)
// =====================
// =====================
// COLOR WIPE WITH PER-COLOR HOLD
// =====================
void colorWipe(uint32_t color, int speed, unsigned long holdMs) {
  static bool holding = false;
  static unsigned long holdStart = 0;

  unsigned long currentMillis = millis();

  // If we've finished a wipe and we're in the "hold" phase
  if (holding) {
    if (currentMillis - holdStart >= holdMs) {
      // Done holding this color → move to next color, reset for next wipe
      holding = false;
      wipeIndex = OMIT_START;
      currentWipeColorIndex = (currentWipeColorIndex + 1) % 3;
    }
    // While holding, do nothing else
    return;
  }

  // Normal wipe behavior
  if (currentMillis - wipePreviousMillis >= (unsigned long)speed) {
    wipePreviousMillis = currentMillis;

    if (wipeIndex < NUMPIXELS - OMIT_END) {
      strip.setPixelColor(wipeIndex, color); // Set color of the current LED
      strip.show();                          // Update the strip
      wipeIndex++;
    } else {
      // Reached the end: enter hold phase
      holding = true;
      holdStart = currentMillis;
    }
  }
}


// =====================
// CANDY CANE CHASING EFFECT
// =====================
void chasingEffect(uint32_t colorRed, uint32_t colorWhite, int wait) {
  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;
  int length      = usableEnd - usableStart;

  // Classic candy cane: blocks of red and bluish white moving along the strip
  for (int offset = 0; offset < length; offset++) {
    for (int i = usableStart; i < usableEnd; i++) {
      int pos = i - usableStart;
      // Pattern: 3 red, 3 white repeating, shifted by "offset"
      int patternIndex = (pos + offset) % 6;
      if (patternIndex < 3) {
        strip.setPixelColor(i, colorRed);
      } else {
        strip.setPixelColor(i, colorWhite);
      }
    }
    strip.show();
    delay(wait); // now 400ms
  }
}

// =====================
// STATIC CHRISTMAS COLORS (NO YELLOW)
// =====================
void setStaticColors() {
  // Clear everything so no leftover colors
  strip.clear();

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;

  // Cycle: Red, Blue, Green, Orange (no yellow)
  for (int i = usableStart; i < usableEnd; i++) {
    int m = (i - usableStart) % 4; // 0–3
    if (m == 0) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));     // Red
    } else if (m == 1) {
      strip.setPixelColor(i, strip.Color(0, 0, 255));     // Blue
    } else if (m == 2) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));     // Green
    } else { // m == 3
      strip.setPixelColor(i, strip.Color(255, 165, 0));   // Orange
    }
  }

  strip.show(); // Update the strip

  // Hold this look for the full effect duration
  delay(effectDuration);
}
