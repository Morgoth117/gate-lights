#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS  200
#define OMIT_START 28
#define OMIT_END   37

// IMPORTANT: using RGB order here as requested
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// ---- Apex calibration (from your test) ----
// You said apex is 31–32 past OMIT_START.
// Try 32 first. If it's one LED off, change to 31.
#define APEX_OFFSET_FROM_OMIT_START 32

void setup() {
  strip.begin();
  strip.show();
  randomSeed(analogRead(A0));
}

void loop() {
  static uint32_t dummyTheme[1] = { 0 }; // function signature requires an array
  fireworksEffect(dummyTheme);
}

// ===================================================================
// YOUR ORIGINAL FUNCTION — ONLY CHANGE IS center IS FIXED TO APEX
// ===================================================================
void fireworksEffect(uint32_t themeColors[]) {
  static int launchPosition = OMIT_START; // Start position of the firework
  static bool exploding = false;         // Flag to track if it's exploding
  static bool fading = false;            // Flag to track if it's fading
  static bool trickling = false;         // Flag to track if it's trickling
  static unsigned long lastUpdate = 0;   // Timing for updates
  static int explosionRadius = 0;        // Radius of the explosion
  static uint32_t explosionColors[15];   // Colors for the explosion
  static int fadeStep = 255;             // Current fade brightness level
  static int trickleIndex = 0;           // Current trickle LED index
  static int trickleFade[NUMPIXELS];     // Array to track fade levels for trickle LEDs
  static bool startNewFirework = true;   // Flag to initialize a new firework
  static bool launchFromStart = true;    // Launch direction (start or end)

  // FIXED CENTER: physical apex you found
  int center = OMIT_START + APEX_OFFSET_FROM_OMIT_START;

  // Safety clamp to keep center inside usable zone
  int usableMin = OMIT_START;
  int usableMax = NUMPIXELS - OMIT_END - 1;
  if (center < usableMin) center = usableMin;
  if (center > usableMax) center = usableMax;

  unsigned long currentMillis = millis();

  // Initialize a new firework
  if (startNewFirework) {
    startNewFirework = false; // Prevent reinitializing
    exploding = false;
    fading = false;
    trickling = false;
    trickleIndex = 0;
    explosionRadius = 0;
    fadeStep = 255;

    // Reset trickle fade array
    memset(trickleFade, 0, sizeof(trickleFade));

    // Randomize launch side: start or end
    if (random(2) == 0) {
      launchFromStart = true;
      launchPosition = OMIT_START; // Launch from the start
    } else {
      launchFromStart = false;
      launchPosition = NUMPIXELS - OMIT_END - 1; // Launch from the end
    }

    strip.clear(); // Clear the strip
    strip.show();
  }

  if (!exploding && !fading && !trickling) {
    // Launching phase
    if (currentMillis - lastUpdate > 50) { // Adjust speed of launch
      if (launchFromStart) {
        // Moving upward from the start
        if (launchPosition > OMIT_START) {
          strip.setPixelColor(launchPosition - 1, 0); // Turn off previous LED
        }
        strip.setPixelColor(launchPosition, strip.Color(255, 165, 0)); // Orange-yellow firework pixel
        launchPosition++;
      } else {
        // Moving downward from the end
        if (launchPosition < NUMPIXELS - OMIT_END - 1) {
          strip.setPixelColor(launchPosition + 1, 0); // Turn off previous LED
        }
        strip.setPixelColor(launchPosition, strip.Color(255, 165, 0)); // Orange-yellow firework pixel
        launchPosition--;
      }

      strip.show();
      lastUpdate = currentMillis;

      // Check if the firework reached the center
      if ((launchFromStart && launchPosition >= center) ||
          (!launchFromStart && launchPosition <= center)) {
        exploding = true;

        // Generate random explosion colors
        for (int i = 0; i < 15; i++) {
          explosionColors[i] = strip.Color(random(256), random(256), random(256)); // Random RGB colors
        }
      }
    }
  } else if (exploding) {
    // Explosion phase
    if (currentMillis - lastUpdate > 100) { // Adjust explosion expansion speed
      for (int i = 0; i <= explosionRadius; i++) {
        if ((center + i) < NUMPIXELS - OMIT_END)
          strip.setPixelColor(center + i, explosionColors[i % 15]);
        if ((center - i) >= OMIT_START)
          strip.setPixelColor(center - i, explosionColors[i % 15]);
      }
      strip.show();
      lastUpdate = currentMillis;
      explosionRadius++;

      // Check if the explosion has reached its maximum size
      if (explosionRadius > 15) {
        exploding = false;
        trickling = true; // Start trickling phase
        trickleIndex = 0; // Reset trickle index
      }
    }
  } else if (trickling) {
    // Trickling phase
    if (currentMillis - lastUpdate > 50) { // Adjust trickling speed
      // Add new trickle LEDs at the current index
      if (center + trickleIndex < NUMPIXELS - OMIT_END) {
        trickleFade[center + trickleIndex] = 255; // Start at full brightness
      }
      if (center - trickleIndex >= OMIT_START) {
        trickleFade[center - trickleIndex] = 255; // Start at full brightness
      }

      // Update all trickle LEDs with fading effect
      for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
        if (trickleFade[i] > 0) {
          uint32_t color = strip.Color(random(256), random(256), random(256)); // Random colors
          uint8_t r = (color >> 16) & 0xFF;
          uint8_t g = (color >> 8) & 0xFF;
          uint8_t b = color & 0xFF;
          strip.setPixelColor(i, strip.Color(r * trickleFade[i] / 255, g * trickleFade[i] / 255, b * trickleFade[i] / 255));
          trickleFade[i] -= 25; // Fade out
        } else {
          strip.setPixelColor(i, 0); // Turn off LED when fully faded
        }
      }

      strip.show();
      trickleIndex++;

      // Check if trickling is complete
      if (center + trickleIndex >= NUMPIXELS - OMIT_END && center - trickleIndex < OMIT_START) {
        trickling = false;
        fading = true; // Start fading phase
        fadeStep = 255; // Reset fade step
      }
      lastUpdate = currentMillis;
    }
  } else if (fading) {
    // Fading phase
    if (currentMillis - lastUpdate > 50) { // Adjust fade speed
      for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
        uint32_t color = strip.getPixelColor(i);
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        strip.setPixelColor(i, strip.Color(r * fadeStep / 255, g * fadeStep / 255, b * fadeStep / 255));
      }
      strip.show();
      fadeStep -= 25; // Reduce brightness

      if (fadeStep <= 0) {
        fading = false;        // End fading
        startNewFirework = true;
      }
      lastUpdate = currentMillis;
    }
  }
}
