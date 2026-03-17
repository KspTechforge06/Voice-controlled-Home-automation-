# SparkHome — Voice Controlled Home Automation

Control a light bulb with your voice using an ESP8266, a relay module, and a browser-based dashboard. No cloud, no app install, no subscription — just your local WiFi.

![Platform](https://img.shields.io/badge/platform-ESP8266-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Status](https://img.shields.io/badge/status-working-brightgreen)

---

## Demo

| Light OFF | Light ON |
|-----------|----------|
| Bulb dimmed, badge shows OFF | Bulb glows amber, badge shows ON |

The dashboard works on both desktop and mobile. On Android Chrome, voice commands are fully supported. Add the HTML file to your home screen for an app-like experience.

---

## Features

- **Voice control** — say "light on", "turn off", "toggle", "band karo" etc.
- **Manual controls** — ON / OFF / Toggle buttons on the dashboard
- **Mobile optimized** — touch-friendly, respects notch/safe areas, no zoom issues
- **No cloud** — everything runs on your local WiFi network
- **REST API** — three simple HTTP endpoints you can call from anything
- **Works offline** — once the HTML is saved, it works without internet

---

## Hardware Required

| Component | Qty | Notes |
|-----------|-----|-------|
| ESP8266 (NodeMCU v1/v3 or Wemos D1 Mini) | 1 | Any ESP8266 board works |
| 5V Single Channel Relay Module | 1 | Most are active LOW |
| Lightbulb + holder + wire | 1 set | 220V AC |
| Jumper wires | 3 | Male-to-female |
| USB cable + power supply | 1 | For ESP8266 |

---

## Wiring

```
ESP8266          Relay Module
─────────        ────────────
D1 (GPIO5) ───→  IN
3.3V / Vin ───→  VCC
GND        ───→  GND

Relay            AC Bulb Circuit
─────            ───────────────
COM        ───→  Live (220V in)
NO         ───→  Bulb Live in
                 Neutral wire → Bulb directly
```

> **⚠️ Safety Warning:** The relay side carries 220V AC mains voltage. Use properly insulated wires, a project enclosure, and ensure no bare conductors are exposed. Do not touch the relay board while it is connected to mains power.

### Relay type

Most cheap relay modules are **active LOW** (relay triggers when IN pin is LOW). The firmware defaults to this. If your relay is active HIGH, change these lines in `voice_home.ino`:

```cpp
#define RELAY_ON  HIGH
#define RELAY_OFF LOW
```

---

## Software Setup

### 1. Arduino IDE

Install the ESP8266 board package if you haven't already:

1. Open Arduino IDE → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Tools → Board Manager → search **esp8266** → Install

### 2. Libraries

Install via Tools → Manage Libraries:

| Library | Author |
|---------|--------|
| ArduinoJson | Benoit Blanchon |

`ESP8266WiFi` and `ESP8266WebServer` are included with the ESP8266 board package.

### 3. Configure credentials

Open `voice_home.ino` and set your WiFi details:

```cpp
const char* ssid     = "YourWiFiName";
const char* password = "YourWiFiPassword";
```

### 4. Flash

1. Select your board: Tools → Board → **NodeMCU 1.0 (ESP-12E Module)**
2. Select the correct port under Tools → Port
3. Click Upload
4. Open Serial Monitor at **115200 baud**
5. Note the IP address printed on connection, e.g. `Connected! IP: 192.168.1.45`

---

## REST API

The ESP8266 exposes three endpoints on port 80:

| Method | Endpoint | Body | Response |
|--------|----------|------|----------|
| `GET` | `/status` | — | `{"light": true, "ip": "192.168.1.45"}` |
| `POST` | `/light` | `{"state": true}` | `{"light": true}` |
| `GET` | `/toggle` | — | `{"light": false}` |

The `/light` endpoint accepts both boolean and string values:

```json
{"state": true}
{"state": false}
{"state": "on"}
{"state": "off"}
```

### Example curl commands

```bash
# Check status
curl http://192.168.1.45/status

# Turn on
curl -X POST http://192.168.1.45/light \
  -H "Content-Type: application/json" \
  -d '{"state": true}'

# Toggle
curl http://192.168.1.45/toggle
```

---

## Dashboard Usage

Open `sparkhome_mobile.html` in a browser on the same WiFi network as the ESP8266.

1. Enter the ESP8266 IP address (from Serial Monitor)
2. Tap **Connect**
3. Use the ON / OFF / Toggle buttons, or tap the bulb icon
4. Tap the **mic button** and speak a command

### Supported voice commands

| Say this | Action |
|----------|--------|
| "light on" / "turn on" / "switch on" | Turn relay ON |
| "light off" / "turn off" / "switch off" | Turn relay OFF |
| "chalao" / "on karo" / "jala" | Turn relay ON |
| "band karo" / "bujha" | Turn relay OFF |
| "toggle" / "flip" | Toggle current state |

> Voice recognition uses the **Web Speech API**. It works in Chrome on Android and Chrome/Edge on desktop. Safari on iOS has partial support.

### Add to home screen (Android)

1. Open `sparkhome_mobile.html` in Chrome
2. Tap the three-dot menu → **Add to Home Screen**
3. The dashboard launches fullscreen like a native app

---

## Project Structure

```
sparkhome/
├── voice_home.ino          # ESP8266 firmware (Arduino)
├── sparkhome_mobile.html   # Mobile-optimized dashboard
├── sparkhome_dashboard.html # Desktop dashboard (alternate)
└── README.md
```

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| ESP8266 not connecting to WiFi | Check SSID/password, ensure 2.4 GHz network |
| Dashboard says "Connection failed" | Confirm phone and ESP8266 are on same WiFi |
| Relay clicks but bulb doesn't turn on | Check COM/NO wiring, try swapping to NC |
| Relay doesn't respond at all | Check `RELAY_ON` polarity, try active HIGH setting |
| Voice button does nothing | Use Chrome on Android; grant microphone permission |
| Voice recognizes but doesn't send command | Connect ESP8266 first before using voice |
| Serial Monitor shows garbage | Set baud rate to 115200 |

---

## How It Works

```
┌─────────────┐   WiFi (HTTP)   ┌──────────────┐   GPIO D1   ┌─────────┐   220V AC   ┌──────┐
│  Browser    │ ──────────────→ │   ESP8266    │ ──────────→ │  Relay  │ ──────────→ │ Bulb │
│  Dashboard  │ ←────────────── │  Web Server  │             │ Module  │             └──────┘
└─────────────┘   JSON response └──────────────┘             └─────────┘
       ↑
  Web Speech API
  (microphone)
```

1. Browser captures voice via Web Speech API and parses the command
2. Dashboard sends an HTTP POST to `ESP8266_IP/light` with `{"state": true/false}`
3. ESP8266 sets GPIO D1 HIGH or LOW
4. Relay coil energizes/de-energizes, switching the 220V AC circuit
5. Bulb turns on or off

---

## Built With

- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [ArduinoJson](https://arduinojson.org/)
- [Web Speech API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Speech_API)

---

## License

MIT — free to use, modify, and build on.

---

*Built by KSP — SparkLab*# SparkHome — Voice Controlled Home Automation

Control a light bulb with your voice using an ESP8266, a relay module, and a browser-based dashboard. No cloud, no app install, no subscription — just your local WiFi.

![Platform](https://img.shields.io/badge/platform-ESP8266-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Status](https://img.shields.io/badge/status-working-brightgreen)

---

## Demo

| Light OFF | Light ON |
|-----------|----------|
| Bulb dimmed, badge shows OFF | Bulb glows amber, badge shows ON |

The dashboard works on both desktop and mobile. On Android Chrome, voice commands are fully supported. Add the HTML file to your home screen for an app-like experience.

---

## Features

- **Voice control** — say "light on", "turn off", "toggle", "band karo" etc.
- **Manual controls** — ON / OFF / Toggle buttons on the dashboard
- **Mobile optimized** — touch-friendly, respects notch/safe areas, no zoom issues
- **No cloud** — everything runs on your local WiFi network
- **REST API** — three simple HTTP endpoints you can call from anything
- **Works offline** — once the HTML is saved, it works without internet

---

## Hardware Required

| Component | Qty | Notes |
|-----------|-----|-------|
| ESP8266 (NodeMCU v1/v3 or Wemos D1 Mini) | 1 | Any ESP8266 board works |
| 5V Single Channel Relay Module | 1 | Most are active LOW |
| Lightbulb + holder + wire | 1 set | 220V AC |
| Jumper wires | 3 | Male-to-female |
| USB cable + power supply | 1 | For ESP8266 |

---

## Wiring

```
ESP8266          Relay Module
─────────        ────────────
D1 (GPIO5) ───→  IN
3.3V / Vin ───→  VCC
GND        ───→  GND

Relay            AC Bulb Circuit
─────            ───────────────
COM        ───→  Live (220V in)
NO         ───→  Bulb Live in
                 Neutral wire → Bulb directly
```

> **⚠️ Safety Warning:** The relay side carries 220V AC mains voltage. Use properly insulated wires, a project enclosure, and ensure no bare conductors are exposed. Do not touch the relay board while it is connected to mains power.

### Relay type

Most cheap relay modules are **active LOW** (relay triggers when IN pin is LOW). The firmware defaults to this. If your relay is active HIGH, change these lines in `voice_home.ino`:

```cpp
#define RELAY_ON  HIGH
#define RELAY_OFF LOW
```

---

## Software Setup

### 1. Arduino IDE

Install the ESP8266 board package if you haven't already:

1. Open Arduino IDE → Preferences
2. Add to "Additional Board Manager URLs":
   ```
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Tools → Board Manager → search **esp8266** → Install

### 2. Libraries

Install via Tools → Manage Libraries:

| Library | Author |
|---------|--------|
| ArduinoJson | Benoit Blanchon |

`ESP8266WiFi` and `ESP8266WebServer` are included with the ESP8266 board package.

### 3. Configure credentials

Open `voice_home.ino` and set your WiFi details:

```cpp
const char* ssid     = "YourWiFiName";
const char* password = "YourWiFiPassword";
```

### 4. Flash

1. Select your board: Tools → Board → **NodeMCU 1.0 (ESP-12E Module)**
2. Select the correct port under Tools → Port
3. Click Upload
4. Open Serial Monitor at **115200 baud**
5. Note the IP address printed on connection, e.g. `Connected! IP: 192.168.1.45`

---

## REST API

The ESP8266 exposes three endpoints on port 80:

| Method | Endpoint | Body | Response |
|--------|----------|------|----------|
| `GET` | `/status` | — | `{"light": true, "ip": "192.168.1.45"}` |
| `POST` | `/light` | `{"state": true}` | `{"light": true}` |
| `GET` | `/toggle` | — | `{"light": false}` |

The `/light` endpoint accepts both boolean and string values:

```json
{"state": true}
{"state": false}
{"state": "on"}
{"state": "off"}
```

### Example curl commands

```bash
# Check status
curl http://192.168.1.45/status

# Turn on
curl -X POST http://192.168.1.45/light \
  -H "Content-Type: application/json" \
  -d '{"state": true}'

# Toggle
curl http://192.168.1.45/toggle
```

---

## Dashboard Usage

Open `sparkhome_mobile.html` in a browser on the same WiFi network as the ESP8266.

1. Enter the ESP8266 IP address (from Serial Monitor)
2. Tap **Connect**
3. Use the ON / OFF / Toggle buttons, or tap the bulb icon
4. Tap the **mic button** and speak a command

### Supported voice commands

| Say this | Action |
|----------|--------|
| "light on" / "turn on" / "switch on" | Turn relay ON |
| "light off" / "turn off" / "switch off" | Turn relay OFF |
| "chalao" / "on karo" / "jala" | Turn relay ON |
| "band karo" / "bujha" | Turn relay OFF |
| "toggle" / "flip" | Toggle current state |

> Voice recognition uses the **Web Speech API**. It works in Chrome on Android and Chrome/Edge on desktop. Safari on iOS has partial support.

### Add to home screen (Android)

1. Open `sparkhome_mobile.html` in Chrome
2. Tap the three-dot menu → **Add to Home Screen**
3. The dashboard launches fullscreen like a native app

---

## Project Structure

```
sparkhome/
├── voice_home.ino          # ESP8266 firmware (Arduino)
├── sparkhome_mobile.html   # Mobile-optimized dashboard
├── sparkhome_dashboard.html # Desktop dashboard (alternate)
└── README.md
```

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| ESP8266 not connecting to WiFi | Check SSID/password, ensure 2.4 GHz network |
| Dashboard says "Connection failed" | Confirm phone and ESP8266 are on same WiFi |
| Relay clicks but bulb doesn't turn on | Check COM/NO wiring, try swapping to NC |
| Relay doesn't respond at all | Check `RELAY_ON` polarity, try active HIGH setting |
| Voice button does nothing | Use Chrome on Android; grant microphone permission |
| Voice recognizes but doesn't send command | Connect ESP8266 first before using voice |
| Serial Monitor shows garbage | Set baud rate to 115200 |

---

## How It Works

```
┌─────────────┐   WiFi (HTTP)   ┌──────────────┐   GPIO D1   ┌─────────┐   220V AC   ┌──────┐
│  Browser    │ ──────────────→ │   ESP8266    │ ──────────→ │  Relay  │ ──────────→ │ Bulb │
│  Dashboard  │ ←────────────── │  Web Server  │             │ Module  │             └──────┘
└─────────────┘   JSON response └──────────────┘             └─────────┘
       ↑
  Web Speech API
  (microphone)
```

1. Browser captures voice via Web Speech API and parses the command
2. Dashboard sends an HTTP POST to `ESP8266_IP/light` with `{"state": true/false}`
3. ESP8266 sets GPIO D1 HIGH or LOW
4. Relay coil energizes/de-energizes, switching the 220V AC circuit
5. Bulb turns on or off

---

## Built With

- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
- [ArduinoJson](https://arduinojson.org/)
- [Web Speech API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Speech_API)

---

## License

MIT — free to use, modify, and build on.

---

*Built by KSP — SparkLab*
