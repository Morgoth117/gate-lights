#include <Adafruit_NeoPixel.h>

#define PIN        7      // Pin where the NeoPixel strip is connected
#define NUMPIXELS  110    // Total number of pixels in your NeoPixel strip
#define OMIT_START 15     // Number of LEDs reserved for twinkling gold at the start
#define OMIT_END   0     // Number of LEDs reserved for twinkling gold at the end

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);


unsigned long previousMillis = 0;      // Last time the effect was updated
unsigned long effectDuration = 180000;   // Duration for each effect in milliseconds
unsigned int currentEffect = 0;         // To track the current effect


// -------------------------------------------------------------------
// Deep Fall Colours: Brown, Orange, Yellow, White
// -------------------------------------------------------------------
const uint32_t themeColors[] = {
  strip.Color( 80,  30,   0),   // Deep Brown
  strip.Color(255,  70,   0),   // Burnt Orange
  strip.Color(255, 140,   0),   // Golden Yellow
  strip.Color(255, 240, 220)    // Warm Soft White
};
const uint8_t COLOR_COUNT = sizeof(themeColors) / sizeof(themeColors[0]);


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  unsigned long currentMillis = millis();

  // Switch effects after effectDuration
  if (currentMillis - previousMillis >= effectDuration) {
    previousMillis = currentMillis;
    currentEffect = (currentEffect + 1) % 5; // Cycle through 6 effects
    strip.clear();       // Clear all pixels to prevent leftover colors
    strip.show();
  }

  // Execute the current effect
  switch (currentEffect) {
    case 0:
      awesomeTwinkle(themeColors);
      break;
    case 1:
      colorWipe(themeColors, 50);
      break;
    case 2:
      chasingEffect(themeColors, 150);
      break;
    case 3:
      setStaticColors(themeColors); // Static green theme colors
      break;
    case 4:
      rainbowRainEffect(); // Lightning flashes + rainbow raindrops
      break;

  
  }
}

//-----------------------------------------------------
// Twinkle Effect with Dynamic Fading using Green Theme
void awesomeTwinkle(uint32_t colors[]) {
  int numTwinkling = random(10, 20); // Number of LEDs to twinkle
  int twinklingPixels[numTwinkling];
  uint32_t twinklingColors[numTwinkling];
  int fadeSpeeds[numTwinkling]; // Each LED fades at a different speed

  // Select random pixels and assign colors
  for (int i = 0; i < numTwinkling; i++) {
    twinklingPixels[i] = random(OMIT_START, NUMPIXELS - OMIT_END);
    twinklingColors[i] = colors[random(COLOR_COUNT)]; // Pick a random green from the theme
    fadeSpeeds[i] = random(15, 40); // Varying fade speeds
    strip.setPixelColor(twinklingPixels[i], twinklingColors[i]);
  }
  strip.show();

  // Dynamic fade out loop
  for (int fade = 255; fade >= 0; fade -= 5) {
    for (int i = 0; i < numTwinkling; i++) {
      uint8_t r = (twinklingColors[i] >> 16) & 0xFF;
      uint8_t g = (twinklingColors[i] >> 8) & 0xFF;
      uint8_t b = twinklingColors[i] & 0xFF;
      
      int adjustedFade = max(0, fade - (fadeSpeeds[i] / 2));
      strip.setPixelColor(twinklingPixels[i], strip.Color(
        r * adjustedFade / 255, 
        g * adjustedFade / 255, 
        b * adjustedFade / 255
      ));
    }
    strip.show();
    delay(random(20, 50)); // Variable delay for a natural twinkle scatter
  }
}

//-----------------------------------------------------
// Color Wipe Effect with a Fading Tail
void colorWipe(uint32_t colors[], int baseSpeed) {
  unsigned long currentMillis = millis();
  static int currentColorIndex = 0;  // Track the current color in theme
  static int wipeIndex = 0;          // Current LED position for the wipe
  static int direction = 1;          // 1 = forward, -1 = reverse
  static bool initialized = false;   // Ensure one-time initialization
  static unsigned long wipePreviousMillis = 0;

  if (!initialized) {
    wipeIndex = random(OMIT_START, NUMPIXELS - OMIT_END); // Random start within active zone
    direction = random(2) == 0 ? 1 : -1; // Random direction
    initialized = true;
  }

  if (currentMillis - wipePreviousMillis >= baseSpeed + random(-10, 10)) {
    wipePreviousMillis = currentMillis;

    // Fading tail effect for previous pixels
    for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
      uint32_t originalColor = strip.getPixelColor(i);
      uint8_t r = (originalColor >> 16) & 0xFF;
      uint8_t g = (originalColor >> 8) & 0xFF;
      uint8_t b = originalColor & 0xFF;
      strip.setPixelColor(i, strip.Color(r * 0.8, g * 0.8, b * 0.8));
    }

    // Set current pixel to current theme color
    strip.setPixelColor(wipeIndex, colors[currentColorIndex]);
    strip.show();

    wipeIndex += direction;

    // Reset at edges with new random parameters
    if (wipeIndex <= OMIT_START || wipeIndex >= NUMPIXELS - OMIT_END) {
      wipeIndex = random(OMIT_START, NUMPIXELS - OMIT_END);
      direction = random(2) == 0 ? 1 : -1;
      currentColorIndex = (currentColorIndex + 1) % COLOR_COUNT;
    }
  }
}

//-----------------------------------------------------
// Chasing Effect with Fading Tails
void chasingEffect(uint32_t colors[], int speed) {
  static unsigned long previousChaseMillis = 0;
  static int offset = 0;

  unsigned long currentMillis = millis();
  if (currentMillis - previousChaseMillis >= speed) {
    previousChaseMillis = currentMillis;

    for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
      int colorIndex = (i + offset) % COLOR_COUNT;
      uint32_t currentColor = colors[colorIndex];
      uint8_t r = (currentColor >> 16) & 0xFF;
      uint8_t g = (currentColor >> 8) & 0xFF;
      uint8_t b = currentColor & 0xFF;

      float fadeFactor = 0.3 + 0.7 * (1.0 - abs((i - offset) % 10) / 10.0);
      strip.setPixelColor(i, strip.Color(r * fadeFactor, g * fadeFactor, b * fadeFactor));
    }
    
    strip.show();
    offset = (offset + 1) % NUMPIXELS;
  }
}

//-----------------------------------------------------
// Static Color Effect
void setStaticColors(uint32_t colors[]) {
  static bool initialized = false;

  if (!initialized) {
    for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
      strip.setPixelColor(i, colors[i % COLOR_COUNT]);
    }
    strip.show();
    initialized = true;
  }

  if (millis() - previousMillis >= effectDuration) {
    initialized = false;
  }
}



//-----------------------------------------------------
// Themed Rain Effect with Lightning and Drops Falling Outward
// (Non-blocking version that respects effect duration)
void rainbowRainEffect() {
  // Define active region boundaries (respecting omission zones)
  const int activeStart = OMIT_START;
  const int activeEnd = NUMPIXELS - OMIT_END - 1; // inclusive index
  int activeLength = activeEnd - activeStart + 1;
  if (activeLength <= 0) return; // Safety check

  // ----- Lightning/echo state machine variables -----
  enum LightningState { LIGHTNING_NONE, LIGHTNING_MAIN, LIGHTNING_ECHO_WAIT, LIGHTNING_ECHO };
  static LightningState lightningState = LIGHTNING_NONE;
  static unsigned long nextLightningTime = 0;
  static unsigned long lightningStartTime = 0;
  static unsigned long nextEchoTime = 0;
  static int echoCount = 0;
  
  // Timing constants (in milliseconds)
  const unsigned long mainFlashDuration = 200;  // Main lightning flash duration
  const unsigned long echoFlashDuration = 50;   // Each echo flash duration
  const unsigned long rainUpdateInterval = 100;   // How often to update the rain effect

  // ----- Rain update timing -----
  static unsigned long lastRainUpdate = 0;
  
  unsigned long now = millis();
  
  // Initialize nextLightningTime if needed
  if (nextLightningTime == 0) {
    nextLightningTime = now + random(5000, 15000);
  }
  
  // ----- Process Lightning/Echo States -----
  if (lightningState != LIGHTNING_NONE) {
    if (lightningState == LIGHTNING_MAIN) {
      // During main flash: force active region white.
      if (now - lightningStartTime < mainFlashDuration) {
        for (int i = activeStart; i <= activeEnd; i++) {
          strip.setPixelColor(i, strip.Color(255, 255, 255));
        }
        strip.show();
        return; // Remain in main flash.
      } else {
        // End main flash; initialize echo phase.
        echoCount = random(1, 5);  // Number of echo flashes (1 to 4)
        lightningState = LIGHTNING_ECHO_WAIT;
        nextEchoTime = now; // Start echo immediately.
        return;
      }
    }
    if (lightningState == LIGHTNING_ECHO_WAIT) {
      if (now >= nextEchoTime && echoCount > 0) {
        // Start an echo flash: set active region to a dim gray.
        for (int i = activeStart; i <= activeEnd; i++) {
          strip.setPixelColor(i, strip.Color(128, 128, 128));
        }
        strip.show();
        lightningStartTime = now;  // Reuse for echo timing.
        lightningState = LIGHTNING_ECHO;
        return;
      }
      return; // Still waiting for the next echo.
    }
    if (lightningState == LIGHTNING_ECHO) {
      if (now - lightningStartTime < echoFlashDuration) {
        return; // Remain in echo flash.
      } else {
        // End echo flash: clear the active region.
        for (int i = activeStart; i <= activeEnd; i++) {
          strip.setPixelColor(i, 0);
        }
        strip.show();
        echoCount--;
        if (echoCount > 0) {
          lightningState = LIGHTNING_ECHO_WAIT;
          nextEchoTime = now + random(50, 150);
        } else {
          // Lightning event is complete.
          lightningState = LIGHTNING_NONE;
          nextLightningTime = now + random(5000, 15000);
        }
        return;
      }
    }
  } else {
    // No lightning event in progress.
    if (now >= nextLightningTime) {
      lightningState = LIGHTNING_MAIN;
      lightningStartTime = now;
      // Immediately do the main lightning flash.
      for (int i = activeStart; i <= activeEnd; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
      strip.show();
      return;
    }
  }
  
  // ----- Rain Effect Update (only when not in a lightning event) -----
  if (now - lastRainUpdate >= rainUpdateInterval) {
    lastRainUpdate = now;
    
    // Shift the existing drop colors outward from the center of the active region.
    if (activeLength % 2 == 0) {
      // Even active length: two center positions.
      int centerLeft = activeStart + activeLength / 2 - 1;
      int centerRight = activeStart + activeLength / 2;
      
      // Shift left half leftwards.
      for (int i = activeStart; i < centerLeft; i++) {
        strip.setPixelColor(i, strip.getPixelColor(i + 1));
      }
      // Shift right half rightwards.
      for (int i = activeEnd; i > centerRight; i--) {
        strip.setPixelColor(i, strip.getPixelColor(i - 1));
      }
      
      // Clear center pixels.
      strip.setPixelColor(centerLeft, 0);
      strip.setPixelColor(centerRight, 0);
      
      // With a chance, spawn new drops at the center using a theme color.
      if (random(100) < 20) {  // 20% chance
        uint32_t color = themeColors[random(COLOR_COUNT)];
        strip.setPixelColor(centerLeft, color);
        strip.setPixelColor(centerRight, color);
      }
    } else {
      // Odd active length: one center position.
      int center = activeStart + activeLength / 2;
      
      // Shift left half leftwards.
      for (int i = activeStart; i < center; i++) {
        strip.setPixelColor(i, strip.getPixelColor(i + 1));
      }
      // Shift right half rightwards.
      for (int i = activeEnd; i > center; i--) {
        strip.setPixelColor(i, strip.getPixelColor(i - 1));
      }
      
      // Clear the center pixel.
      strip.setPixelColor(center, 0);
      
      // With a chance, spawn a new drop at the center.
      if (random(100) < 20) {
        uint32_t color = themeColors[random(COLOR_COUNT)];
        strip.setPixelColor(center, color);
      }
    }
    strip.show();
  }
}



