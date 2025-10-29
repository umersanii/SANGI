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

// ===== GITHUB CONTRIBUTION STRUCTURE =====
// Stores GitHub contribution heat map data (52 weeks x 7 days)
struct GitHubContributionData {
  uint8_t contributions[52][7];  // 52 weeks x 7 days grid (0-4+ scale)
  int totalContributions;        // Total count for the year
  int currentStreak;             // Current consecutive days
  int longestStreak;             // Longest streak this year
  char username[32];             // GitHub username
  bool dataLoaded;               // Data is valid
};

// ===== GITHUB STATS STRUCTURE =====
// Stores current GitHub profile statistics (from GitHub API)
struct GitHubStatsData {
  char username[32];             // GitHub username
  int repos;                     // Public repositories
  int followers;                 // Follower count
  int following;                 // Following count
  int contributions;             // Total contributions (last year)
  int commits;                   // Total commits (last year)
  int prs;                       // Total pull requests
  int issues;                    // Total issues
  int stars;                     // Total stars received
  unsigned long timestamp;       // Last update time
  bool dataLoaded;               // Data is valid
};

// Maximum notifications in queue
#define MAX_NOTIFICATION_QUEUE 5

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
  void publishBattery(float voltage, bool charging = false);
  void publishSystemStatus(unsigned long uptime, uint32_t heap, int rssi);
  void publishEmotionChange(int emotionState);
  
  // Serial logging (buffered, sends every 5 seconds)
  void log(const char* message);
  void logDebug(const char* message);
  void logInfo(const char* message);
  void logWarn(const char* message);
  void logError(const char* message);
  void flushLogs();  // Force send buffered logs immediately
  
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
  
  // GitHub contribution data management
  void setGitHubContributions(const uint8_t contributions[52][7], int total, int streak, int longest, const char* user);
  GitHubContributionData* getGitHubData();
  bool hasGitHubData() const;
  void clearGitHubData();
  
  // GitHub stats data management
  void setGitHubStats(const char* user, int repos, int followers, int following, 
                      int contributions, int commits, int prs, int issues, int stars);
  GitHubStatsData* getGitHubStats();
  bool hasGitHubStats() const;
  
  // Offline mode detection
  bool isInWorkspaceMode() const { return workspaceMode; }
  unsigned long getLastMQTTMessageTime() const { return lastMQTTMessageTime; }
  
private:
  WiFiClientSecure wifiClient;
  PubSubClient mqttClient;
  NetworkState currentState;
  unsigned long lastReconnectAttempt;
  unsigned long lastStatusPublish;
  
  // Serial log buffering (sends every 5 seconds)
  static const int MAX_LOG_BUFFER_SIZE = 512;  // Max characters per batch
  char logBuffer[MAX_LOG_BUFFER_SIZE];
  int logBufferPos;
  unsigned long lastLogFlush;
  static const unsigned long LOG_FLUSH_INTERVAL = 5000;  // 5 seconds
  
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
  
  // GitHub contribution data
  GitHubContributionData githubData;
  
  // GitHub stats data
  GitHubStatsData githubStats;
  
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
