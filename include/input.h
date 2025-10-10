#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "config.h"
#include "emotion.h"

// ===== INPUT MANAGER =====
class InputManager {
public:
  InputManager();
  
  void init();
  bool isTouched();
  void handleTouchInteraction();
  
  unsigned long getLastInteraction() const { return lastInteraction; }
  void updateLastInteraction(unsigned long time) { lastInteraction = time; }
  
private:
  unsigned long lastInteraction;
};

extern InputManager inputManager;

#endif // INPUT_H
