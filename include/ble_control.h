#ifndef BLE_CONTROL_H
#define BLE_CONTROL_H

#include <Arduino.h>
#include "emotion.h"

// Callback for BLE-initiated emotion changes
typedef void (*BleEmotionFn)(EmotionState emotion);

class BleControl {
public:
  void init(BleEmotionFn onEmotion);
  void updateCurrentEmotion(uint8_t emotionId);
private:
  BleEmotionFn onEmotion_;
};

extern BleControl bleControl;

#endif
