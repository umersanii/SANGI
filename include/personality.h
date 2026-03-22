#ifndef PERSONALITY_H
#define PERSONALITY_H

#include <Arduino.h>
#include "emotion.h"

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

  // Call when touch occurs — resets attention timer. Returns true if was neglected
  // (i.e., recovery arc applies, caller should set EMOTION_SHY).
  bool onTouch(unsigned long currentTime, EmotionState currentEmotion);

  // For testing: inspect internal state
  int getAttentionStage() const { return attentionStage_; }
  unsigned long getLastTouchTime() const { return lastTouchTime_; }

  // Public for testing
  unsigned long jitter(unsigned long base);

private:
  unsigned long lastTouchTime_;
  unsigned long lastDriftTime_;
  unsigned long nextDriftInterval_;    // jittered
  unsigned long nextStageThreshold_;   // jittered threshold for next attention stage
  unsigned long recoveryEndTime_;      // when SHY animation ends → transition to HAPPY
  int attentionStage_;                 // 0=none, 1=BORED, 2=SAD, 3=CONFUSED, 4=ANGRY
  bool recoveryActive_;

  EmotionState moodDrift(unsigned long currentTime);
  Decision attentionArc(unsigned long currentTime, EmotionState current);
  bool shouldMicroExpress();

  unsigned long stageBaseThreshold(int stage);
};

extern Personality personality;

#endif
