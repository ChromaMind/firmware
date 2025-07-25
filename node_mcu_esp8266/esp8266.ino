#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Adafruit_NeoPixel.h>
#include <SSD1306Wire.h>

#define LED_PIN     D4
#define NUM_PIXELS  32
#define MAX_WAIT_MS 4000
#define WEBSOCKET_LOOP_INTERVAL_MS 5

SSD1306Wire display(0x3C, 14, 12);
Adafruit_NeoPixel strip(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "Cubex";
const char* password = "thunderluck";
IPAddress local_IP(10, 151, 240, 37);
IPAddress gateway(10, 151, 240, 97);
IPAddress subnet(255, 255, 255, 0);

WebSocketsServer webSocket = WebSocketsServer(81);

uint8_t blinkMode = 0;
uint16_t blinkInterval = 50;
uint8_t brightness = 100;

uint16_t totalFrames = 0;
uint16_t currentFrame = 0;
bool playing = false;
unsigned long frameStartTime = 0;
unsigned long lastDataTime = 0;
unsigned long lastFrameFlash = 0;
bool frameOn = true;
unsigned long lastWebSocketLoop = 0;
bool rowToggle = false;

uint16_t XY(uint8_t x, uint8_t y) {
  return y * 16 + x;
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.clear();
  display.drawString(0, 0, "Booting...");
  display.display();

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  display.drawString(0, 15, "Connecting to WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  display.clear();
  display.drawString(0, 0, "Connected!");
  display.drawString(0, 15, "IP: " + WiFi.localIP().toString());
  display.display();
  Serial.println("WiFi Connected");

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket Server Started");
  display.drawString(0, 25, "WebSocket: :81");
  display.display();
}

void loop() {
  unsigned long now = millis();

  if (now - lastWebSocketLoop >= WEBSOCKET_LOOP_INTERVAL_MS) {
    webSocket.loop();
    lastWebSocketLoop = now;
  }

  if (playing) {
    switch (blinkMode) {
      case 1: mode_1(now); break;
      case 2: mode_2(now); break;
      case 3: mode_3(now); break;
      case 4: mode_4(now); break;
      case 5: mode_5(now); break;
      case 6: mode_6(now); break;
      case 7: mode_7(now); break;
      case 8: mode_8(now); break;
    }
  }

  if (!playing && now - lastDataTime > MAX_WAIT_MS) {
    clearStrip();
    totalFrames = 0;
    currentFrame = 0;
  }
}

void clearStrip() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}

void mode_1(unsigned long now) {
  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    frameOn = !frameOn;

    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, frameOn ? strip.Color(255, 255, 255) : 0);
    }
    strip.show();
  }
}

void mode_2(unsigned long now) {
  static uint8_t r1 = random(256), g1 = random(256), b1 = random(256);
  static uint8_t r2 = random(256), g2 = random(256), b2 = random(256);
  static uint8_t targetR1 = random(256), targetG1 = random(256), targetB1 = random(256);
  static uint8_t targetR2 = random(256), targetG2 = random(256), targetB2 = random(256);
  static unsigned long lastColorUpdate = 0;
  const uint8_t step = 5;

  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    frameOn = !frameOn;

    if (frameOn && now - lastColorUpdate >= blinkInterval) {
      lastColorUpdate = now;
      r1 += (targetR1 > r1) ? min((int)step, (int)(targetR1 - r1)) : -min((int)step, (int)(r1 - targetR1));
      g1 += (targetG1 > g1) ? min((int)step, (int)(targetG1 - g1)) : -min((int)step, (int)(g1 - targetG1));
      b1 += (targetB1 > b1) ? min((int)step, (int)(targetB1 - b1)) : -min((int)step, (int)(b1 - targetB1));

      r2 += (targetR2 > r2) ? min((int)step, (int)(targetR2 - r2)) : -min((int)step, (int)(r2 - targetR2));
      g2 += (targetG2 > g2) ? min((int)step, (int)(targetG2 - g2)) : -min((int)step, (int)(g2 - targetG2));
      b2 += (targetB2 > b2) ? min((int)step, (int)(targetB2 - b2)) : -min((int)step, (int)(b2 - targetB2));

      if (abs(r1 - targetR1) < 5) targetR1 = random(256);
      if (abs(g1 - targetG1) < 5) targetG1 = random(256);
      if (abs(b1 - targetB1) < 5) targetB1 = random(256);
      if (abs(r2 - targetR2) < 5) targetR2 = random(256);
      if (abs(g2 - targetG2) < 5) targetG2 = random(256);
      if (abs(b2 - targetB2) < 5) targetB2 = random(256);
    }

    for (int i = 0; i < NUM_PIXELS; i++) {
      strip.setPixelColor(i, frameOn ?
        (i < 16 ? strip.Color(r1, g1, b1) : strip.Color(r2, g2, b2)) : 0);
    }
    strip.show();
  }
}

void mode_3(unsigned long now) {
  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    frameOn = !frameOn;

    for (uint8_t x = 0; x < 16; x++) {
      for (uint8_t y = 0; y < 2; y++) {
        bool edge = (x == 0 || x == 15);
        strip.setPixelColor(XY(x, y), (frameOn && edge) ? strip.Color(255, 0, 0) : 0);
      }
    }
    strip.show();
  }
}

void mode_4(unsigned long now) {
  static uint8_t radius = 0;
  static bool expanding = true;

  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    frameOn = !frameOn;
    clearStrip();
    uint8_t mid = 7;

    for (int dx = -radius; dx <= radius; dx++) {
      uint8_t xL = mid + dx;
      if (xL < 16) {
        for (uint8_t y = 0; y < 2; y++)
          strip.setPixelColor(XY(xL, y), frameOn ? strip.Color(0, 255, 0) : 0);
      }
    }

    expanding ? radius++ : radius--;
    if (radius >= 7) expanding = false;
    if (radius == 0) expanding = true;

    strip.show();
  }
}

void mode_5(unsigned long now) {
  static uint8_t step = 0;

  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    frameOn = !frameOn;
    clearStrip();

    uint8_t left = step;
    uint8_t right = 15 - step;
    if (left <= right) {
      for (uint8_t y = 0; y < 2; y++) {
        strip.setPixelColor(XY(left, y), frameOn ? strip.Color(0, 255, 255) : 0);
        strip.setPixelColor(XY(right, y), frameOn ? strip.Color(0, 255, 255) : 0);
      }
      step++;
    } else {
      step = 0;
    }

    strip.show();
  }
}

void mode_6(unsigned long now) {
  static uint8_t pos = 0;

  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    clearStrip();
    frameOn = !frameOn;

    for (uint8_t y = 0; y < 2; y++)
      strip.setPixelColor(XY(pos, y), frameOn ? strip.Color(255, 255, 0) : 0);

    pos = (pos + 1) % 16;
    strip.show();
  }
}

void mode_7(unsigned long now) {
  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    rowToggle = !rowToggle;

    for (uint8_t x = 0; x < 16; x++) {
      for (uint8_t y = 0; y < 2; y++) {
        bool show = (y == 0 && rowToggle) || (y == 1 && !rowToggle);
        strip.setPixelColor(XY(x, y), show ? strip.Color(255, 0, 255) : 0);
      }
    }
    strip.show();
  }
}

void mode_8(unsigned long now) {
  static uint8_t head = 0;
  static int8_t dir = 1;

  if (now - lastFrameFlash >= blinkInterval) {
    lastFrameFlash = now;
    clearStrip();
    frameOn = !frameOn;

    for (int8_t i = 0; i < 3; i++) {
      int8_t x = head - i * dir;
      if (x >= 0 && x < 16) {
        for (uint8_t y = 0; y < 2; y++)
          strip.setPixelColor(XY(x, y), frameOn ? strip.Color(100, 100, 255) : 0);
      }
    }

    head += dir;
    if (head == 15 || head == 0) dir *= -1;

    strip.show();
  }
}

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t* payload, size_t length) {
  if (type != WStype_TEXT) return;

  char* token = strtok((char*)payload, ";");
  if (token) blinkMode = atoi(token);

  token = strtok(NULL, ";");
  if (token) blinkInterval = atoi(token);

  token = strtok(NULL, ";");
  if (token) {
    brightness = atoi(token);
    strip.setBrightness(brightness);
  }

  Serial.printf("[mode=%d blink=%dms brightness=%d]\n", blinkMode, blinkInterval, brightness);

  totalFrames = 1;
  currentFrame = 0;
  playing = true;
  frameStartTime = millis();
  lastFrameFlash = frameStartTime;
  frameOn = true;
  lastDataTime = millis();
}
