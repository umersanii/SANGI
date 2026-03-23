#include "input.h"

InputManager inputManager;

InputManager::InputManager()
  : lastInteraction(0),
    onGesture_(nullptr),
    touchStartTime_(0),
    lastTapTime_(0),
    pendingTapTime_(0),
    touchActive_(false),
    longPressFired_(false),
    pendingTap_(false) {
}

void InputManager::init() {
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  Serial.printf("Touch sensor configured on GPIO%d\n", TOUCH_PIN);
}

bool InputManager::isTouched() {
  return digitalRead(TOUCH_PIN) == LOW;
}

// Returns gesture classification based on press duration and time since last tap.
// Exposed for unit testing.
TouchGesture classifyGesture(unsigned long pressDuration, unsigned long sincePrevTap) {
  if (pressDuration >= LONG_PRESS_MS) return GESTURE_LONG_PRESS;
  if (sincePrevTap <= DOUBLE_TAP_WINDOW_MS) return GESTURE_DOUBLE_TAP;
  return GESTURE_TAP;
}

void InputManager::handleTouchInteraction() {
  bool currentlyTouched = isTouched();
  unsigned long now = millis();

  // Detect press start (HIGH→LOW, i.e. not touched → touched)
  if (currentlyTouched && !touchActive_) {
    touchActive_ = true;
    touchStartTime_ = now;
    longPressFired_ = false;

    // If a tap was pending and a new touch arrives within the window: double tap
    if (pendingTap_ && now - pendingTapTime_ <= DOUBLE_TAP_WINDOW_MS) {
      pendingTap_ = false;
      lastTapTime_ = 0;
      if (onGesture_) onGesture_(GESTURE_DOUBLE_TAP, now);
      lastInteraction = now;
    }
  }

  // While held: fire long press on threshold crossing (before release)
  if (touchActive_ && !longPressFired_) {
    if (now - touchStartTime_ >= LONG_PRESS_MS) {
      longPressFired_ = true;
      pendingTap_ = false;  // cancel any pending tap
      if (onGesture_) onGesture_(GESTURE_LONG_PRESS, now);
      lastInteraction = now;
    }
  }

  // Detect release (LOW→HIGH, i.e. touched → not touched)
  if (!currentlyTouched && touchActive_) {
    touchActive_ = false;
    unsigned long duration = now - touchStartTime_;

    if (!longPressFired_ && duration < LONG_PRESS_MS) {
      // Short press: set pending tap, wait for possible second tap
      pendingTap_ = true;
      pendingTapTime_ = now;
      lastTapTime_ = now;
    }
    lastInteraction = now;
  }

  // Fire pending tap if double-tap window has expired
  if (pendingTap_ && !touchActive_ && now - pendingTapTime_ > DOUBLE_TAP_WINDOW_MS) {
    pendingTap_ = false;
    if (onGesture_) onGesture_(GESTURE_TAP, now);
  }
}
