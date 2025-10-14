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
    lastStatusPublish(0) {
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
    
    // Publish initial status
    publishStatus("connected");
    
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
    Serial.println("WiFi disconnected - attempting reconnect");
    if (connectWiFi()) {
      connectMQTT();
    }
    return;
  }
  
  // Check MQTT status and reconnect if needed
  if (!isMQTTConnected()) {
    if (currentTime - lastReconnectAttempt > MQTT_RECONNECT_INTERVAL) {
      Serial.println("MQTT disconnected - attempting reconnect");
      lastReconnectAttempt = currentTime;
      connectMQTT();
    }
    return;
  }
  
  // Process MQTT messages
  mqttClient.loop();
  
  // Periodic status publishing
  if (currentTime - lastStatusPublish > STATUS_PUBLISH_INTERVAL) {
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

// ===== MESSAGE HANDLER =====
void NetworkManager::handleIncomingMessage(const char* topic, const char* payload) {
  // Parse JSON payload
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }
  
  // Handle emotion set command
  if (strcmp(topic, MQTT_TOPIC_EMOTION_SET) == 0) {
    if (doc.containsKey("emotion")) {
      int emotionValue = doc["emotion"];
      
      // Validate emotion range
      if (emotionValue >= EMOTION_IDLE && emotionValue <= EMOTION_MUSIC) {
        Serial.print("Setting emotion via MQTT: ");
        Serial.println(emotionValue);
        
        emotionManager.setTargetEmotion((EmotionState)emotionValue);
        publishStatus("emotion_updated");
      } else {
        Serial.print("Invalid emotion value: ");
        Serial.println(emotionValue);
        publishStatus("invalid_emotion");
      }
    }
  }
}

// ===== STATUS CHECKERS =====
bool NetworkManager::isWiFiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::isMQTTConnected() const {
 // return mqttClient.connected();
}

void NetworkManager::mqttLoop() {
#if ENABLE_MQTT
  mqttClient.loop();
#endif
}
