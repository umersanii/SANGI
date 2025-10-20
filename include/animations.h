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
  void animateMusic();
  void animateDead();
  void animateNotification(const char* title = "", const char* message = "");
  void animateCoding();
  
  // Reset animation frames when switching emotions
  void resetAnimation(EmotionState emotion);
  
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
  unsigned long lastMusicAnim;
  unsigned long lastDeadAnim;
  unsigned long lastNotificationAnim;
  unsigned long lastCodingAnim;
  
  int sleepyFrame;
  int thinkFrame;
  int exciteFrame;
  int confuseFrame;
  int happyFrame;
  int loveFrame;
  int angryFrame;
  int sadFrame;
  int surprisedFrame;
  int musicFrame;
  int deadFrame;
  int notificationFrame;
  int codingFrame;
};

extern AnimationManager animationManager;

#endif // ANIMATIONS_H
