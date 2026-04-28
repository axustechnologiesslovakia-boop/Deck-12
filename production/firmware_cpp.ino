/*
HackPad Firmware - XIAO RP2040

Features:
- 4x3 key matrix (12 keys)
- SK6812 RGB LEDs (animations supported)
- SSD1306 OLED display (Spotify info)
- USB Serial communication (Node.js app)

Purpose:
Custom macro pad / Stream Deck alternative with real-time display

Author: Sebastian
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <math.h>

// ===================== CONFIG =====================
#define NUM_KEYS 12
#define NUM_ROWS 4
#define NUM_COLS 3

const int rowPins[NUM_ROWS] = {2, 3, 4, 5};
const int colPins[NUM_COLS] = {6, 7, 8};

#define LED_PIN 9
#define NUM_LEDS 12

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ===================== OBJECTS =====================
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===================== STATE =====================
struct KeyState {
  bool current;
  bool last;
  unsigned long lastDebounce;
};

KeyState keys[NUM_KEYS];

struct DeviceState {
  String song;
  String artist;
  int progress;
  int duration;
  uint8_t brightness;
  uint32_t ledColors[NUM_KEYS];
} state;

// ===================== ANIMATION =====================
enum AnimMode {
  STATIC,
  RAINBOW,
  BREATH,
  REACTIVE
};

AnimMode currentAnim = STATIC;
unsigned long animStep = 0;

// ===================== SERIAL =====================
String serialBuffer = "";

// ===================== HELPERS =====================
String formatTime(int seconds) {
  int mins = seconds / 60;
  int secs = seconds % 60;
  char buffer[6];
  sprintf(buffer, "%02d:%02d", mins, secs);
  return String(buffer);
}

uint32_t wheel(byte pos) {
  if (pos < 85) {
    return strip.Color(pos * 3, 255 - pos * 3, 0);
  } else if (pos < 170) {
    pos -= 85;
    return strip.Color(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return strip.Color(0, pos * 3, 255 - pos * 3);
  }
}

uint8_t breathValue() {
  return (sin(animStep * 0.05) * 127 + 128);
}

// ===================== SETUP =====================
void setupMatrix() {
  for (int r = 0; r < NUM_ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (int c = 0; c < NUM_COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }
}

void setupLEDs() {
  strip.begin();
  strip.setBrightness(50);
  strip.show();
}

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }
}

void setup() {
  Serial.begin(115200);
  setupMatrix();
  setupLEDs();
  setupDisplay();

  // Default state
  state.song = "Demo Song";
  state.artist = "Offline";
  state.progress = 30;
  state.duration = 120;
  state.brightness = 50;

  for (int i = 0; i < NUM_KEYS; i++) {
    keys[i] = {false, false, 0};
    state.ledColors[i] = strip.Color(0, 0, 50);
  }
}

// ===================== MATRIX =====================
void sendKeyPress(int key) {
  StaticJsonDocument<64> doc;
  doc["key"] = key;
  serializeJson(doc, Serial);
  Serial.println();

  if (currentAnim == REACTIVE) {
    strip.setPixelColor(key, strip.Color(255, 255, 255));
    strip.show();
  }
}

void scanMatrix() {
  for (int r = 0; r < NUM_ROWS; r++) {
    digitalWrite(rowPins[r], LOW);

    for (int c = 0; c < NUM_COLS; c++) {
      int index = r * NUM_COLS + c;
      bool reading = (digitalRead(colPins[c]) == LOW);

      if (reading != keys[index].last) {
        keys[index].lastDebounce = millis();
      }

      if ((millis() - keys[index].lastDebounce) > 20) {
        if (reading != keys[index].current) {
          keys[index].current = reading;
          if (reading) sendKeyPress(index);
        }
      }

      keys[index].last = reading;
    }

    digitalWrite(rowPins[r], HIGH);
  }
}

// ===================== SERIAL =====================
void parseJSON(String json) {
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, json)) return;

  if (doc.containsKey("song")) state.song = doc["song"].as<String>();
  if (doc.containsKey("artist")) state.artist = doc["artist"].as<String>();
  if (doc.containsKey("progress")) state.progress = doc["progress"];
  if (doc.containsKey("duration")) state.duration = doc["duration"];

  if (doc.containsKey("brightness")) {
    state.brightness = doc["brightness"];
    strip.setBrightness(state.brightness);
  }

  if (doc.containsKey("anim")) {
    String mode = doc["anim"];
    if (mode == "rainbow") currentAnim = RAINBOW;
    else if (mode == "breath") currentAnim = BREATH;
    else if (mode == "reactive") currentAnim = REACTIVE;
    else currentAnim = STATIC;
  }

  if (doc.containsKey("leds")) {
    JsonArray arr = doc["leds"];
    for (int i = 0; i < NUM_KEYS && i < arr.size(); i++) {
      JsonArray c = arr[i];
      state.ledColors[i] = strip.Color(c[0], c[1], c[2]);
    }
  }
}

void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseJSON(serialBuffer);
      serialBuffer = "";
    } else {
      serialBuffer += c;
    }
  }
}

// ===================== LED =====================
void updateLEDs() {
  switch (currentAnim) {
    case STATIC:
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, state.ledColors[i]);
      }
      break;

    case RAINBOW:
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, wheel((i * 256 / NUM_LEDS + animStep) & 255));
      }
      break;

    case BREATH: {
      uint8_t b = breathValue();
      for (int i = 0; i < NUM_LEDS; i++) {
        uint32_t c = state.ledColors[i];
        uint8_t r = ((c >> 16) & 0xFF) * b / 255;
        uint8_t g = ((c >> 8) & 0xFF) * b / 255;
        uint8_t bl = (c & 0xFF) * b / 255;
        strip.setPixelColor(i, r, g, bl);
      }
      break;
    }

    case REACTIVE:
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, state.ledColors[i]);
      }
      break;
  }

  strip.show();
  animStep++;
}

// ===================== OLED =====================
void drawOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println(state.song);

  display.setCursor(0, 10);
  display.println(state.artist);

  display.setCursor(0, 30);
  display.print(formatTime(state.progress));
  display.print(" / ");
  display.print(formatTime(state.duration));

  int barWidth = map(state.progress, 0, state.duration, 0, 128);
  display.drawRect(0, 50, 128, 10, SSD1306_WHITE);
  display.fillRect(0, 50, barWidth, 10, SSD1306_WHITE);

  display.display();
}

// ===================== LOOP =====================
unsigned long lastOLED = 0;
unsigned long lastLED = 0;

void loop() {
  scanMatrix();
  handleSerial();

  if (millis() - lastLED > 20) {
    updateLEDs();
    lastLED = millis();
  }

  if (millis() - lastOLED > 200) {
    drawOLED();
    lastOLED = millis();
  }
}