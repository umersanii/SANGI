#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>
#include "config.h"
#include "emotion.h"

// ===== BATTERY MANAGER =====
class BatteryManager {
public:
  BatteryManager();
  
  void init();
  float readVoltage();
  EmotionState getBatteryBasedEmotion();
  
private:
  // No state needed currently
};

extern BatteryManager batteryManager;

#endif // BATTERY_H
