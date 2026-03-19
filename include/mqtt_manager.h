#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

// Callback type for incoming MQTT messages
typedef void (*MqttMessageHandler)(const char* topic, const char* payload);

struct MqttSubscription {
  const char* topic;
  MqttMessageHandler handler;
};

// ===== NETWORK STATE =====
enum NetworkState {
  NET_DISCONNECTED,
  NET_WIFI_CONNECTING,
  NET_WIFI_CONNECTED,
  NET_MQTT_CONNECTING,
  NET_MQTT_CONNECTED,
  NET_ERROR
};

// ===== MQTT MANAGER =====
class MqttManager {
public:
  static const int MAX_SUBSCRIPTIONS = 10;

  MqttManager();

  bool init(const char* endpoint, int port,
            const char* ca, const char* cert, const char* key,
            const char* clientId);
  void update();

  // Subscribe to a topic with a handler — no hardcoded switch needed
  bool subscribe(const char* topic, MqttMessageHandler handler);

  bool publish(const char* topic, const char* payload);
  bool isConnected() const;

  // WiFi
  bool connectWiFi(const char* ssid, const char* password,
                   unsigned long timeoutMs);
  bool isWiFiConnected() const;
  const char* getConnectedSSID() const { return connectedSSID_; }
  int getRSSI() const;

  // State
  NetworkState getState() const { return state_; }

  // Diagnostics
  bool pingEndpoint(const char* hostname);
  void testConnectivity();

private:
  WiFiClientSecure wifiClient_;
  PubSubClient mqttClient_;
  NetworkState state_;
  unsigned long lastReconnect_;
  char connectedSSID_[33];
  const char* clientId_;

  MqttSubscription subs_[MAX_SUBSCRIPTIONS];
  int subCount_;

  static void staticCallback(char* topic, byte* payload, unsigned int len);
  void dispatchMessage(const char* topic, const char* payload);
  bool connectMqtt();
  void setupTime();
};

extern MqttManager mqttManager;

#endif // MQTT_MANAGER_H
