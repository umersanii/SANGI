#ifndef EMOTION_H
#define EMOTION_H

// EmotionManager — state machine for emotion transitions.
// CHANGED in Phase 1: No longer #includes animations.h, speaker.h, or network.h.
// Cross-module side-effects (beep, animation reset, MQTT logging) are now
// handled via callbacks set by main.cpp, eliminating circular dependencies.

#include <Arduino.h>
#include "config.h"

// ===== EMOTION STATE ENUM =====
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
  EMOTION_BORED,
  EMOTION_SHY,
  EMOTION_NEEDY,
  EMOTION_BLINK
};

// Callback signatures for dependency injection
typedef void (*TransitionCompleteFn)(EmotionState newEmotion);
typedef void (*EmotionChangeFn)(EmotionState from, EmotionState to);

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

  // Dependency injection: main.cpp sets these so EmotionManager
  // doesn't need to know about AnimationManager, BeepManager, etc.
  void setOnTransitionComplete(TransitionCompleteFn fn) {
    onTransitionComplete = fn;
  }
  void setOnEmotionChange(EmotionChangeFn fn) { onEmotionChange = fn; }

private:
  EmotionState currentEmotion;
  EmotionState previousEmotion;
  EmotionState targetEmotion;
  unsigned long lastEmotionChange;
  unsigned long bootTime;
  bool isTransitioning;
  int transitionFrame;

  TransitionCompleteFn onTransitionComplete;
  EmotionChangeFn onEmotionChange;
};

extern EmotionManager emotionManager;

#endif // EMOTION_H
