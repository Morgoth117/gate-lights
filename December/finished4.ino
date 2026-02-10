#include <Adafruit_NeoPixel.h>

#define PIN        6      // Pin where the NeoPixel strip is connected
#define NUMPIXELS  200     // Number of pixels in your NeoPixel strip
#define OMIT_START  25      // Number of LEDs to omit from the start
#define OMIT_END    37      // Number of LEDs to omit from the end

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0; // Store the last time the effect was updated
unsigned long effectDuration = 300000; // Duration of each effect in milliseconds
unsigned int currentEffect = 0; // To track the current effect
unsigned long wipePreviousMillis = 0; // Store the last time the color wipe was updated
int wipeIndex = OMIT_START; // Index to track the current LED in the color wipe
uint32_t wipeColors[3] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255)}; // Colors for wipe effect
int currentWipeColorIndex = 0; // Index to track the current color in the wipe

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= effectDuration) {
    previousMillis = currentMillis;
    currentEffect = (currentEffect + 1) % 4; // Cycle through the effects
    wipeIndex = OMIT_START; // Reset wipe index when switching effects
    currentWipeColorIndex = 0; // Reset wipe color index when switching effects
    wipePreviousMillis = currentMillis; // Reset wipe timer to align with effect change
  }

  switch (currentEffect) {
    case 0:
      awesomeTwinkle();
      break;
    case 1:
      colorWipe(wipeColors[currentWipeColorIndex], 50); // Color wipe with adjustable speed
      break;
    case 2:
      chasingEffect(strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255), 100); // Chasing effect
      break;
    case 3:
      setStaticColors(); // Set static colors
      break;
  }
}

// Modified awesomeTwinkle function without delay loop
void awesomeTwinkle() {
  // Randomly pick multiple LEDs to twinkle
  int numTwinkling = random(10, 20); // Twinkle between 10 and 20 LEDs at once
  int twinklingPixels[numTwinkling];
  uint32_t colors[numTwinkling];

  for (int i = 0; i < numTwinkling; i++) {
    twinklingPixels[i] = random(OMIT_START, NUMPIXELS - OMIT_END);
    int randomColor = random(3);

    // Set Christmas-themed colors
    switch (randomColor) {
      case 0: colors[i] = strip.Color(255, 0, 0); break;  // Red
      case 1: colors[i] = strip.Color(0, 255, 0); break;  // Green
      case 2: colors[i] = strip.Color(0, 0, 255); break; // White
    }

    // Set the picked LED to the color
    strip.setPixelColor(twinklingPixels[i], colors[i]);
  }
  strip.show();

  // Fade out all twinkling LEDs gradually
  for (int fade = 255; fade >= 0; fade -= 25) {
    for (int i = 0; i < numTwinkling; i++) {
      uint8_t r = (colors[i] >> 16) & 0xFF;
      uint8_t g = (colors[i] >> 8) & 0xFF;
      uint8_t b = colors[i] & 0xFF;
      strip.setPixelColor(twinklingPixels[i], strip.Color(r * fade / 255, g * fade / 255, b * fade / 255));
    }
    strip.show();
    delay(30); // Speed of fade-out
  }
}

// Consolidated color wipe function
void colorWipe(uint32_t color, int speed) {
  unsigned long currentMillis = millis();
  if (currentMillis - wipePreviousMillis >= speed) {
    wipePreviousMillis = currentMillis;

    if (wipeIndex < NUMPIXELS - OMIT_END) {
      strip.setPixelColor(wipeIndex, color); // Set color of the current LED
      strip.show(); // Update the strip
      wipeIndex++;
    } else {
      wipeIndex = OMIT_START; // Reset to start the wipe again
      currentWipeColorIndex = (currentWipeColorIndex + 1) % 3; // Cycle through red, green, blue
    }
  }
}

// Chasing effect with three colors
void chasingEffect(uint32_t color1, uint32_t color2, uint32_t color3, int wait) {
  for (int offset = OMIT_START; offset < NUMPIXELS - OMIT_END; offset++) {
    for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
      // Alternate colors based on position + offset
      if ((i + offset) % 9 < 3) { // First 3 LEDs in each group
        strip.setPixelColor(i, color1); // First color
      } else if ((i + offset) % 9 < 6) { // Next 3 LEDs in each group
        strip.setPixelColor(i, color2); // Second color
      } else { // Last 3 LEDs in each group
        strip.setPixelColor(i, color3); // Third color
      }
    }
    strip.show();
    delay(wait); // Delay for animation speed
  }
}

// Function to set static red, green, blue, and orange colors
void setStaticColors() {
  for (int i = OMIT_START; i < NUMPIXELS - OMIT_END; i++) {
    if (i % 4 == 0) {
      strip.setPixelColor(i, strip.Color(255, 0, 0)); // Red
    } else if (i % 4 == 1) {
      strip.setPixelColor(i, strip.Color(0, 255, 0)); // Green
    } else if (i % 4 == 2) {
      strip.setPixelColor(i, strip.Color(0, 0, 255)); // Blue
    } else {
      strip.setPixelColor(i, strip.Color(255, 165, 0)); // Orange
    }
  }
  strip.show(); // Update the strip
  delay(effectDuration);
}
