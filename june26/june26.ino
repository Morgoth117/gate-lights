#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS  100
#define OMIT_START 22
#define OMIT_END   8

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// -------------------- June / Lightning Bug theme palette --------------------
uint32_t juneColors[6];

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

// -------------------- Flutter particles restart flag --------------------
bool flutterRestart = true;

void setup() {
  strip.begin();
  strip.show();
  randomSeed(analogRead(A0));

  juneColors[0] = strip.Color(180, 255, 20);    // lightning bug yellow-green
  juneColors[1] = strip.Color(255, 220, 60);    // warm firefly gold
  juneColors[2] = strip.Color(80, 180, 255);    // light blue
  juneColors[3] = strip.Color(130, 60, 255);    // violet
  juneColors[4] = strip.Color(40, 255, 80);     // soft green
  juneColors[5] = strip.Color(220, 240, 255);   // cool moon white
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
    flutterRestart = true;
  }

  switch (currentEffect) {
    case 0:
      awesomeTwinkleJune();
      break;
    case 1:
      colorWipeJune(juneColors[currentWipeColorIndex], 65, 10000);
      break;
    case 2:
      flutterParticlesEffect(juneColors);
      break;
    case 3:
      setStaticColorsJune();
      break;
  }
}

// =====================
// SLOW ACCUMULATING TWINKLE — JUNE LIGHTNING BUG COLORS
// =====================
void awesomeTwinkleJune() {
  static unsigned long lastSpawn = 0;
  static unsigned long lastFade  = 0;

  // Slower and gentler than the Cinco version
  const unsigned long spawnInterval = 280;
  const unsigned long fadeInterval  = 85;

  unsigned long now = millis();
  bool updated = false;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;

  if (now - lastSpawn >= spawnInterval) {
    lastSpawn = now;

    int numNew = random(1, 4);

    for (int n = 0; n < numNew; n++) {
      int idx = random(usableStart, usableEnd);
      uint32_t c = juneColors[random(6)];

      // Most twinkles should feel like lightning bugs
      if (random(100) < 60) {
        c = juneColors[random(2)];
      }

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

      // Slower fade
      r = (uint8_t)((r * 235) / 255);
      g = (uint8_t)((g * 235) / 255);
      b = (uint8_t)((b * 235) / 255);

      if (r < 3 && g < 3 && b < 3) r = g = b = 0;

      strip.setPixelColor(i, strip.Color(r, g, b));
    }

    updated = true;
  }

  if (updated) strip.show();
}

// =====================
// COLOR WIPE WITH PER-COLOR HOLD — JUNE COLORS
// =====================
void colorWipeJune(uint32_t color, int speed, unsigned long holdMs) {
  unsigned long currentMillis = millis();

  if (wipeHolding) {
    if (currentMillis - wipeHoldStart >= holdMs) {
      wipeHolding = false;
      wipeIndex = OMIT_START;
      currentWipeColorIndex = (currentWipeColorIndex + 1) % 6;
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
// CENTER SLOW FLUTTER PARTICLES
// Particles appear near center/apex, then drift all the way toward each end.
// =====================
void flutterParticlesEffect(uint32_t themeColors[]) {
  const int MAX_PARTICLES = 18;

  static bool active[MAX_PARTICLES];
  static float pos[MAX_PARTICLES];
  static float vel[MAX_PARTICLES];
  static int dir[MAX_PARTICLES];
  static int brightness[MAX_PARTICLES];
  static uint32_t color[MAX_PARTICLES];

  static unsigned long lastUpdate = 0;
  static unsigned long lastSpawn = 0;

  if (flutterRestart) {
    flutterRestart = false;

    for (int i = 0; i < MAX_PARTICLES; i++) {
      active[i] = false;
      pos[i] = 0;
      vel[i] = 0;
      dir[i] = 1;
      brightness[i] = 0;
      color[i] = 0;
    }

    strip.clear();
    strip.show();

    lastUpdate = millis();
    lastSpawn = millis();
  }

  int center = OMIT_START + APEX_OFFSET_FROM_OMIT_START;

  int usableMin = OMIT_START;
  int usableMax = NUMPIXELS - OMIT_END - 1;

  if (center < usableMin) center = usableMin;
  if (center > usableMax) center = usableMax;

  unsigned long now = millis();

  // Spawn new particles near center, slower than before
  if (now - lastSpawn >= 350) {
    lastSpawn = now;

    int spawnCount = random(1, 3);

    for (int s = 0; s < spawnCount; s++) {
      for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!active[i]) {
          active[i] = true;

          pos[i] = center + random(-1, 2);

          dir[i] = random(2) == 0 ? -1 : 1;

          // Much slower outward drift
          vel[i] = random(2, 6) / 10.0;   // 0.2 to 0.5 pixels per update

          // High enough to survive the full trip
          brightness[i] = 255;

          // Mostly firefly colors, occasional blue/violet
          if (random(100) < 65) {
            color[i] = themeColors[random(2)];
          } else {
            color[i] = themeColors[random(6)];
          }

          break;
        }
      }
    }
  }

  // Slower update rate
  if (now - lastUpdate >= 120) {
    lastUpdate = now;

    strip.clear();

    for (int i = 0; i < MAX_PARTICLES; i++) {
      if (!active[i]) continue;

      // Gentle flutter: tiny wiggle, mostly outward travel
      float flutter = random(-3, 4) / 10.0;   // -0.3 to +0.3

      // Make sure it still generally travels outward
      pos[i] += (vel[i] * dir[i]) + flutter;

      // Tiny pause/float moment
      if (random(100) < 10) {
        pos[i] -= dir[i] * 0.2;
      }

      int pixel = (int)(pos[i] + 0.5);

      // Let particles stay alive until they physically leave the usable strip
      if (pixel < usableMin || pixel > usableMax) {
        active[i] = false;
        continue;
      }

      // Fade based on distance from center, not time.
      // This keeps them visible until they reach the ends.
      int distanceToEnd;

      if (dir[i] < 0) {
        distanceToEnd = center - usableMin;
        brightness[i] = map(pixel, center, usableMin, 255, 60);
      } else {
        distanceToEnd = usableMax - center;
        brightness[i] = map(pixel, center, usableMax, 255, 60);
      }

      if (brightness[i] < 60) brightness[i] = 60;
      if (brightness[i] > 255) brightness[i] = 255;

      uint32_t c = color[i];
      uint8_t r = (c >> 16) & 0xFF;
      uint8_t g = (c >> 8)  & 0xFF;
      uint8_t b =  c        & 0xFF;

      uint8_t rr = r * brightness[i] / 255;
      uint8_t gg = g * brightness[i] / 255;
      uint8_t bb = b * brightness[i] / 255;

      strip.setPixelColor(pixel, strip.Color(rr, gg, bb));

      // Soft tail/glow behind the particle
      int tailPixel = pixel - dir[i];

      if (tailPixel >= usableMin && tailPixel <= usableMax) {
        strip.setPixelColor(
          tailPixel,
          strip.Color(rr / 4, gg / 4, bb / 4)
        );
      }
    }

    strip.show();
  }
}

// =====================
// STATIC JUNE COLORS
// Firefly / gold / light blue / violet / green / moon white repeating pattern
// =====================
void setStaticColorsJune() {
  if (staticApplied) return;
  staticApplied = true;

  int usableStart = OMIT_START;
  int usableEnd   = NUMPIXELS - OMIT_END;

  strip.clear();

  for (int i = usableStart; i < usableEnd; i++) {
    int m = (i - usableStart) % 6;
    strip.setPixelColor(i, juneColors[m]);
  }

  strip.show();
}