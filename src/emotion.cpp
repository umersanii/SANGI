#include "emotion.h"

EmotionManager emotionManager;

EmotionManager::EmotionManager() 
  : currentEmotion(EMOTION_IDLE),
    previousEmotion(EMOTION_IDLE),
    targetEmotion(EMOTION_IDLE),
    lastEmotionChange(0),
    bootTime(0),
    isTransitioning(false),
    transitionFrame(0) {
}

void EmotionManager::init(unsigned long currentTime) {
  bootTime = currentTime;
  lastEmotionChange = currentTime;
  currentEmotion = EMOTION_IDLE;
  targetEmotion = EMOTION_IDLE;
}

void EmotionManager::setTargetEmotion(EmotionState newEmotion) {
  if (currentEmotion != newEmotion) {
    previousEmotion = currentEmotion;
    targetEmotion = newEmotion;
    isTransitioning = true;
    transitionFrame = 0;
  }
}

void EmotionManager::update(unsigned long currentTime) {
  // This would contain the emotion update logic
  // Currently disabled in the original code for animation testing
}

EmotionState EmotionManager::getTimeBasedEmotion(unsigned long currentTime) const {
  unsigned long currentUptime = currentTime - bootTime;
  unsigned long hourOfDay = (currentUptime / HOUR_IN_MILLIS) % 24;
  
  if (hourOfDay >= 6 && hourOfDay < 12) {
    return EMOTION_HAPPY;  // Morning
  } else if (hourOfDay >= 12 && hourOfDay < 18) {
    return EMOTION_THINKING;  // Afternoon
  } else if (hourOfDay >= 18 && hourOfDay < 22) {
    return EMOTION_IDLE;  // Evening
  } else {
    return EMOTION_SLEEPY;  // Night
  }
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
}
