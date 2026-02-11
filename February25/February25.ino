#include <Adafruit_NeoPixel.h>

#define PIN        6      // Pin where the NeoPixel strip is connected
#define NUMPIXELS  200    // Number of pixels in your NeoPixel strip
#define OMIT_START 28     // Number of LEDs to omit from the start
#define OMIT_END   37     // Number of LEDs to omit from the end

// IMPORTANT: using RGB order here as requested
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// -------------------- February theme palette --------------------
// Pink-forward Valentine palette
uint32_t febColors[4];

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

// Make these global so we can reset them when switching effects
bool wipeHolding = false;
unsigned long wipeHoldStart = 0;

// -------------------- Static colors state (non-blocking) --------------------
bool staticApplied = false;

// -------------------- Valentine burst restart flag --------------------
bool valRestart = true;

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(A0));

  // February colors (pink heavy)
  febColors[0] = strip.Color(255, 70, 150);  // Bright pink
  febColors[1] = strip.Color(255, 20, 95);   // Deep rose
  febColors[2] = strip.Color(255, 165, 200); // Soft pink
  febColors[3] = strip.Color(255, 240, 245); // Blush white
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= effectDuration) {
    previousMillis = currentMillis;
    currentEffect = (currentEffect + 1) % 4;

    // Reset wipe state when switching effects
    wipeIndex = OMIT_START;
    currentWipeColorIndex = 0;
    wipePreviousMillis = currentMillis;
    wipeHolding = false;
    wipeHoldStart = 0;

    // Reset static redraw
    staticApplied = false;

    // Restart valentine burst cleanly next time we enter it
    valRestart = true;
  }

  switch (currentEffect) {
    case 0:
      awesomeTwinkleFebruary();
      break;
    case 1:
      colorWipeFebruary(febColors[currentWipeColorIndex], 50, 10000); // hold per color
      break;
    case 2:
      valentineBurstEffect(febColors);
      break;
    case 3:
      setStaticColorsFebruary();
      break;
  }
}

// =====================
// ACCUMULATING AWESOME TWINKLE (NON-BLOCKING) — FEBRUARY COLORS
// =====================
void awesomeTwinkleFebruary() {
  static unsigned long lastSpawn = 0;
  static unsigned long lastFade  = 0;

  const unsigned long spawnInterval = 120;
  const unsigned long fadeInterval  = 50;

  unsigned long now = millis();
  bool updated = false;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END; // exclusive

  // Spawn new twinkles
  if (now - lastSpawn >= spawnInterval) {
    lastSpawn = now;

    int numNew = random(3, 8);
    for (int n = 0; n < numNew; n++) {
      int idx = random(usableStart, usableEnd);
      uint32_t c = febColors[random(4)];
      strip.setPixelColor(idx, c);
    }
    updated = true;
  }

  // Fade
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
// COLOR WIPE WITH PER-COLOR HOLD — FEBRUARY COLORS
// =====================
void colorWipeFebruary(uint32_t color, int speed, unsigned long holdMs) {
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

    if (wipeIndex < NUMPIXELS - OMIT_END) {   // exclusive end
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
// VALENTINE BURST (replaces fireworks)
// - launches pink comet to apex
// - does two "heartbeat" pulses
// - releases falling glitter in pink/rose tones
// =====================
void valentineBurstEffect(uint32_t themeColors[]) {
  static int launchPosition = OMIT_START;
  static bool launching = true;
  static bool pulsing = false;
  static bool glittering = false;
  static bool fading = false;

  static unsigned long lastUpdate = 0;
  static int pulseRadius = 0;
  static int pulseCount = 0;
  static int pulseBrightness = 255;

  static int glitterFade[NUMPIXELS];
  static int glitterLife = 0;

  static int fadeStep = 255;
  static bool startNewPattern = true;
  static bool launchFromStart = true;

  // Force a clean restart when we switch back to this effect
  if (valRestart) {
    valRestart = false;
    startNewPattern = true;
    launching = pulsing = glittering = fading = false;
    lastUpdate = 0;
    pulseRadius = 0;
    pulseCount = 0;
    pulseBrightness = 255;
    glitterLife = 0;
    fadeStep = 255;
    memset(glitterFade, 0, sizeof(glitterFade));
    strip.clear();
    strip.show();
  }

  // FIXED CENTER: physical apex you found
  int center = OMIT_START + APEX_OFFSET_FROM_OMIT_START;

  // Safety clamp to usable zone
  int usableMin = OMIT_START;
  int usableMax = NUMPIXELS - OMIT_END - 1;
  if (center < usableMin) center = usableMin;
  if (center > usableMax) center = usableMax;

  unsigned long currentMillis = millis();

  // Initialize a new valentine burst
  if (startNewPattern) {
    startNewPattern = false;
    launching = true;
    pulsing = false;
    glittering = false;
    fading = false;

    pulseRadius = 0;
    pulseCount = 0;
    pulseBrightness = 255;
    glitterLife = 0;
    fadeStep = 255;

    memset(glitterFade, 0, sizeof(glitterFade));

    // Randomize launch side: start or end
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

  if (launching) {
    // Comet launch phase
    if (currentMillis - lastUpdate > 45) {
      if (launchFromStart) {
        if (launchPosition > OMIT_START) {
          strip.setPixelColor(launchPosition - 1, strip.Color(80, 20, 40));
        }
        strip.setPixelColor(launchPosition, strip.Color(255, 80, 150));
        launchPosition++;
      } else {
        if (launchPosition < NUMPIXELS - OMIT_END - 1) {
          strip.setPixelColor(launchPosition + 1, strip.Color(80, 20, 40));
        }
        strip.setPixelColor(launchPosition, strip.Color(255, 80, 150));
        launchPosition--;
      }

      strip.show();
      lastUpdate = currentMillis;

      if ((launchFromStart && launchPosition >= center) ||
          (!launchFromStart && launchPosition <= center)) {
        launching = false;
        pulsing = true;
        pulseRadius = 0;
        pulseBrightness = 255;
      }
    }
  } else if (pulsing) {
    // Heartbeat pulse phase
    if (currentMillis - lastUpdate > 85) {
      strip.clear();

      // Draw expanding ring in valentine colors
      for (int i = 0; i <= pulseRadius; i++) {
        uint32_t baseColor = themeColors[(pulseCount + i) % 4];
        uint8_t r = (baseColor >> 16) & 0xFF;
        uint8_t g = (baseColor >> 8) & 0xFF;
        uint8_t b = baseColor & 0xFF;

        r = (uint8_t)((r * pulseBrightness) / 255);
        g = (uint8_t)((g * pulseBrightness) / 255);
        b = (uint8_t)((b * pulseBrightness) / 255);

        if ((center + i) < NUMPIXELS - OMIT_END) strip.setPixelColor(center + i, strip.Color(r, g, b));
        if ((center - i) >= OMIT_START)          strip.setPixelColor(center - i, strip.Color(r, g, b));
      }

      // Highlight center to mimic a heart beat pop
      strip.setPixelColor(center, strip.Color(255, 120, 180));
      strip.show();

      pulseRadius++;
      pulseBrightness = max(80, pulseBrightness - 20);

      if (pulseRadius > 12) {
        pulseCount++;
        if (pulseCount >= 2) {
          pulsing = false;
          glittering = true;
          glitterLife = 24;
          for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
            glitterFade[i] = 0;
          }
        } else {
          pulseRadius = 0;
          pulseBrightness = 255;
        }
      }

      lastUpdate = currentMillis;
    }
  } else if (glittering) {
    // Falling glitter phase (pink/rose tones)
    if (currentMillis - lastUpdate > 55) {
      // Seed glitter from center outward with slight randomness
      int spread = random(1, 5);
      for (int s = 0; s < spread; s++) {
        int offset = random(-16, 17);
        int idx = center + offset;
        if (idx >= OMIT_START && idx < NUMPIXELS - OMIT_END) {
          glitterFade[idx] = 255;
        }
      }

      for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
        if (glitterFade[i] > 0) {
          uint32_t c = themeColors[random(4)];
          uint8_t r = (c >> 16) & 0xFF;
          uint8_t g = (c >> 8) & 0xFF;
          uint8_t b = c & 0xFF;

          strip.setPixelColor(i, strip.Color((r * glitterFade[i]) / 255,
                                             (g * glitterFade[i]) / 255,
                                             (b * glitterFade[i]) / 255));
          glitterFade[i] -= 28;
          if (glitterFade[i] < 0) glitterFade[i] = 0;
        } else {
          strip.setPixelColor(i, 0);
        }
      }

      strip.show();
      glitterLife--;

      if (glitterLife <= 0) {
        glittering = false;
        fading = true;
        fadeStep = 255;
      }

      lastUpdate = currentMillis;
    }
  } else if (fading) {
    // Global fade out before restarting
    if (currentMillis - lastUpdate > 50) {
      for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
        uint32_t color = strip.getPixelColor(i);
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        strip.setPixelColor(i, strip.Color((r * fadeStep) / 255,
                                           (g * fadeStep) / 255,
                                           (b * fadeStep) / 255));
      }
      strip.show();
      fadeStep -= 25;

      if (fadeStep <= 0) {
        fading = false;
        startNewPattern = true;
      }
      lastUpdate = currentMillis;
    }
  }
}

// =====================
// STATIC FEBRUARY COLORS (NON-BLOCKING)
// Pink, Rose, Soft Pink, Blush White repeating
// =====================
void setStaticColorsFebruary() {
  if (staticApplied) return;
  staticApplied = true;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END; // exclusive

  strip.clear();

  for (int i = usableStart; i < usableEnd; i++) {
    int m = (i - usableStart) % 4;
    strip.setPixelColor(i, febColors[m]);
  }

  strip.show();
}
