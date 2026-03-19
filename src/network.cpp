// NetworkManager — slim coordinator.
// Phase 3: MQTT connection, message dispatch, WiFi, diagnostics, and
// subscriptions moved to MqttManager.  Notification queue and GitHub data
// moved to their own classes.  What remains here is log-buffering,
// periodic publishing, and the wiring between sub-components.

#include "network.h"
#include "emotion.h"
#include "emotion_registry.h"
#include "battery.h"

#if ENABLE_MQTT
#include "secrets.h"
#endif

// ===== GLOBAL INSTANCE =====
NetworkManager networkManager;

// ===== CONSTRUCTOR =====
NetworkManager::NetworkManager()
  : logBufferPos_(0),
    lastLogFlush_(0),
    lastStatusPublish_(0),
    workspaceMode_(false),
    lastMQTTMessageTime_(0) {
  memset(logBuffer_, 0, MAX_LOG_BUFFER_SIZE);
}

// ===== HARDCODED GITHUB CONTRIBUTION DATA (debug only) =====
#if DEBUG_MODE_ENABLED
void loadHardcodedGitHubData() {
  networkManager.github().clearContributions();

  uint8_t sampleContributions[52][7];
  memset(sampleContributions, 0, sizeof(sampleContributions));

  for (int week = 0; week < 5; week++)
    for (int day = 0; day < 7; day++)
      if (week >= 1 && (day == 2 || day == 3 || day == 4))
        sampleContributions[week][day] = (week % 2 == 0) ? 2 : 1;

  for (int week = 5; week < 9; week++)
    for (int day = 0; day < 7; day++)
      if (day >= 1 && day <= 5)
        sampleContributions[week][day] = (day % 2 == 0) ? 3 : 2;

  for (int week = 9; week < 17; week++)
    for (int day = 0; day < 7; day++)
      if (day >= 1 && day <= 4)
        sampleContributions[week][day] = (week % 3 == 0) ? 2 : 1;

  for (int week = 17; week < 26; week++)
    for (int day = 0; day < 7; day++)
      if (day >= 0 && day <= 5)
        sampleContributions[week][day] = ((week + day) % 4 == 0) ? 4 : 3;

  for (int week = 26; week < 35; week++)
    for (int day = 0; day < 7; day++)
      if (day >= 1 && day <= 6)
        sampleContributions[week][day] = (day % 2 == 0) ? 4 : 3;

  for (int week = 35; week < 44; week++)
    for (int day = 0; day < 7; day++)
      if (day >= 0 && day <= 6)
        sampleContributions[week][day] = ((week + day) % 3 == 0) ? 4 : 3;

  for (int week = 44; week < 52; week++)
    for (int day = 0; day < 7; day++)
      if (day >= 1 && day <= 5)
        sampleContributions[week][day] = (day % 2 == 0) ? 4 : 3;

  networkManager.github().setContributions(
    sampleContributions, 397, 15, 45, "umersanii");

  Serial.println("Loaded hardcoded GitHub contribution data for testing");
}
#endif

// ===== INITIALIZATION =====
bool NetworkManager::init() {
#if !ENABLE_MQTT
  Serial.println("MQTT disabled in config.h - skipping network init");
  return false;
#else
  return mqttManager.init(AWS_IOT_ENDPOINT, 8883,
                          AWS_CERT_CA, AWS_CERT_CRT, AWS_CERT_PRIVATE,
                          THINGNAME);
#endif
}

// ===== UPDATE LOOP =====
void NetworkManager::update() {
#if ENABLE_MQTT
  unsigned long currentTime = millis();

  // Update MQTT (handles WiFi reconnect, MQTT reconnect, message loop)
  mqttManager.update();

  // Auto-flush serial logs every 5 seconds
  bool logFlushOverflow = currentTime < lastLogFlush_;
  if (logBufferPos_ > 0 && (logFlushOverflow || (currentTime - lastLogFlush_ >= LOG_FLUSH_INTERVAL))) {
    flushLogs();
  }

  // Handle publish timing overflow
  bool publishOverflow = currentTime < lastStatusPublish_;

  // Periodic status publishing (every 90 seconds)
  if (publishOverflow || (currentTime - lastStatusPublish_ > STATUS_PUBLISH_INTERVAL * 3)) {
    float voltage = batteryManager.readVoltage();
    publishBattery(voltage, false);
    publishSystemStatus(currentTime / 1000, ESP.getFreeHeap(), WiFi.RSSI());
    publishEmotionChange(emotionManager.getCurrentEmotion());
    lastStatusPublish_ = currentTime;
  }
#endif
}

// ===== SSID VALIDATION =====
bool NetworkManager::validateSSID(const char* receivedSSID) const {
  if (!receivedSSID || strlen(receivedSSID) == 0) {
    return false;
  }

  const char* connectedSSID = mqttManager.getConnectedSSID();
  bool matches = (strcmp(connectedSSID, receivedSSID) == 0);

  if (matches) {
    Serial.printf("SSID validated: %s\n", receivedSSID);
  } else {
    Serial.printf("SSID mismatch: expected '%s', got '%s'\n", connectedSSID, receivedSSID);
  }

  return matches;
}

// ===== PUBLISHING FUNCTIONS =====

void NetworkManager::publishStatus(const char* status) {
#if ENABLE_MQTT
  if (!mqttManager.isConnected()) return;

  StaticJsonDocument<200> doc;
  doc["status"] = status;
  doc["device"] = THINGNAME;
  doc["timestamp"] = millis();

  char buffer[256];
  serializeJson(doc, buffer);

  if (mqttManager.publish(MQTT_TOPIC_STATUS, buffer)) {
    Serial.print("Published status: ");
    Serial.println(status);
  }
#endif
}

void NetworkManager::publishBattery(float voltage, bool charging) {
#if ENABLE_MQTT
  if (!mqttManager.isConnected()) return;

  StaticJsonDocument<200> doc;
  doc["voltage"] = String(voltage, 2).toFloat();
  doc["level"] = ((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100;
  doc["charging"] = charging;

  char buffer[256];
  serializeJson(doc, buffer);

  mqttManager.publish(MQTT_TOPIC_BATTERY, buffer);
#endif
}

void NetworkManager::publishSystemStatus(unsigned long uptime, uint32_t heap, int rssi) {
#if ENABLE_MQTT
  if (!mqttManager.isConnected()) return;

  StaticJsonDocument<200> doc;
  doc["uptime"] = uptime;
  doc["heap"] = heap;
  doc["rssi"] = rssi;

  char buffer[256];
  serializeJson(doc, buffer);

  mqttManager.publish(MQTT_TOPIC_SYSTEM, buffer);
#endif
}

void NetworkManager::publishEmotionChange(int emotionState) {
#if ENABLE_MQTT
  if (!mqttManager.isConnected()) return;

  StaticJsonDocument<200> doc;
  doc["current"] = emotionRegistry.getName((EmotionState)emotionState);
  doc["timestamp"] = millis();

  char buffer[256];
  serializeJson(doc, buffer);

  mqttManager.publish(MQTT_TOPIC_EMOTION, buffer);
#endif
}

// ===== SERIAL LOG BUFFERING =====
void NetworkManager::log(const char* message) {
  Serial.println(message);

#if ENABLE_MQTT
  if (!message) return;

  int msgLen = strlen(message);

  if (logBufferPos_ + msgLen + 2 > MAX_LOG_BUFFER_SIZE - 1) {
    flushLogs();
  }

  if (logBufferPos_ > 0) {
    logBuffer_[logBufferPos_++] = '\n';
  }
  strncpy(logBuffer_ + logBufferPos_, message, MAX_LOG_BUFFER_SIZE - logBufferPos_ - 1);
  logBufferPos_ += msgLen;
  logBuffer_[logBufferPos_] = '\0';
#endif
}

void NetworkManager::logDebug(const char* message) {
  char prefixed[256];
  snprintf(prefixed, sizeof(prefixed), "[DEBUG] %s", message);
  log(prefixed);
}

void NetworkManager::logInfo(const char* message) {
  char prefixed[256];
  snprintf(prefixed, sizeof(prefixed), "[INFO] %s", message);
  log(prefixed);
}

void NetworkManager::logWarn(const char* message) {
  char prefixed[256];
  snprintf(prefixed, sizeof(prefixed), "[WARN] %s", message);
  log(prefixed);
}

void NetworkManager::logError(const char* message) {
  char prefixed[256];
  snprintf(prefixed, sizeof(prefixed), "[ERROR] %s", message);
  log(prefixed);
}

void NetworkManager::flushLogs() {
#if ENABLE_MQTT
  if (!mqttManager.isConnected() || logBufferPos_ == 0) return;

  StaticJsonDocument<600> doc;
  doc["line"] = logBuffer_;
  doc["timestamp"] = millis();

  char jsonBuffer[650];
  serializeJson(doc, jsonBuffer);

  if (mqttManager.publish(MQTT_TOPIC_SERIAL_LOGS, jsonBuffer)) {
    logBufferPos_ = 0;
    memset(logBuffer_, 0, MAX_LOG_BUFFER_SIZE);
    lastLogFlush_ = millis();
  }
#endif
}
