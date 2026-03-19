// EmotionManager implementation.
// CHANGED in Phase 1: Removed direct #includes of animations.h, speaker.h,
// network.h. All cross-module effects now go through callbacks, making
// EmotionManager fully self-contained and testable in isolation.

#include "emotion.h"

EmotionManager emotionManager;

EmotionManager::EmotionManager()
    : currentEmotion(EMOTION_IDLE),
      previousEmotion(EMOTION_IDLE),
      targetEmotion(EMOTION_IDLE),
      lastEmotionChange(0),
      bootTime(0),
      isTransitioning(false),
      transitionFrame(0),
      onTransitionComplete(nullptr),
      onEmotionChange(nullptr) {}

void EmotionManager::init(unsigned long currentTime) {
  bootTime = currentTime;
  lastEmotionChange = currentTime;
  currentEmotion = EMOTION_IDLE;
  targetEmotion = EMOTION_IDLE;
}

void EmotionManager::setTargetEmotion(EmotionState newEmotion) {
  if (newEmotion < EMOTION_IDLE || newEmotion > EMOTION_GITHUB_STATS) {
    Serial.printf("ERROR: Invalid emotion state %d\n", newEmotion);
    return;
  }

  if (currentEmotion != newEmotion) {
    previousEmotion = currentEmotion;
    targetEmotion = newEmotion;
    isTransitioning = true;
    transitionFrame = 0;

    Serial.printf("Emotion transition: %d -> %d\n", currentEmotion,
                  newEmotion);

    // Notify main.cpp (which forwards to beep, MQTT log, etc.)
    if (onEmotionChange) {
      onEmotionChange(currentEmotion, newEmotion);
    }
  }
}

void EmotionManager::update(unsigned long currentTime) {
  // Reserved for future autonomous emotion logic
}

EmotionState
EmotionManager::getTimeBasedEmotion(unsigned long currentTime) const {
  unsigned long currentUptime = currentTime - bootTime;
  unsigned long hourOfDay = (currentUptime / HOUR_IN_MILLIS) % 24;

  if (hourOfDay >= 6 && hourOfDay < 12) return EMOTION_HAPPY;
  if (hourOfDay >= 12 && hourOfDay < 18) return EMOTION_THINKING;
  if (hourOfDay >= 18 && hourOfDay < 22) return EMOTION_IDLE;
  return EMOTION_SLEEPY;
}

void EmotionManager::startTransition() {
  isTransitioning = true;
  transitionFrame = 0;
}

void EmotionManager::advanceTransition() {
  if (isTransitioning) {
    transitionFrame++;
  }
}

void EmotionManager::completeTransition() {
  currentEmotion = targetEmotion;
  isTransitioning = false;
  transitionFrame = 0;

  // Notify main.cpp (which calls animationManager.resetAnimation)
  if (onTransitionComplete) {
    onTransitionComplete(currentEmotion);
  }
}
