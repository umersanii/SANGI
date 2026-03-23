#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include "config.h"

// Gesture types detected by InputManager
enum TouchGesture {
  GESTURE_NONE,
  GESTURE_TAP,
  GESTURE_LONG_PRESS,
  GESTURE_DOUBLE_TAP
};

// Callback for gesture events
typedef void (*GestureHandlerFn)(TouchGesture gesture, unsigned long currentTime);

// Testable gesture classification (exposed for unit testing)
TouchGesture classifyGesture(unsigned long pressDuration, unsigned long sincePrevTap);

// ===== INPUT MANAGER =====
class InputManager {
public:
  InputManager();

  void init();
  bool isTouched();
  void handleTouchInteraction();

  unsigned long getLastInteraction() const { return lastInteraction; }
  void updateLastInteraction(unsigned long time) { lastInteraction = time; }

  void setOnGesture(GestureHandlerFn fn) { onGesture_ = fn; }

private:
  unsigned long lastInteraction;
  GestureHandlerFn onGesture_;

  unsigned long touchStartTime_;
  unsigned long lastTapTime_;
  unsigned long pendingTapTime_;
  bool touchActive_;
  bool longPressFired_;
  bool pendingTap_;
};

extern InputManager inputManager;

#endif // INPUT_H
