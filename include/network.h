// NetworkManager — slim coordinator wrapping MqttManager, NotificationQueue,
// and GitHubDataStore.  Phase 3 refactoring moved the bulk of the logic into
// those focused components; this class wires them together and provides the
// serial-log buffering + periodic publishing that lives on top of MQTT.

#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "mqtt_manager.h"
#include "notification_queue.h"
#include "github_data.h"

class NetworkManager {
public:
  NetworkManager();

  // Initialization and connection
  bool init();
  void update();

  // Accessors for sub-components
  MqttManager& mqtt() { return mqttManager; }
  NotificationQueue& notifications() { return notifications_; }
  GitHubDataStore& github() { return githubData_; }
  const GitHubDataStore& github() const { return githubData_; }

  // Publishing (convenience wrappers over mqttManager.publish)
  void publishStatus(const char* status);
  void publishBattery(float voltage, bool charging = false);
  void publishSystemStatus(unsigned long uptime, uint32_t heap, int rssi);
  void publishEmotionChange(int emotionState);

  // Serial logging (buffered, sends every 5 seconds)
  void log(const char* message);
  void logDebug(const char* message);
  void logInfo(const char* message);
  void logWarn(const char* message);
  void logError(const char* message);
  void flushLogs();

  // Offline mode detection
  bool isInWorkspaceMode() const { return workspaceMode_; }
  unsigned long getLastMQTTMessageTime() const { return lastMQTTMessageTime_; }
  void setWorkspaceMode(bool mode) { workspaceMode_ = mode; }
  void updateLastMQTTMessageTime() { lastMQTTMessageTime_ = millis(); }

  // SSID validation
  bool validateSSID(const char* receivedSSID) const;

private:
  NotificationQueue notifications_;
  GitHubDataStore githubData_;

  // Serial log buffering
  static const int MAX_LOG_BUFFER_SIZE = 512;
  char logBuffer_[MAX_LOG_BUFFER_SIZE];
  int logBufferPos_;
  unsigned long lastLogFlush_;
  static const unsigned long LOG_FLUSH_INTERVAL = 5000;

  // Publishing timing
  unsigned long lastStatusPublish_;

  // Offline mode detection
  bool workspaceMode_;
  unsigned long lastMQTTMessageTime_;
};

extern NetworkManager networkManager;

#endif // NETWORK_H
