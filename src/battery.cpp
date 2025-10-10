#include "battery.h"

BatteryManager batteryManager;

BatteryManager::BatteryManager() {
}

void BatteryManager::init() {
  pinMode(BATTERY_PIN, INPUT);
  analogReadResolution(12);
  Serial.printf("Battery monitoring on GPIO%d\n", BATTERY_PIN);
}

float BatteryManager::readVoltage() {
  int rawValue = analogRead(BATTERY_PIN);
  float voltage = (rawValue / 4095.0) * 3.3;
  return voltage;
}

EmotionState BatteryManager::getBatteryBasedEmotion() {
  // Disabled for USB power - uncomment when using LiPo battery
  /*
  float voltage = readVoltage();
  
  if (voltage < BATTERY_MIN_VOLTAGE) {
    return EMOTION_DEAD;
  } else if (voltage < BATTERY_LOW_THRESHOLD) {
    return EMOTION_SLEEPY;
  }
  */
  
  return EMOTION_IDLE;  // No override when on USB power
}
