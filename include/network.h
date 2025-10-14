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
  
private:
  WiFiClientSecure wifiClient;
  PubSubClient mqttClient;
  NetworkState currentState;
  unsigned long lastReconnectAttempt;
  unsigned long lastStatusPublish;
  
  void setupTime();
  void handleIncomingMessage(const char* topic, const char* payload);
};

extern NetworkManager networkManager;

#endif // NETWORK_H
