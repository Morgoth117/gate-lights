#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS  100
#define OMIT_START 22
#define OMIT_END   5

// IMPORTANT: using RGB order here as requested
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// -------------------- Cinco de Mayo theme palette --------------------
// Mexican flag + festive gold
uint32_t cincoColors[4];

// Apex you found: ~31–32 past OMIT_START. Start with 32.
#define APEX_OFFSET_FROM_OMIT_START 32

// -------------------- Effect cycling --------------------
unsigned long previousMillis = 0;
unsigned long effectDuration = 300000;  // 5 minutes per effect
unsigned int currentEffect = 0;

// -------------------- Color wipe state --------------------
unsigned long wipePreviousMillis = 0;
int wipeIndex = OMIT_START;
int currentWipeColorIndex = 0;

bool wipeHolding = false;
unsigned long wipeHoldStart = 0;

// -------------------- Static colors state --------------------
bool staticApplied = false;

// -------------------- Fireworks restart flag --------------------
bool fwRestart = true;

void setup() {
  strip.begin();
  strip.show();
  randomSeed(analogRead(A0));

  cincoColors[0] = strip.Color(0, 180, 0);       // green
  cincoColors[1] = strip.Color(255, 255, 0);   // Should be YELLOW
  cincoColors[2] = strip.Color(255, 0, 0);       // true red
  cincoColors[3] = strip.Color(255, 60, 0);      // orange/gold
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= effectDuration) {
    previousMillis = currentMillis;
    currentEffect = (currentEffect + 1) % 4;

    wipeIndex = OMIT_START;
    currentWipeColorIndex = 0;
    wipePreviousMillis = currentMillis;
    wipeHolding = false;
    wipeHoldStart = 0;

    staticApplied = false;
    fwRestart = true;
  }

  switch (currentEffect) {
    case 0:
      awesomeTwinkleCinco();
      break;
    case 1:
      colorWipeCinco(cincoColors[currentWipeColorIndex], 50, 10000);
      break;
    case 2:
      fireworksEffect(cincoColors);
      break;
    case 3:
      setStaticColorsCinco();
      break;
  }
}

// =====================
// ACCUMULATING AWESOME TWINKLE — CINCO COLORS
// =====================
void awesomeTwinkleCinco() {
  static unsigned long lastSpawn = 0;
  static unsigned long lastFade  = 0;

  const unsigned long spawnInterval = 120;
  const unsigned long fadeInterval  = 50;

  unsigned long now = millis();
  bool updated = false;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;

  if (now - lastSpawn >= spawnInterval) {
    lastSpawn = now;

    int numNew = random(3, 8);
    for (int n = 0; n < numNew; n++) {
      int idx = random(usableStart, usableEnd);
      uint32_t c = cincoColors[random(4)];
      strip.setPixelColor(idx, c);
    }
    updated = true;
  }

  if (now - lastFade >= fadeInterval) {
    lastFade = now;

    for (int i = usableStart; i < usableEnd; i++) {
      uint32_t c = strip.getPixelColor(i);
      uint8_t r = (c >> 16) & 0xFF;
      uint8_t g = (c >> 8)  & 0xFF;
      uint8_t b =  c        & 0xFF;

      if (r == 0 && g == 0 && b == 0) continue;

      r = (uint8_t)((r * 220) / 255);
      g = (uint8_t)((g * 220) / 255);
      b = (uint8_t)((b * 220) / 255);

      if (r < 3 && g < 3 && b < 3) r = g = b = 0;

      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    updated = true;
  }

  if (updated) strip.show();
}

// =====================
// COLOR WIPE WITH PER-COLOR HOLD — CINCO COLORS
// =====================
void colorWipeCinco(uint32_t color, int speed, unsigned long holdMs) {
  unsigned long currentMillis = millis();

  if (wipeHolding) {
    if (currentMillis - wipeHoldStart >= holdMs) {
      wipeHolding = false;
      wipeIndex = OMIT_START;
      currentWipeColorIndex = (currentWipeColorIndex + 1) % 4;
    }
    return;
  }

  if (currentMillis - wipePreviousMillis >= (unsigned long)speed) {
    wipePreviousMillis = currentMillis;

    if (wipeIndex < NUMPIXELS - OMIT_END) {
      strip.setPixelColor(wipeIndex, color);
      strip.show();
      wipeIndex++;
    } else {
      wipeHolding = true;
      wipeHoldStart = currentMillis;
    }
  }
}

// =====================
// FIREWORKS
// Uses Cinco palette for explosion/trickle
// =====================
void fireworksEffect(uint32_t themeColors[]) {
  static int launchPosition = OMIT_START;
  static bool exploding = false;
  static bool fading = false;
  static bool trickling = false;
  static unsigned long lastUpdate = 0;
  static int explosionRadius = 0;
  static uint32_t explosionColors[15];
  static int fadeStep = 255;
  static int trickleIndex = 0;
  static int trickleFade[NUMPIXELS];
  static bool startNewFirework = true;
  static bool launchFromStart = true;

  if (fwRestart) {
    fwRestart = false;
    startNewFirework = true;
    exploding = fading = trickling = false;
    lastUpdate = 0;
    explosionRadius = 0;
    fadeStep = 255;
    trickleIndex = 0;
    memset(trickleFade, 0, sizeof(trickleFade));
    strip.clear();
    strip.show();
  }

  int center = OMIT_START + APEX_OFFSET_FROM_OMIT_START;

  int usableMin = OMIT_START;
  int usableMax = NUMPIXELS - OMIT_END - 1;
  if (center < usableMin) center = usableMin;
  if (center > usableMax) center = usableMax;

  unsigned long currentMillis = millis();

  if (startNewFirework) {
    startNewFirework = false;
    exploding = false;
    fading = false;
    trickling = false;
    trickleIndex = 0;
    explosionRadius = 0;
    fadeStep = 255;

    memset(trickleFade, 0, sizeof(trickleFade));

    if (random(2) == 0) {
      launchFromStart = true;
      launchPosition = OMIT_START;
    } else {
      launchFromStart = false;
      launchPosition = NUMPIXELS - OMIT_END - 1;
    }

    strip.clear();
    strip.show();
  }

  if (!exploding && !fading && !trickling) {
    if (currentMillis - lastUpdate > 50) {
      if (launchFromStart) {
        if (launchPosition > OMIT_START) {
          strip.setPixelColor(launchPosition - 1, 0);
        }

        // Gold launch spark
        strip.setPixelColor(launchPosition, strip.Color(255, 140, 20));
        launchPosition++;
      } else {
        if (launchPosition < NUMPIXELS - OMIT_END - 1) {
          strip.setPixelColor(launchPosition + 1, 0);
        }

        // Gold launch spark
        strip.setPixelColor(launchPosition, strip.Color(255, 140, 20));
        launchPosition--;
      }

      strip.show();
      lastUpdate = currentMillis;

      if ((launchFromStart && launchPosition >= center) ||
          (!launchFromStart && launchPosition <= center)) {
        exploding = true;

        for (int i = 0; i < 15; i++) {
          explosionColors[i] = themeColors[random(4)];
        }
      }
    }
  } else if (exploding) {
    if (currentMillis - lastUpdate > 100) {
      for (int i = 0; i <= explosionRadius; i++) {
        if ((center + i) < NUMPIXELS - OMIT_END)
          strip.setPixelColor(center + i, explosionColors[i % 15]);
        if ((center - i) >= OMIT_START)
          strip.setPixelColor(center - i, explosionColors[i % 15]);
      }

      strip.show();
      lastUpdate = currentMillis;
      explosionRadius++;

      if (explosionRadius > 15) {
        exploding = false;
        trickling = true;
        trickleIndex = 0;
      }
    }
  } else if (trickling) {
    if (currentMillis - lastUpdate > 50) {
      if (center + trickleIndex < NUMPIXELS - OMIT_END) {
        trickleFade[center + trickleIndex] = 255;
      }
      if (center - trickleIndex >= OMIT_START) {
        trickleFade[center - trickleIndex] = 255;
      }

      for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
        if (trickleFade[i] > 0) {
          uint32_t color = themeColors[random(4)];
          uint8_t r = (color >> 16) & 0xFF;
          uint8_t g = (color >> 8) & 0xFF;
          uint8_t b = color & 0xFF;

          strip.setPixelColor(
            i,
            strip.Color(
              r * trickleFade[i] / 255,
              g * trickleFade[i] / 255,
              b * trickleFade[i] / 255
            )
          );

          trickleFade[i] -= 25;
        } else {
          strip.setPixelColor(i, 0);
        }
      }

      strip.show();
      trickleIndex++;

      if (center + trickleIndex >= NUMPIXELS - OMIT_END &&
          center - trickleIndex < OMIT_START) {
        trickling = false;
        fading = true;
        fadeStep = 255;
      }

      lastUpdate = currentMillis;
    }
  } else if (fading) {
    if (currentMillis - lastUpdate > 50) {
      for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
        uint32_t color = strip.getPixelColor(i);
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        strip.setPixelColor(
          i,
          strip.Color(
            r * fadeStep / 255,
            g * fadeStep / 255,
            b * fadeStep / 255
          )
        );
      }

      strip.show();
      fadeStep -= 25;

      if (fadeStep <= 0) {
        fading = false;
        startNewFirework = true;
      }

      lastUpdate = currentMillis;
    }
  }
}

// =====================
// STATIC CINCO COLORS
// Green / white / red / gold repeating pattern
// =====================
void setStaticColorsCinco() {
  if (staticApplied) return;
  staticApplied = true;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;

  strip.clear();

  for (int i = usableStart; i < usableEnd; i++) {
    int m = (i - usableStart) % 4;
    strip.setPixelColor(i, cincoColors[m]);
  }

  strip.show();
}