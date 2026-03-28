#ifndef PERSONALITY_H
#define PERSONALITY_H

#include <Arduino.h>
#include "emotion.h"

// Optional real-time hour provider (0–23). If not set, millis()-based fallback is used.
typedef int (*TimeProviderFn)();

class Personality {
public:
  Personality();
  void init(unsigned long currentTime);

  // Call every loop(). Returns emotion to set, or shouldChange=false if no change.
  struct Decision {
    EmotionState emotion;
    bool shouldChange;
  };
  Decision update(unsigned long currentTime, EmotionState currentEmotion);

  // Call when touch occurs — resets attention timer. Returns true if was neglected.
  bool onTouch(unsigned long currentTime, EmotionState currentEmotion);

  // Inject a real-time hour provider (e.g. NTP-backed). Falls back to millis() if null.
  void setTimeProvider(TimeProviderFn fn) { timeProvider_ = fn; }

  // For testing: inspect internal state
  int getAttentionStage() const { return attentionStage_; }
  unsigned long getLastTouchTime() const { return lastTouchTime_; }
  bool isWarmed() const { return warmthActive_; }
  int getGlowCycles() const { return glowCycles_; }
  int getConsecutiveSameDrifts() const { return consecutiveSameDrifts_; }

  // Public for testing
  unsigned long jitter(unsigned long base);

private:
  unsigned long lastTouchTime_;
  unsigned long lastDriftTime_;
  unsigned long nextDriftInterval_;    // jittered
  unsigned long nextStageThreshold_;   // jittered threshold for next attention stage
  int attentionStage_;                 // 0=none, 1=BORED, 2=SAD, 3=CONFUSED, 4=ANGRY

  // NTP time provider
  TimeProviderFn timeProvider_;

  // Warmth arc — sustained positive bias after frequent interaction
  unsigned long warmthWindowStart_;
  int touchCountRecent_;
  bool warmthActive_;
  int warmthDriftCyclesLeft_;

  // Post-interaction glow — immediate positive burst after any touch
  int glowCycles_;

  // Habituation — tracks consecutive same-emotion drifts to force variety
  EmotionState lastDriftEmotion_;
  int consecutiveSameDrifts_;

  EmotionState moodDrift(unsigned long currentTime);
  EmotionState moodDriftGlow();
  EmotionState moodDriftWarmed();
  EmotionState randomEmotionExcluding(EmotionState excluded);
  int getTimeOfDayHour(unsigned long currentTime) const;
  Decision attentionArc(unsigned long currentTime, EmotionState current);
  bool shouldMicroExpress();

  unsigned long stageBaseThreshold(int stage);
};

extern Personality personality;

#endif
