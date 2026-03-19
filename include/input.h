#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "config.h"

// Callback type for touch events
typedef void (*TouchHandlerFn)(unsigned long currentTime);

// ===== INPUT MANAGER =====
class InputManager {
public:
  InputManager();

  void init();
  bool isTouched();
  void handleTouchInteraction();

  unsigned long getLastInteraction() const { return lastInteraction; }
  void updateLastInteraction(unsigned long time) { lastInteraction = time; }

  void setOnTouch(TouchHandlerFn fn) { onTouch_ = fn; }

private:
  unsigned long lastInteraction;
  TouchHandlerFn onTouch_;
};

extern InputManager inputManager;

#endif // INPUT_H
