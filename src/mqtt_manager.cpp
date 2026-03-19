#include "mqtt_manager.h"
#include <time.h>

#if ENABLE_MQTT
#include "secrets.h"
#endif

// ===== GLOBAL INSTANCE =====
MqttManager mqttManager;

// Static instance pointer for the callback
static MqttManager* mqttInstance = nullptr;

MqttManager::MqttManager()
  : mqttClient_(wifiClient_),
    state_(NET_DISCONNECTED),
    lastReconnect_(0),
    clientId_(nullptr),
    subCount_(0) {
  memset(connectedSSID_, 0, sizeof(connectedSSID_));
}

bool MqttManager::init(const char* endpoint, int port,
                        const char* ca, const char* cert, const char* key,
                        const char* clientId) {
#if !ENABLE_MQTT
  Serial.println("MQTT disabled in config.h");
  state_ = NET_DISCONNECTED;
  return false;
#else
  clientId_ = clientId;
  mqttInstance = this;

  Serial.println("\n=== MQTT Manager Initializing ===");

  // Configure WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  // Setup MQTT client
  mqttClient_.setServer(endpoint, port);
  mqttClient_.setCallback(MqttManager::staticCallback);
  mqttClient_.setBufferSize(1024);
  mqttClient_.setKeepAlive(60);
  mqttClient_.setSocketTimeout(30);

  // Configure certificates
  wifiClient_.setCACert(ca);
  wifiClient_.setCertificate(cert);
  wifiClient_.setPrivateKey(key);

  // Connect WiFi
  if (!connectWiFi(WIFI_SSID, WIFI_PASSWORD, NETWORK_TIMEOUT)) {
    Serial.println("WiFi connection failed");
    state_ = NET_ERROR;
    return false;
  }

  // Sync time (required for AWS IoT Core)
  setupTime();

  // Connect MQTT
  if (!connectMqtt()) {
    Serial.println("MQTT connection failed (will retry)");
    state_ = NET_MQTT_CONNECTING;
    return false;
  }

  Serial.println("=== MQTT Manager Ready ===\n");
  return true;
#endif
}

bool MqttManager::connectWiFi(const char* ssid, const char* password,
                               unsigned long timeoutMs) {
#if !ENABLE_MQTT
  return false;
#else
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  state_ = NET_WIFI_CONNECTING;
  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeoutMs) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection timeout");
    state_ = NET_ERROR;
    return false;
  }

  state_ = NET_WIFI_CONNECTED;
  Serial.print("WiFi connected! IP: ");
  Serial.println(WiFi.localIP());

  // Store connected SSID for validation
  String ssidStr = WiFi.SSID();
  strncpy(connectedSSID_, ssidStr.c_str(), 32);
  connectedSSID_[32] = '\0';
  Serial.print("Connected SSID: ");
  Serial.println(connectedSSID_);

  return true;
#endif
}

bool MqttManager::connectMqtt() {
#if !ENABLE_MQTT
  return false;
#else
  if (!isWiFiConnected()) {
    Serial.println("WiFi not connected - cannot connect MQTT");
    return false;
  }

  Serial.print("Connecting to AWS IoT Core as: ");
  Serial.println(clientId_);

  state_ = NET_MQTT_CONNECTING;

  if (mqttClient_.connect(clientId_)) {
    Serial.println("MQTT connected!");
    state_ = NET_MQTT_CONNECTED;

    // Subscribe to all registered topics
    for (int i = 0; i < subCount_; i++) {
      if (mqttClient_.subscribe(subs_[i].topic)) {
        Serial.print("Subscribed to: ");
        Serial.println(subs_[i].topic);
      } else {
        Serial.print("Failed to subscribe to: ");
        Serial.println(subs_[i].topic);
      }
    }

    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqttClient_.state());
    state_ = NET_ERROR;
    return false;
  }
#endif
}

void MqttManager::setupTime() {
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

void MqttManager::update() {
#if ENABLE_MQTT
  unsigned long currentTime = millis();

  // Check WiFi status
  if (!isWiFiConnected()) {
    static bool wifiDisconnectLogged = false;
    if (!wifiDisconnectLogged) {
      Serial.println("WiFi disconnected - attempting reconnect");
      wifiDisconnectLogged = true;
    }

    if (connectWiFi(WIFI_SSID, WIFI_PASSWORD, NETWORK_TIMEOUT)) {
      connectMqtt();
      wifiDisconnectLogged = false;
    }
    return;
  }

  // Check MQTT status and reconnect if needed
  if (!isConnected()) {
    bool overflow = currentTime < lastReconnect_;

    if (overflow || (currentTime - lastReconnect_ > MQTT_RECONNECT_INTERVAL)) {
      Serial.println("MQTT disconnected - attempting reconnect");
      lastReconnect_ = currentTime;
      connectMqtt();
    }
    return;
  }

  // Process MQTT messages - THIS IS CRITICAL!
  mqttClient_.loop();
#endif
}

bool MqttManager::subscribe(const char* topic, MqttMessageHandler handler) {
  if (subCount_ >= MAX_SUBSCRIPTIONS) {
    Serial.println("ERROR: Max MQTT subscriptions reached");
    return false;
  }

  subs_[subCount_].topic = topic;
  subs_[subCount_].handler = handler;
  subCount_++;

  // If already connected, subscribe immediately
  if (isConnected()) {
    if (mqttClient_.subscribe(topic)) {
      Serial.print("Subscribed to: ");
      Serial.println(topic);
    } else {
      Serial.print("Failed to subscribe to: ");
      Serial.println(topic);
      return false;
    }
  }

  return true;
}

bool MqttManager::publish(const char* topic, const char* payload) {
#if ENABLE_MQTT
  if (!isConnected()) return false;
  return mqttClient_.publish(topic, payload);
#else
  return false;
#endif
}

bool MqttManager::isConnected() const {
#if ENABLE_MQTT
  return const_cast<PubSubClient&>(mqttClient_).connected();
#else
  return false;
#endif
}

bool MqttManager::isWiFiConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

int MqttManager::getRSSI() const {
  return WiFi.RSSI();
}

// ===== STATIC MQTT CALLBACK =====
void MqttManager::staticCallback(char* topic, byte* payload, unsigned int len) {
  // Create null-terminated string from payload
  char message[len + 1];
  memcpy(message, payload, len);
  message[len] = '\0';

  Serial.print("MQTT message received on topic: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(message);

  if (mqttInstance) {
    mqttInstance->dispatchMessage(topic, message);
  }
}

void MqttManager::dispatchMessage(const char* topic, const char* payload) {
  for (int i = 0; i < subCount_; i++) {
    if (strcmp(subs_[i].topic, topic) == 0) {
      subs_[i].handler(topic, payload);
      return;
    }
  }
  Serial.printf("Unhandled topic: %s\n", topic);
}

// ===== NETWORK DIAGNOSTICS =====
bool MqttManager::pingEndpoint(const char* hostname) {
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

  if (testClient.connect(serverIP, 8883, 5000)) {
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

void MqttManager::testConnectivity() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   NETWORK CONNECTIVITY TEST        ║");
  Serial.println("╚════════════════════════════════════╝");

  // WiFi Status
  Serial.print("\n[1] WiFi Status: ");
  if (isWiFiConnected()) {
    Serial.println("CONNECTED");
    Serial.print("    SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("    IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("    Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("DISCONNECTED");
    return;
  }

#if ENABLE_MQTT
  // DNS Resolution Test
  Serial.print("\n[2] DNS Resolution Test: ");
  IPAddress testIP;
  if (WiFi.hostByName(AWS_IOT_ENDPOINT, testIP)) {
    Serial.println("SUCCESS");
    Serial.print("    ");
    Serial.print(AWS_IOT_ENDPOINT);
    Serial.print(" -> ");
    Serial.println(testIP);
  } else {
    Serial.println("FAILED");
  }

  // Ping AWS IoT Endpoint
  Serial.print("\n[3] AWS IoT Endpoint Connectivity: ");
  bool pingSuccess = pingEndpoint(AWS_IOT_ENDPOINT);
  if (pingSuccess) {
    Serial.println("    Port 8883 is reachable");
  } else {
    Serial.println("    Cannot reach endpoint");
  }

  // MQTT Status
  Serial.print("\n[4] MQTT Status: ");
  if (isConnected()) {
    Serial.println("CONNECTED");
    Serial.print("    Thing Name: ");
    Serial.println(clientId_);
  } else {
    Serial.println("DISCONNECTED");
    Serial.print("    Last error code: ");
    Serial.println(mqttClient_.state());
  }
#else
  Serial.println("\n[2] MQTT: DISABLED in config.h");
#endif

  Serial.println("\n════════════════════════════════════\n");
}
