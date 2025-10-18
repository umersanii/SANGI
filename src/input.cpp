#include "input.h"

InputManager inputManager;

InputManager::InputManager() 
  : lastInteraction(0) {
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
    
    // Random emotion selection with better distribution
    int randomChoice = random(0, 100);
    
    if (randomChoice < 50) {
      emotionManager.setTargetEmotion(EMOTION_EXCITED);
    } else {
      emotionManager.setTargetEmotion(EMOTION_SURPRISED);
    }
    
    delay(50);  // Debounce
  }
}
