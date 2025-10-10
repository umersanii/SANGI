#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include "config.h"
#include "emotion.h"

// ===== ANIMATION MANAGER =====
class AnimationManager {
public:
  AnimationManager();
  
  void animateSleepy();
  void animateThinking();
  void animateExcited();
  void animateConfused();
  void animateHappy();
  void animateLove();
  void animateAngry();
  void animateSad();
  void animateSurprised();
  
private:
  // Animation state tracking
  unsigned long lastSleepyAnim;
  unsigned long lastThinkAnim;
  unsigned long lastExciteAnim;
  unsigned long lastConfuseAnim;
  unsigned long lastHappyAnim;
  unsigned long lastLoveAnim;
  unsigned long lastAngryAnim;
  unsigned long lastSadAnim;
  unsigned long lastSurprisedAnim;
  
  int sleepyFrame;
  int thinkFrame;
  int exciteFrame;
  int confuseFrame;
  int happyFrame;
  int loveFrame;
  int angryFrame;
  int sadFrame;
  int surprisedFrame;
};

extern AnimationManager animationManager;

#endif // ANIMATIONS_H
