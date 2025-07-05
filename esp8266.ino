#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <SSD1306Wire.h>
//BOARD LOLIN (WEMOS) D1 ESP8266
// ====== OLED SETUP (ThingPulse SSD1306 library) ======
SSD1306Wire display(0x3C, 14, 12); // SDA = GPIO14, SCL = GPIO12

// ====== NeoPixel Setup ======
#define LED_PIN     D4
#define NUM_PIXELS  32
Adafruit_NeoPixel strip(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ====== WiFi Setup ======
const char* ssid = "Cubex";
const char* password = "thunderluck";

IPAddress local_IP(10, 151, 240, 37);
IPAddress gateway(10, 151, 240, 97);
IPAddress subnet(255, 255, 255, 0);

// ====== WebSocket Setup ======
WebSocketsServer webSocket = WebSocketsServer(81);

// ====== Animation Frame Buffer ======
#define MAX_FRAMES 150
#define MAX_WAIT_MS 1000

struct Frame {
  uint32_t delayFromPrevious;
  uint8_t leds[NUM_PIXELS][3]; // [r,g,b]
};

Frame frames[MAX_FRAMES];
uint16_t totalFrames = 0;
uint16_t currentFrame = 0;
bool playing = false;
unsigned long frameStartTime = 0;
unsigned long lastDataTime = 0;

// ====== Setup ======
void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();

  display.init();
  display.flipScreenVertically(); // Flip the display

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

// ====== Main Loop ======
void loop() {
  webSocket.loop();
  unsigned long now = millis();

  if (playing && totalFrames > 0) {
    if (now - frameStartTime >= frames[currentFrame].delayFromPrevious) {
      displayFrame(currentFrame);
      currentFrame++;
      frameStartTime = now;
      if (currentFrame >= totalFrames) {
        playing = false;
        lastDataTime = now;
      }
    }
  }

  if (!playing && totalFrames > 0 && now - lastDataTime > MAX_WAIT_MS) {
    clearStrip();
    totalFrames = 0;
    currentFrame = 0;
  }
}

// ====== Render Frame ======
void displayFrame(uint16_t index) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, strip.Color(
      frames[index].leds[i][0],
      frames[index].leds[i][1],
      frames[index].leds[i][2]
    ));
  }
  strip.show();
}

// ====== Clear Strip ======
void clearStrip() {
  const int steps = 30;
  const int delayPerStep = 100; // 30 steps * 100 ms = 3 seconds

  for (int step = steps; step >= 0; step--) {
    float factor = step / (float)steps;

    for (int i = 0; i < NUM_PIXELS; i++) {
      uint32_t color = strip.getPixelColor(i);
      uint8_t r = (uint8_t)(factor * ((color >> 16) & 0xFF));
      uint8_t g = (uint8_t)(factor * ((color >> 8) & 0xFF));
      uint8_t b = (uint8_t)(factor * (color & 0xFF));
      strip.setPixelColor(i, strip.Color(r, g, b));
    }

    strip.show();
    delay(delayPerStep);
  }

  // Final clear to make sure it's off
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();

  Serial.println("Strip faded out and cleared.");
  display.drawString(0, 50, "Strip faded out");
  display.display();
}

// ====== WebSocket Event Handler ======
void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t* payload, size_t length) {
  if (type != WStype_TEXT) return;

  StaticJsonDocument<16384> doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.println("JSON error: " + String(err.c_str()));
    display.drawString(0, 40, "JSON error");
    display.display();
    return;
  }

  JsonArray arr = doc.as<JsonArray>();
  if (arr.size() == 0) return;

  totalFrames = 0;
  currentFrame = 0;
  playing = false;
  uint32_t prevTime = arr[0]["time"] | 0;
  bool overflow = false;

  for (JsonObject obj : arr) {
    if (totalFrames >= MAX_FRAMES) {
      overflow = true;
      break;
    }

    uint32_t currentTime = obj["time"] | 0;
    uint32_t delay = currentTime - prevTime;
    prevTime = currentTime;

    JsonArray leds2D = obj["leds"];
    if (leds2D.size() != 2) continue;

    JsonArray ledsA = leds2D[0];
    JsonArray ledsB = leds2D[1];
    if (ledsA.size() + ledsB.size() != NUM_PIXELS) continue;

    Frame& f = frames[totalFrames];
    f.delayFromPrevious = delay;

    int idx = 0;
    for (JsonObject c : ledsA) {
      uint8_t a = c.containsKey("a") ? (uint8_t)c["a"] : 100;
      f.leds[idx][0] = (c["r"] | 0) * a / 100;
      f.leds[idx][1] = (c["g"] | 0) * a / 100;
      f.leds[idx][2] = (c["b"] | 0) * a / 100;
      idx++;
    }
    for (JsonObject c : ledsB) {
      uint8_t a = c.containsKey("a") ? (uint8_t)c["a"] : 100;
      f.leds[idx][0] = (c["r"] | 0) * a / 100;
      f.leds[idx][1] = (c["g"] | 0) * a / 100;
      f.leds[idx][2] = (c["b"] | 0) * a / 100;
      idx++;
    }

    totalFrames++;
  }

  if (overflow) {
    Serial.println("[!] Frame overflow");
    display.drawString(0, 40, "Overflow!");
    display.display();
  }

  if (totalFrames > 0) {
    Serial.printf("✅ %d frame(s) received\n", totalFrames);
    display.clear();
    display.drawString(0, 0, "Frames: " + String(totalFrames));
    display.drawString(0, 20, "Playing...");
    display.display();
    currentFrame = 0;
    frameStartTime = millis();
    playing = true;
  }
}
