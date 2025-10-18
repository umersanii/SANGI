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
  
  // ESP32-C3 ADC is 12-bit (0-4095) with 3.3V reference
  // Add bounds checking for safety
  if (rawValue < 0) rawValue = 0;
  if (rawValue > 4095) rawValue = 4095;
  
  float voltage = (rawValue / 4095.0) * 3.3;
  
  // Sanity check: voltage should be between 0V and 5V
  if (voltage < 0.0f) voltage = 0.0f;
  if (voltage > 5.0f) {
    Serial.printf("WARNING: Voltage reading %f V exceeds expected range\n", voltage);
    voltage = 3.3f;  // Default to safe value
  }
  
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
