#include "input.h"

InputManager inputManager;

InputManager::InputManager()
  : lastInteraction(0), onTouch_(nullptr) {
}

void InputManager::init() {
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  Serial.printf("Touch sensor configured on GPIO%d\n", TOUCH_PIN);
}

bool InputManager::isTouched() {
  return digitalRead(TOUCH_PIN) == LOW;
}

void InputManager::handleTouchInteraction() {
  if (isTouched()) {
    unsigned long currentTime = millis();
    lastInteraction = currentTime;

    if (onTouch_) {
      onTouch_(currentTime);
    }

    delay(50);  // Debounce
  }
}
