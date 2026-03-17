#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// ── WiFi Credentials ─────────────────────────────────────────────
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// ── Pin Definitions ──────────────────────────────────────────────
#define RELAY_PIN D1   // GPIO5
// Set to LOW or HIGH depending on relay module (most are ACTIVE LOW)
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ── State ────────────────────────────────────────────────────────
bool lightState = false;

ESP8266WebServer server(80);

// ── CORS Helper ──────────────────────────────────────────────────
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// ── Route: GET /status ───────────────────────────────────────────
void handleStatus() {
  addCORSHeaders();
  StaticJsonDocument<64> doc;
  doc["light"] = lightState;
  doc["ip"]    = WiFi.localIP().toString();
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// ── Route: POST /light ───────────────────────────────────────────
void handleLight() {
  addCORSHeaders();

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
    return;
  }

  StaticJsonDocument<64> req;
  DeserializationError err = deserializeJson(req, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"error\":\"Bad JSON\"}");
    return;
  }

  // Accepts: {"state": true/false} or {"state": "on"/"off"}
  if (req["state"].is<bool>()) {
    lightState = req["state"].as<bool>();
  } else {
    String s = req["state"].as<String>();
    s.toLowerCase();
    if      (s == "on"  || s == "true")  lightState = true;
    else if (s == "off" || s == "false") lightState = false;
  }

  digitalWrite(RELAY_PIN, lightState ? RELAY_ON : RELAY_OFF);
  Serial.printf("[RELAY] Light → %s\n", lightState ? "ON" : "OFF");

  StaticJsonDocument<32> resp;
  resp["light"] = lightState;
  String json;
  serializeJson(resp, json);
  server.send(200, "application/json", json);
}

// ── Route: GET /toggle ───────────────────────────────────────────
void handleToggle() {
  addCORSHeaders();
  lightState = !lightState;
  digitalWrite(RELAY_PIN, lightState ? RELAY_ON : RELAY_OFF);
  Serial.printf("[RELAY] Toggled → %s\n", lightState ? "ON" : "OFF");

  StaticJsonDocument<32> resp;
  resp["light"] = lightState;
  String json;
  serializeJson(resp, json);
  server.send(200, "application/json", json);
}

// ── Route: OPTIONS (preflight CORS) ─────────────────────────────
void handleOptions() {
  addCORSHeaders();
  server.send(204);
}

// ── Setup ────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);   // Start with light OFF

  // Connect to WiFi
  Serial.printf("\nConnecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());

  // Register routes
  server.on("/status",  HTTP_GET,     handleStatus);
  server.on("/light",   HTTP_POST,    handleLight);
  server.on("/toggle",  HTTP_GET,     handleToggle);
  server.on("/light",   HTTP_OPTIONS, handleOptions);
  server.onNotFound([]() {
    server.send(404, "application/json", "{\"error\":\"Not found\"}");
  });

  server.begin();
  Serial.println("HTTP server started on port 80");
  Serial.println("Endpoints:");
  Serial.println("  GET  /status   → current state");
  Serial.println("  POST /light    → {\"state\": true/false}");
  Serial.println("  GET  /toggle   → flip state");
}

// ── Loop ─────────────────────────────────────────────────────────
void loop() {
  server.handleClient();
}
