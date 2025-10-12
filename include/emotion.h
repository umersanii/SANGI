#ifndef EMOTION_H
#define EMOTION_H

#include <Arduino.h>
#include "config.h"

// ===== EMOTION STATE MACHINE =====
enum EmotionState {
  EMOTION_IDLE,
  EMOTION_HAPPY,
  EMOTION_SLEEPY,
  EMOTION_EXCITED,
  EMOTION_SAD,
  EMOTION_ANGRY,
  EMOTION_CONFUSED,
  EMOTION_THINKING,
  EMOTION_LOVE,
  EMOTION_SURPRISED,
  EMOTION_DEAD,
  EMOTION_BATMAN,
  EMOTION_MUSIC,
  EMOTION_BLINK
};

// ===== EMOTION STATE MANAGEMENT =====
class EmotionManager {
public:
  EmotionManager();
  
  void init(unsigned long currentTime);
  void update(unsigned long currentTime);
  void setTargetEmotion(EmotionState newEmotion);
  
  EmotionState getCurrentEmotion() const { return currentEmotion; }
  EmotionState getPreviousEmotion() const { return previousEmotion; }
  EmotionState getTargetEmotion() const { return targetEmotion; }
  bool isTransitionActive() const { return isTransitioning; }
  int getTransitionFrame() const { return transitionFrame; }
  
  void startTransition();
  void advanceTransition();
  void completeTransition();
  
  EmotionState getTimeBasedEmotion(unsigned long currentTime) const;
  
private:
  EmotionState currentEmotion;
  EmotionState previousEmotion;
  EmotionState targetEmotion;
  unsigned long lastEmotionChange;
  unsigned long bootTime;
  bool isTransitioning;
  int transitionFrame;
};

extern EmotionManager emotionManager;

#endif // EMOTION_H
