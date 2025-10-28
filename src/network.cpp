#include "network.h"
#include "emotion.h"
#include "battery.h"
#include <time.h>

#if ENABLE_MQTT
#include "secrets.h"
#endif

// ===== GLOBAL INSTANCE =====
NetworkManager networkManager;

// ===== CONSTRUCTOR =====
NetworkManager::NetworkManager() 
  : mqttClient(wifiClient),
    currentState(NET_DISCONNECTED),
    lastReconnectAttempt(0),
    lastStatusPublish(0),
    pcActivityScore(0),
    piActivityScore(0),
    combinedActivityScore(0),
    lastPcActivity(0),
    lastPiActivity(0),
    pcOnline(false),
    piOnline(false),
    notificationCount(0),
    currentNotificationIndex(0),
    workspaceMode(false),
    lastMQTTMessageTime(0) {
  // Initialize notification queue
  for (int i = 0; i < MAX_NOTIFICATION_QUEUE; i++) {
    notificationQueue[i].active = false;
  }
  
  // Initialize GitHub data
  githubData.dataLoaded = false;
  githubData.totalContributions = 0;
  githubData.currentStreak = 0;
  githubData.longestStreak = 0;
  memset(githubData.username, 0, sizeof(githubData.username));
  memset(githubData.contributions, 0, sizeof(githubData.contributions));
  
  // Initialize SSID storage
  memset(connectedSSID, 0, sizeof(connectedSSID));
}

// ===== INITIALIZATION =====
bool NetworkManager::init() {
#if !ENABLE_MQTT
  Serial.println("MQTT disabled in config.h - skipping network init");
  currentState = NET_DISCONNECTED;
  return false;
#else
  Serial.println("\n=== Network Manager Initializing ===");
  
  // Configure WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  
  // Setup MQTT client
  mqttClient.setServer(AWS_IOT_ENDPOINT, 8883);
  mqttClient.setCallback(NetworkManager::messageCallback);
  mqttClient.setBufferSize(1024);  // Increase from default 256 to 1024 for AWS IoT
  mqttClient.setKeepAlive(60);
  mqttClient.setSocketTimeout(30);
  
  // Configure certificates for AWS IoT Core
  wifiClient.setCACert(AWS_CERT_CA);
  wifiClient.setCertificate(AWS_CERT_CRT);
  wifiClient.setPrivateKey(AWS_CERT_PRIVATE);
  
  // Connect WiFi
  if (!connectWiFi()) {
    Serial.println("WiFi connection failed");
    currentState = NET_ERROR;
    return false;
  }
  
  // Sync time (required for AWS IoT Core)
  setupTime();
  
  // Connect MQTT
  if (!connectMQTT()) {
    Serial.println("MQTT connection failed (will retry)");
    currentState = NET_MQTT_CONNECTING;
    return false;
  }
  
  Serial.println("=== Network Manager Ready ===\n");
  return true;
#endif
}

// ===== HARDCODED GITHUB CONTRIBUTION DATA (for testing) =====
// This will be replaced by real GitHub API data from Pi service via MQTT
void loadHardcodedGitHubData() {
  // Clear existing data
  networkManager.clearGitHubData();
  
  // Sample contribution data simulating a year of activity
  // Values: 0 = no contributions, 1 = 1-3 contributions, 2 = 4-6, 3 = 7-9, 4 = 10+
  uint8_t sampleContributions[52][7];
  
  // Initialize all to 0
  memset(sampleContributions, 0, sizeof(sampleContributions));
  
  // Based on your graph: concentrated activity in certain periods
  // Simulating the pattern from your screenshot
  
  // December 2024 - sparse activity
  for (int week = 0; week < 5; week++) {
    for (int day = 0; day < 7; day++) {
      if (week >= 1 && (day == 2 || day == 3 || day == 4)) {
        sampleContributions[week][day] = (week % 2 == 0) ? 2 : 1;
      }
    }
  }
  
  // January - increasing activity
  for (int week = 5; week < 9; week++) {
    for (int day = 0; day < 7; day++) {
      if (day >= 1 && day <= 5) {
        sampleContributions[week][day] = (day % 2 == 0) ? 3 : 2;
      }
    }
  }
  
  // February-March - moderate activity
  for (int week = 9; week < 17; week++) {
    for (int day = 0; day < 7; day++) {
      if (day >= 1 && day <= 4) {
        sampleContributions[week][day] = (week % 3 == 0) ? 2 : 1;
      }
    }
  }
  
  // April-May - peak activity
  for (int week = 17; week < 26; week++) {
    for (int day = 0; day < 7; day++) {
      if (day >= 0 && day <= 5) {
        sampleContributions[week][day] = ((week + day) % 4 == 0) ? 4 : 3;
      }
    }
  }
  
  // June-July - high activity
  for (int week = 26; week < 35; week++) {
    for (int day = 0; day < 7; day++) {
      if (day >= 1 && day <= 6) {
        sampleContributions[week][day] = (day % 2 == 0) ? 4 : 3;
      }
    }
  }
  
  // August-September - very high activity
  for (int week = 35; week < 44; week++) {
    for (int day = 0; day < 7; day++) {
      if (day >= 0 && day <= 6) {
        sampleContributions[week][day] = ((week + day) % 3 == 0) ? 4 : 3;
      }
    }
  }
  
  // October - recent high activity
  for (int week = 44; week < 52; week++) {
    for (int day = 0; day < 7; day++) {
      if (day >= 1 && day <= 5) {
        sampleContributions[week][day] = (day % 2 == 0) ? 4 : 3;
      }
    }
  }
  
  // Set the data with stats
  networkManager.setGitHubContributions(
    sampleContributions,
    397,  // Total contributions from your graph
    15,   // Current streak (sample)
    45    // Longest streak (sample)
    , "umersanii"
  );
  
  Serial.println("Loaded hardcoded GitHub contribution data for testing");
  Serial.printf("Total: 397 contributions | Current streak: 15 days\n");
}

// ===== WiFi CONNECTION =====
bool NetworkManager::connectWiFi() {
#if !ENABLE_MQTT
  return false;
#else
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  currentState = NET_WIFI_CONNECTING;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < NETWORK_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection timeout");
    currentState = NET_ERROR;
    return false;
  }
  
  currentState = NET_WIFI_CONNECTED;
  Serial.print("WiFi connected! IP: ");
  Serial.println(WiFi.localIP());
  
  // Store connected SSID for validation
  String ssid = WiFi.SSID();
  strncpy(connectedSSID, ssid.c_str(), 32);
  connectedSSID[32] = '\0';
  Serial.print("Connected SSID: ");
  Serial.println(connectedSSID);
  
  return true;
#endif
}

// ===== MQTT CONNECTION =====
bool NetworkManager::connectMQTT() {
#if !ENABLE_MQTT
  return false;
#else
  if (!isWiFiConnected()) {
    Serial.println("WiFi not connected - cannot connect MQTT");
    return false;
  }
  
  Serial.print("Connecting to AWS IoT Core as: ");
  Serial.println(THINGNAME);
  
  currentState = NET_MQTT_CONNECTING;
  
  // Attempt MQTT connection
  if (mqttClient.connect(THINGNAME)) {
    Serial.println("MQTT connected!");
    currentState = NET_MQTT_CONNECTED;
    
    // Subscribe to emotion control topic
    if (mqttClient.subscribe(MQTT_TOPIC_EMOTION_SET)) {
      Serial.print("Subscribed to: ");
      Serial.println(MQTT_TOPIC_EMOTION_SET);
    } else {
      Serial.println("Failed to subscribe to emotion topic");
    }
    
    // Subscribe to notification topic
    if (mqttClient.subscribe("sangi/notification/push")) {
      Serial.println("Subscribed to: sangi/notification/push");
    } else {
      Serial.println("Failed to subscribe to notification topic");
    }
    
    // Subscribe to GitHub commit history topic
    if (mqttClient.subscribe(MQTT_TOPIC_GITHUB_COMMITS)) {
      Serial.print("Subscribed to: ");
      Serial.println(MQTT_TOPIC_GITHUB_COMMITS);
    } else {
      Serial.println("Failed to subscribe to commit history topic");
    }
    
    // Don't publish immediately - causes disconnects
    // Will publish on first status update
    
    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqttClient.state());
    currentState = NET_ERROR;
    return false;
  }
#endif
}

// ===== TIME SYNCHRONIZATION =====
void NetworkManager::setupTime() {
#if ENABLE_MQTT
  Serial.println("Synchronizing time with NTP...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  time_t now = time(nullptr);
  int retries = 0;
  while (now < 8 * 3600 * 2 && retries < 15) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    retries++;
  }
  Serial.println();
  
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.print("Time synchronized: ");
    Serial.println(asctime(&timeinfo));
  } else {
    Serial.println("Failed to sync time (continuing anyway)");
  }
#endif
}

// ===== UPDATE LOOP =====
void NetworkManager::update() {
#if ENABLE_MQTT
  unsigned long currentTime = millis();
  
  // Check WiFi status
  if (!isWiFiConnected()) {
    // Only log once to avoid spam
    static bool wifiDisconnectLogged = false;
    if (!wifiDisconnectLogged) {
      Serial.println("WiFi disconnected - attempting reconnect");
      wifiDisconnectLogged = true;
    }
    
    if (connectWiFi()) {
      connectMQTT();
      wifiDisconnectLogged = false;
    }
    return;
  }
  
  // Check MQTT status and reconnect if needed
  if (!isMQTTConnected()) {
    // Handle millis() overflow
    bool overflow = currentTime < lastReconnectAttempt;
    
    if (overflow || (currentTime - lastReconnectAttempt > MQTT_RECONNECT_INTERVAL)) {
      Serial.println("MQTT disconnected - attempting reconnect");
      lastReconnectAttempt = currentTime;
      connectMQTT();
    }
    return;
  }
  
  // Process MQTT messages - THIS IS CRITICAL!
  mqttClient.loop();
  
  // Handle publish timing overflow
  bool publishOverflow = currentTime < lastStatusPublish;
  
  // Periodic status publishing (reduced frequency)
  if (publishOverflow || (currentTime - lastStatusPublish > STATUS_PUBLISH_INTERVAL * 3)) {  // Every 90 seconds
    float voltage = batteryManager.readVoltage();
    publishBattery(voltage);
    publishEmotionChange(emotionManager.getCurrentEmotion());
    lastStatusPublish = currentTime;
  }
#endif
}

// ===== PUBLISHING FUNCTIONS =====
void NetworkManager::publishStatus(const char* status) {
#if ENABLE_MQTT
  if (!isMQTTConnected()) return;
  
  StaticJsonDocument<200> doc;
  doc["status"] = status;
  doc["device"] = THINGNAME;
  doc["timestamp"] = millis();
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  if (mqttClient.publish(MQTT_TOPIC_STATUS, buffer)) {
    Serial.print("Published status: ");
    Serial.println(status);
  }
#endif
}

void NetworkManager::publishBattery(float voltage) {
#if ENABLE_MQTT
  if (!isMQTTConnected()) return;
  
  StaticJsonDocument<200> doc;
  doc["voltage"] = voltage;
  doc["percentage"] = ((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100;
  doc["timestamp"] = millis();
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  mqttClient.publish(MQTT_TOPIC_BATTERY, buffer);
#endif
}

void NetworkManager::publishUptime(unsigned long seconds) {
#if ENABLE_MQTT
  if (!isMQTTConnected()) return;
  
  StaticJsonDocument<200> doc;
  doc["uptime_seconds"] = seconds;
  doc["timestamp"] = millis();
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  mqttClient.publish(MQTT_TOPIC_UPTIME, buffer);
#endif
}

void NetworkManager::publishEmotionChange(int emotionState) {
#if ENABLE_MQTT
  if (!isMQTTConnected()) return;
  
  StaticJsonDocument<200> doc;
  doc["emotion"] = emotionState;
  doc["timestamp"] = millis();
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  mqttClient.publish("sangi/emotion/current", buffer);
#endif
}

// ===== MQTT MESSAGE CALLBACK =====
void NetworkManager::messageCallback(char* topic, byte* payload, unsigned int length) {
  // Create null-terminated string from payload
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  Serial.print("MQTT message received on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(message);
  
  // Pass to instance handler
  networkManager.handleIncomingMessage(topic, message);
}

// ===== SSID VALIDATION =====
bool NetworkManager::validateSSID(const char* receivedSSID) {
  if (!receivedSSID || strlen(receivedSSID) == 0) {
    return false;  // No SSID in message - might be from different source
  }
  
  bool matches = (strcmp(connectedSSID, receivedSSID) == 0);
  
  if (matches) {
    Serial.printf("✅ SSID validated: %s\n", receivedSSID);
  } else {
    Serial.printf("⚠️  SSID mismatch: expected '%s', got '%s'\n", connectedSSID, receivedSSID);
  }
  
  return matches;
}

// ===== MESSAGE HANDLER =====
void NetworkManager::handleIncomingMessage(const char* topic, const char* payload) {
  // Validate inputs
  if (!topic || !payload) {
    Serial.println("ERROR: Null topic or payload");
    return;
  }
  
  // Parse JSON payload with larger buffer
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Validate SSID if present in message
  if (doc.containsKey("ssid")) {
    const char* receivedSSID = doc["ssid"];
    if (!validateSSID(receivedSSID)) {
      Serial.println("🚫 SSID validation failed - message ignored");
      workspaceMode = false;
      return;
    }
    workspaceMode = true;
    lastMQTTMessageTime = millis();
  }
  
  // Handle emotion set command
  if (strcmp(topic, MQTT_TOPIC_EMOTION_SET) == 0) {
    if (doc.containsKey("emotion")) {
      int emotionValue = doc["emotion"];
      
      Serial.printf(">>> MQTT EMOTION: %d <<<\n", emotionValue);
      
      // Validate emotion range (0-14) - matches EmotionState enum
      if (emotionValue >= EMOTION_IDLE && emotionValue <= EMOTION_CODING) {
        emotionManager.setTargetEmotion((EmotionState)emotionValue);
        Serial.printf("✓ Emotion set to: %d\n", emotionValue);
      } else {
        Serial.printf("✗ Invalid emotion: %d (valid range: 0-14)\n", emotionValue);
      }
    } else {
      Serial.println("✗ Missing 'emotion' field in JSON");
    }
  }
  // Handle notification push
  else if (strcmp(topic, "sangi/notification/push") == 0) {
    NotificationType notifType = NOTIFY_GENERIC;
    const char* title = "";
    const char* message = "";
    
    // Parse notification type
    if (doc.containsKey("type")) {
      const char* typeStr = doc["type"];
      if (strcmp(typeStr, "discord") == 0) notifType = NOTIFY_DISCORD;
      else if (strcmp(typeStr, "slack") == 0) notifType = NOTIFY_SLACK;
      else if (strcmp(typeStr, "email") == 0) notifType = NOTIFY_EMAIL;
      else if (strcmp(typeStr, "github") == 0) notifType = NOTIFY_GITHUB;
      else if (strcmp(typeStr, "calendar") == 0) notifType = NOTIFY_CALENDAR;
      else if (strcmp(typeStr, "system") == 0) notifType = NOTIFY_SYSTEM;
    }
    
    if (doc.containsKey("title")) {
      title = doc["title"];
    }
    
    if (doc.containsKey("message")) {
      message = doc["message"];
    }
    
    // Add to notification queue
    if (addNotification(notifType, title, message)) {
      Serial.printf("📩 Notification queued: [%d] %s - %s\n", notifType, title, message);
      
      // Trigger notification emotion if not already showing one
      if (emotionManager.getCurrentEmotion() != EMOTION_NOTIFICATION) {
        emotionManager.setTargetEmotion(EMOTION_NOTIFICATION);
      }
    } else {
      Serial.println("⚠️  Notification queue full - dropped");
    }
  }
  // Handle GitHub contribution data updates
  else if (strcmp(topic, MQTT_TOPIC_GITHUB_COMMITS) == 0) {
    // Expected JSON format:
    // {
    //   "username": "umersanii",
    //   "total": 397,
    //   "current_streak": 15,
    //   "longest_streak": 45,
    //   "contributions": [[0,1,2,3,2,1,0], [1,2,3,4,3,2,1], ...] // 52 weeks x 7 days
    // }
    
    if (doc.containsKey("contributions") && doc["contributions"].is<JsonArray>()) {
      const char* username = doc["username"] | "user";
      int total = doc["total"] | 0;
      int currentStreak = doc["current_streak"] | 0;
      int longestStreak = doc["longest_streak"] | 0;
      
      JsonArray weeksArray = doc["contributions"].as<JsonArray>();
      
      uint8_t contributions[52][7];
      memset(contributions, 0, sizeof(contributions));
      
      int weekIndex = 0;
      for (JsonVariant weekVar : weeksArray) {
        if (weekIndex >= 52) break;
        
        if (weekVar.is<JsonArray>()) {
          JsonArray daysArray = weekVar.as<JsonArray>();
          int dayIndex = 0;
          
          for (JsonVariant dayVar : daysArray) {
            if (dayIndex >= 7) break;
            contributions[weekIndex][dayIndex] = dayVar.as<uint8_t>();
            dayIndex++;
          }
        }
        weekIndex++;
      }
      
      // Set the data
      setGitHubContributions(contributions, total, currentStreak, longestStreak, username);
      
      Serial.printf("📊 Updated GitHub data: %d contributions, %d day streak\n", total, currentStreak);
      
      // Optionally trigger GitHub stats emotion
      if (emotionManager.getCurrentEmotion() != EMOTION_GITHUB_STATS) {
        // Could auto-switch to stats view
        // emotionManager.setTargetEmotion(EMOTION_GITHUB_STATS);
      }
    } else {
      Serial.println("⚠️  Invalid GitHub contribution format");
    }
  }
  else {
    Serial.printf("Unknown topic: %s\n", topic);
  }
}

// ===== NOTIFICATION QUEUE MANAGEMENT =====
bool NetworkManager::addNotification(NotificationType type, const char* title, const char* message) {
  if (notificationCount >= MAX_NOTIFICATION_QUEUE) {
    return false;  // Queue full
  }
  
  // Validate input parameters
  if (!title) title = "";
  if (!message) message = "";
  
  // Find next available slot
  for (int i = 0; i < MAX_NOTIFICATION_QUEUE; i++) {
    if (!notificationQueue[i].active) {
      notificationQueue[i].type = type;
      
      // Safe string copy with explicit null termination
      strncpy(notificationQueue[i].title, title, 31);
      notificationQueue[i].title[31] = '\0';
      
      strncpy(notificationQueue[i].message, message, 63);
      notificationQueue[i].message[63] = '\0';
      
      notificationQueue[i].timestamp = millis();
      notificationQueue[i].active = true;
      notificationCount++;
      return true;
    }
  }
  
  return false;
}

Notification* NetworkManager::getCurrentNotification() {
  if (notificationCount == 0) {
    return nullptr;
  }
  
  // Find first active notification sequentially
  for (int i = 0; i < MAX_NOTIFICATION_QUEUE; i++) {
    if (notificationQueue[i].active) {
      currentNotificationIndex = i;
      return &notificationQueue[i];
    }
  }
  
  // Inconsistency detected: count says we have notifications but none found
  Serial.println("WARNING: Notification count mismatch - resetting");
  notificationCount = 0;
  return nullptr;
}

void NetworkManager::clearCurrentNotification() {
  if (currentNotificationIndex >= MAX_NOTIFICATION_QUEUE) {
    Serial.println("ERROR: Invalid notification index");
    return;
  }
  
  if (currentNotificationIndex < MAX_NOTIFICATION_QUEUE && 
      notificationQueue[currentNotificationIndex].active) {
    notificationQueue[currentNotificationIndex].active = false;
    
    // Prevent underflow
    if (notificationCount > 0) {
      notificationCount--;
    } else {
      Serial.println("WARNING: Notification count already at 0");
    }
    
    Serial.printf("✓ Notification cleared. Queue: %d remaining\n", notificationCount);
    
    // If no more notifications, return to previous emotion
    if (notificationCount == 0 && emotionManager.getCurrentEmotion() == EMOTION_NOTIFICATION) {
      emotionManager.setTargetEmotion(EMOTION_IDLE);
    }
  }
}

// ===== STATUS CHECKERS =====
bool NetworkManager::isWiFiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isMQTTConnected() const {
#if ENABLE_MQTT
  return const_cast<PubSubClient&>(mqttClient).connected();
#else
  return false;
#endif
}

void NetworkManager::mqttLoop() {
#if ENABLE_MQTT
  mqttClient.loop();
#endif
}

// ===== NETWORK DIAGNOSTICS =====
bool NetworkManager::pingEndpoint(const char* hostname) {
  Serial.print("\n=== Pinging ");
  Serial.print(hostname);
  Serial.println(" ===");
  
  if (!isWiFiConnected()) {
    Serial.println("ERROR: WiFi not connected!");
    return false;
  }
  
  // Resolve hostname to IP
  IPAddress serverIP;
  if (!WiFi.hostByName(hostname, serverIP)) {
    Serial.println("ERROR: DNS lookup failed!");
    return false;
  }
  
  Serial.print("Resolved to IP: ");
  Serial.println(serverIP);
  
  // Test TCP connection on port 8883 (AWS IoT MQTT)
  WiFiClient testClient;
  Serial.print("Attempting TCP connection on port 8883... ");
  
  if (testClient.connect(serverIP, 8883, 5000)) {  // 5 second timeout
    Serial.println("SUCCESS!");
    Serial.print("Connected to: ");
    Serial.println(serverIP);
    testClient.stop();
    return true;
  } else {
    Serial.println("FAILED!");
    Serial.println("Could not establish TCP connection");
    return false;
  }
}

void NetworkManager::testConnectivity() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   NETWORK CONNECTIVITY TEST        ║");
  Serial.println("╚════════════════════════════════════╝");
  
  // WiFi Status
  Serial.print("\n[1] WiFi Status: ");
  if (isWiFiConnected()) {
    Serial.println("CONNECTED ✓");
    Serial.print("    SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("    IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("    Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("DISCONNECTED ✗");
    return;
  }
  
#if ENABLE_MQTT
  // DNS Resolution Test
  Serial.print("\n[2] DNS Resolution Test: ");
  IPAddress testIP;
  if (WiFi.hostByName(AWS_IOT_ENDPOINT, testIP)) {
    Serial.println("SUCCESS ✓");
    Serial.print("    ");
    Serial.print(AWS_IOT_ENDPOINT);
    Serial.print(" → ");
    Serial.println(testIP);
  } else {
    Serial.println("FAILED ✗");
  }
  
  // Ping AWS IoT Endpoint
  Serial.print("\n[3] AWS IoT Endpoint Connectivity: ");
  bool pingSuccess = pingEndpoint(AWS_IOT_ENDPOINT);
  if (pingSuccess) {
    Serial.println("    ✓ Port 8883 is reachable");
  } else {
    Serial.println("    ✗ Cannot reach endpoint");
  }
  
  // MQTT Status
  Serial.print("\n[4] MQTT Status: ");
  if (isMQTTConnected()) {
    Serial.println("CONNECTED ✓");
    Serial.print("    Thing Name: ");
    Serial.println(THINGNAME);
  } else {
    Serial.println("DISCONNECTED ✗");
    Serial.print("    Last error code: ");
    Serial.println(mqttClient.state());
  }
#else
  Serial.println("\n[2] MQTT: DISABLED in config.h");
#endif
  
  Serial.println("\n════════════════════════════════════\n");
}

// ===== GITHUB CONTRIBUTION DATA MANAGEMENT =====
void NetworkManager::setGitHubContributions(const uint8_t contributions[52][7], int total, int streak, int longest, const char* user) {
  // Copy contribution grid
  memcpy(githubData.contributions, contributions, sizeof(githubData.contributions));
  
  githubData.totalContributions = total;
  githubData.currentStreak = streak;
  githubData.longestStreak = longest;
  
  // Safe string copy
  if (user) {
    strncpy(githubData.username, user, 31);
    githubData.username[31] = '\0';
  } else {
    strcpy(githubData.username, "user");
  }
  
  githubData.dataLoaded = true;
  
  Serial.printf("GitHub data loaded: %d contributions, %d day streak\n", total, streak);
}

GitHubContributionData* NetworkManager::getGitHubData() {
  if (!githubData.dataLoaded) {
    return nullptr;
  }
  return &githubData;
}

bool NetworkManager::hasGitHubData() const {
  return githubData.dataLoaded;
}

void NetworkManager::clearGitHubData() {
  githubData.dataLoaded = false;
  githubData.totalContributions = 0;
  githubData.currentStreak = 0;
  githubData.longestStreak = 0;
  memset(githubData.username, 0, sizeof(githubData.username));
  memset(githubData.contributions, 0, sizeof(githubData.contributions));
  Serial.println("GitHub data cleared");
}

