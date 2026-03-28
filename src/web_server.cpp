// web_server.cpp — WiFi AP-mode HTTP server for SANGI web UI.
// Starts a softAP named "SANGI" (open network) and serves a control page
// at http://192.168.4.1. No router or internet connection required.

#ifndef NATIVE_BUILD

#include "web_server.h"
#include "web_ui.h"
#include <WiFi.h>

WebServerManager webServerManager;

WebServerManager::WebServerManager()
  : server_(WIFI_SERVER_PORT),
    em_(nullptr), bm_(nullptr), im_(nullptr), cfg_(nullptr), p_(nullptr) {}

// ===== DIAGNOSTICS =====

void WebServerManager::logHeap(const char* ctx) {
  Serial.printf("[WEB] Free heap (%s): %u bytes\n", ctx, ESP.getFreeHeap());
}

// ===== INIT =====

void WebServerManager::init() {
  logHeap("pre-WiFi");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, nullptr, WIFI_AP_CHANNEL);  // nullptr = open network

  Serial.printf("[WEB] AP started — SSID: %s | IP: %s\n",
                WIFI_AP_SSID,
                WiFi.softAPIP().toString().c_str());
  logHeap("post-WiFi");

  if (ESP.getFreeHeap() < WEB_MIN_FREE_HEAP) {
    Serial.printf("[WEB] WARNING: heap below %u byte threshold\n", WEB_MIN_FREE_HEAP);
  }

  // Route registration
  server_.on("/",                HTTP_GET,  [this]() { handleRoot(); });
  server_.on("/api/status",      HTTP_GET,  [this]() { handleApiStatus(); });
  server_.on("/api/emotion",     HTTP_POST, [this]() { handleApiEmotion(); });
  server_.on("/api/gesture",     HTTP_POST, [this]() { handleApiGesture(); });
  server_.on("/api/config",      HTTP_GET,  [this]() { handleApiConfigGet(); });
  server_.on("/api/config",      HTTP_POST, [this]() { handleApiConfigPost(); });
  server_.on("/api/config/reset",HTTP_POST, [this]() { handleApiConfigReset(); });
  server_.onNotFound([this]() { handleNotFound(); });

  server_.begin();
  Serial.printf("[WEB] Server listening on port %d\n", WIFI_SERVER_PORT);
}

// ===== UPDATE =====

void WebServerManager::update() {
  server_.handleClient();
}

// ===== ROUTE HANDLERS =====

// GET / — serves the full HTML control UI from PROGMEM.
void WebServerManager::handleRoot() {
  server_.sendHeader("Content-Encoding", "identity");
  server_.send_P(200, "text/html", WEB_UI_HTML);
}

// GET /api/status — returns robot state as JSON (hand-crafted, no library).
void WebServerManager::handleApiStatus() {
  char buf[256];
  int emotionId        = em_ ? (int)em_->getCurrentEmotion() : 0;
  const char* emoName  = em_ ? emotionRegistry.getName(em_->getCurrentEmotion()) : "UNKNOWN";
  float voltage        = bm_ ? bm_->readVoltage() : 0.0f;
  int stage            = p_  ? p_->getAttentionStage() : 0;
  uint32_t freeHeap    = ESP.getFreeHeap();

  snprintf(buf, sizeof(buf),
    "{\"emotion\":%d,\"emotionName\":\"%s\","
    "\"batteryVoltage\":%.2f,\"uptimeMs\":%lu,"
    "\"attentionStage\":%d,\"freeHeap\":%u}",
    emotionId, emoName, voltage, millis(), stage, freeHeap);

  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.send(200, "application/json", buf);
}

// POST /api/emotion body: emotion=N — sets the active emotion by enum index.
void WebServerManager::handleApiEmotion() {
  if (!server_.hasArg("emotion")) {
    server_.send(400, "application/json", "{\"error\":\"missing emotion\"}");
    return;
  }
  int id = server_.arg("emotion").toInt();
  if (!emotionRegistry.get((EmotionState)id)) {
    server_.send(400, "application/json", "{\"error\":\"invalid emotion id\"}");
    return;
  }
  if (onEmotionSet_) onEmotionSet_((EmotionState)id);
  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/gesture body: gesture=tap|long|double — injects a touch gesture.
void WebServerManager::handleApiGesture() {
  if (!server_.hasArg("gesture")) {
    server_.send(400, "application/json", "{\"error\":\"missing gesture\"}");
    return;
  }
  String g = server_.arg("gesture");
  TouchGesture gesture = GESTURE_NONE;
  if      (g == "tap")    gesture = GESTURE_TAP;
  else if (g == "long")   gesture = GESTURE_LONG_PRESS;
  else if (g == "double") gesture = GESTURE_DOUBLE_TAP;
  else {
    server_.send(400, "application/json", "{\"error\":\"unknown gesture\"}");
    return;
  }
  if (onGesture_) onGesture_(gesture);
  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.send(200, "application/json", "{\"ok\":true}");
}

// GET /api/config — returns all runtimeConfig fields as JSON.
void WebServerManager::handleApiConfigGet() {
  if (!cfg_) {
    server_.send(503, "application/json", "{\"error\":\"config unavailable\"}");
    return;
  }
  char buf[320];
  snprintf(buf, sizeof(buf),
    "{\"attentionStage1Ms\":%lu,\"attentionStage2Ms\":%lu,"
    "\"attentionStage3Ms\":%lu,\"attentionStage4Ms\":%lu,"
    "\"moodDriftIntervalMs\":%lu,\"microExpressionChance\":%u,"
    "\"jitterPercent\":%u}",
    cfg_->attentionStage1Ms, cfg_->attentionStage2Ms,
    cfg_->attentionStage3Ms, cfg_->attentionStage4Ms,
    cfg_->moodDriftIntervalMs,
    cfg_->microExpressionChance,
    cfg_->jitterPercent);

  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.send(200, "application/json", buf);
}

// POST /api/config — updates any subset of runtimeConfig fields and saves to NVS.
void WebServerManager::handleApiConfigPost() {
  if (!cfg_) {
    server_.send(503, "application/json", "{\"error\":\"config unavailable\"}");
    return;
  }
  if (server_.hasArg("attentionStage1Ms"))
    cfg_->attentionStage1Ms   = (unsigned long)server_.arg("attentionStage1Ms").toInt();
  if (server_.hasArg("attentionStage2Ms"))
    cfg_->attentionStage2Ms   = (unsigned long)server_.arg("attentionStage2Ms").toInt();
  if (server_.hasArg("attentionStage3Ms"))
    cfg_->attentionStage3Ms   = (unsigned long)server_.arg("attentionStage3Ms").toInt();
  if (server_.hasArg("attentionStage4Ms"))
    cfg_->attentionStage4Ms   = (unsigned long)server_.arg("attentionStage4Ms").toInt();
  if (server_.hasArg("moodDriftIntervalMs"))
    cfg_->moodDriftIntervalMs = (unsigned long)server_.arg("moodDriftIntervalMs").toInt();
  if (server_.hasArg("microExpressionChance")) {
    int v = server_.arg("microExpressionChance").toInt();
    cfg_->microExpressionChance = (uint8_t)(v < 0 ? 0 : v > 100 ? 100 : v);
  }
  if (server_.hasArg("jitterPercent")) {
    int v = server_.arg("jitterPercent").toInt();
    cfg_->jitterPercent = (uint8_t)(v < 0 ? 0 : v > 50 ? 50 : v);
  }
  runtimeConfigSave();
  Serial.printf("[WEB] Config saved\n");
  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.send(200, "application/json", "{\"ok\":true}");
}

// POST /api/config/reset — resets personality params to config.h defaults.
void WebServerManager::handleApiConfigReset() {
  runtimeConfigReset();
  Serial.printf("[WEB] Config reset to defaults\n");
  server_.sendHeader("Access-Control-Allow-Origin", "*");
  server_.send(200, "application/json", "{\"ok\":true}");
}

// 404 handler for unregistered routes.
void WebServerManager::handleNotFound() {
  server_.send(404, "text/plain", "Not found");
}

#endif // NATIVE_BUILD
