## NeoPixel WebSocket Control \
### ESP8266 / ESP32 / ESP32-C3
___ 
This project allows control of NeoPixel LED animations over WebSocket using different ESP modules (ESP8266, ESP32, ESP32-C3) with a simple semicolon-delimited message protocol.

---

Supported Boards:
- ESP32-C3 Super Mini
- ESP32 MH-ET Live MiniKit A (with and without OLED)
- NodeMCU ESP8266 V3

---

WebSocket Control:

The ESP device runs a WebSocket server on port 81.
To control it, connect to:

  ws://<device-ip>:81/

You can use any WebSocket client or script (e.g. Python, browser).

Message Format:
  mode;blink_rate;power

Example:
  2;50;5

Fields:
- mode: Integer (1–6) — selects the animation mode
- blink_rate: Integer (Hz) — blinking speed or beat rate (interpreted per mode)
- power: Integer — reserved for brightness or pixel energy (defined by firmware)

---

Animation Modes:

1 = Strobe effect (whole strip flashes random colors)
2 = Snake effect (moving 3-pixel block over flowing background)
3 = Alternating group blink (odd/even pixels)
4 = Rapid blink alternate (faster odd/even toggle)
5 = Fading strobe through vibrant colors
6 = Fading alternate between two fixed color zones (odd/even)

---

Timing:

- blink_rate affects how fast animation steps happen, typically in Hz (blinks per second)
- message is interpreted immediately, overriding the previous frame

---

OLED Support (for ESP32 with display):
- Address: 0x3C
- SDA: GPIO21
- SCL: GPIO22
- Controller: SH1106 (128x64)

---

Pin Connections:

ESP32-C3 Super Mini:
  - Data to NeoPixel: GPIO2

ESP32 MH-ET Live:
  - Data to NeoPixel: GPIO18
  - OLED SDA: GPIO21
  - OLED SCL: GPIO22

ESP8266 NodeMCU:
  - Data to NeoPixel: D4 (GPIO2)

---

NeoPixel Wiring:

- VCC: 3.3V (ESP32) or 5V (ESP8266)
- GND: shared
- DIN of first module goes to ESP pin
- DOUT of each module connects to next module’s DIN

Each module = CJMCU-2812-8 (8 RGB LEDs)

---

Required Arduino Libraries:

- Adafruit_NeoPixel
- Adafruit_GFX
- Adafruit_SH1106 or SSD1306 (if adapted)
- WebSockets (arduinoWebSockets)
- WiFi (ESP8266WiFi / WiFi.h for ESP32)
- ArduinoJson (if using JSON in other builds)

---
Static IPs

Since 2 of the devices don't have display they are set with static ip

ESP32 without display **10.151.240.18**
ESP C3 supermini without display **10.151.240.99**

____



