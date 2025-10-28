#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Forward declaration for callback
class NetworkManager;

// ===== NOTIFICATION TYPES =====
enum NotificationType {
  NOTIFY_GENERIC,
  NOTIFY_DISCORD,
  NOTIFY_SLACK,
  NOTIFY_EMAIL,
  NOTIFY_GITHUB,
  NOTIFY_CALENDAR,
  NOTIFY_SYSTEM
};

// ===== NOTIFICATION STRUCTURE =====
struct Notification {
  NotificationType type;
  char title[32];
  char message[64];
  unsigned long timestamp;
  bool active;
};

// ===== GITHUB COMMIT STRUCTURE =====
struct CommitHistoryEntry {
  char repo[32];          // Repository name (e.g., "SANGI")
  char message[64];       // Commit message
  char author[24];        // Author name
  char sha[16];           // Short commit SHA (first 7 chars)
  unsigned long timestamp; // Commit timestamp
  bool active;            // Entry is valid
};

// Maximum notifications in queue
#define MAX_NOTIFICATION_QUEUE 5

// Maximum commit history entries
#define MAX_COMMIT_HISTORY 10

// ===== NETWORK STATE =====
enum NetworkState {
  NET_DISCONNECTED,
  NET_WIFI_CONNECTING,
  NET_WIFI_CONNECTED,
  NET_MQTT_CONNECTING,
  NET_MQTT_CONNECTED,
  NET_ERROR
};

// ===== NETWORK MANAGER =====
class NetworkManager {
public:
  NetworkManager();
  
  // Initialization and connection
  bool init();
  void update();
  
  // WiFi management
  bool connectWiFi();
  bool isWiFiConnected() const;
  
  // MQTT management
  bool connectMQTT();
  bool isMQTTConnected() const;
  void mqttLoop();
  
  // Publishing
  void publishStatus(const char* status);
  void publishBattery(float voltage);
  void publishUptime(unsigned long seconds);
  void publishEmotionChange(int emotionState);
  
  // State getters
  NetworkState getState() const { return currentState; }
  unsigned long getLastReconnectAttempt() const { return lastReconnectAttempt; }
  
  // Network diagnostics
  bool pingEndpoint(const char* hostname);
  void testConnectivity();
  
  // Callback for MQTT messages
  static void messageCallback(char* topic, byte* payload, unsigned int length);
  
  // Workspace activity tracking
  void handleWorkspaceActivity(const char* device, int activityScore);
  int getCombinedActivityScore() const { return combinedActivityScore; }
  
  // Notification queue management
  bool addNotification(NotificationType type, const char* title, const char* message);
  bool hasNotifications() const { return notificationCount > 0; }
  Notification* getCurrentNotification();
  void clearCurrentNotification();
  int getNotificationCount() const { return notificationCount; }
  
  // Commit history management
  bool addCommit(const char* repo, const char* message, const char* author, const char* sha, unsigned long timestamp);
  bool hasCommits() const { return commitCount > 0; }
  CommitHistoryEntry* getCommitAtIndex(int index);
  int getCommitCount() const { return commitCount; }
  void clearCommitHistory();
  
  // Offline mode detection
  bool isInWorkspaceMode() const { return workspaceMode; }
  unsigned long getLastMQTTMessageTime() const { return lastMQTTMessageTime; }
  
private:
  WiFiClientSecure wifiClient;
  PubSubClient mqttClient;
  NetworkState currentState;
  unsigned long lastReconnectAttempt;
  unsigned long lastStatusPublish;
  
  // Workspace activity state
  int pcActivityScore;
  int piActivityScore;
  int combinedActivityScore;
  unsigned long lastPcActivity;
  unsigned long lastPiActivity;
  bool pcOnline;
  bool piOnline;
  
  // Notification queue
  Notification notificationQueue[MAX_NOTIFICATION_QUEUE];
  int notificationCount;
  int currentNotificationIndex;
  
  // Commit history storage
  CommitHistoryEntry commitHistory[MAX_COMMIT_HISTORY];
  int commitCount;
  
  // Offline mode detection
  bool workspaceMode;
  unsigned long lastMQTTMessageTime;
  char connectedSSID[33];  // Current WiFi SSID (max 32 chars + null terminator)
  
  void setupTime();
  void handleIncomingMessage(const char* topic, const char* payload);
  bool validateSSID(const char* receivedSSID);
};

extern NetworkManager networkManager;

#endif // NETWORK_H
