#include "personality.h"
#include "config.h"
#include "emotion_registry.h"

Personality personality;

Personality::Personality()
  : lastTouchTime_(0),
    lastDriftTime_(0),
    nextDriftInterval_(MOOD_DRIFT_INTERVAL_MS),
    nextStageThreshold_(ATTENTION_STAGE1_MS),
    recoveryEndTime_(0),
    attentionStage_(0),
    recoveryActive_(false) {
}

void Personality::init(unsigned long currentTime) {
  lastTouchTime_ = currentTime;
  lastDriftTime_ = currentTime;
  nextDriftInterval_ = jitter(MOOD_DRIFT_INTERVAL_MS);
  nextStageThreshold_ = jitter(ATTENTION_STAGE1_MS);
  attentionStage_ = 0;
  recoveryActive_ = false;
  recoveryEndTime_ = 0;
}

// Returns base ± JITTER_PERCENT%, clamped to [base/2, base*2].
unsigned long Personality::jitter(unsigned long base) {
  unsigned long jitterAmt = base * JITTER_PERCENT / 100;
  if (jitterAmt == 0) return base;
  long delta = (long)random(0, (long)(jitterAmt * 2 + 1)) - (long)jitterAmt;
  long result = (long)base + delta;
  if (result < (long)(base / 2)) result = (long)(base / 2);
  return (unsigned long)result;
}

unsigned long Personality::stageBaseThreshold(int stage) {
  switch (stage) {
    case 1: return ATTENTION_STAGE1_MS;
    case 2: return ATTENTION_STAGE2_MS;
    case 3: return ATTENTION_STAGE3_MS;
    case 4: return ATTENTION_STAGE4_MS;
    default: return ATTENTION_STAGE1_MS;
  }
}

// Hour-of-uptime weighted random emotion.
EmotionState Personality::moodDrift(unsigned long currentTime) {
  unsigned long hour = (currentTime / HOUR_IN_MILLIS) % 24;
  int r = (int)random(0, 100);

  if (hour < 6) {
    // Night: SLEEPY 60%, IDLE 30%, THINKING 10%
    if (r < 60) return EMOTION_SLEEPY;
    if (r < 90) return EMOTION_IDLE;
    return EMOTION_THINKING;
  } else if (hour < 12) {
    // Morning: HAPPY 40%, EXCITED 30%, IDLE 20%, THINKING 10%
    if (r < 40) return EMOTION_HAPPY;
    if (r < 70) return EMOTION_EXCITED;
    if (r < 90) return EMOTION_IDLE;
    return EMOTION_THINKING;
  } else if (hour < 18) {
    // Afternoon: THINKING 30%, IDLE 30%, HAPPY 20%, CONFUSED 10%, SURPRISED 10%
    if (r < 30) return EMOTION_THINKING;
    if (r < 60) return EMOTION_IDLE;
    if (r < 80) return EMOTION_HAPPY;
    if (r < 90) return EMOTION_CONFUSED;
    return EMOTION_SURPRISED;
  } else {
    // Evening: IDLE 30%, SLEEPY 30%, SAD 15%, LOVE 15%, THINKING 10%
    if (r < 30) return EMOTION_IDLE;
    if (r < 60) return EMOTION_SLEEPY;
    if (r < 75) return EMOTION_SAD;
    if (r < 90) return EMOTION_LOVE;
    return EMOTION_THINKING;
  }
}

Personality::Decision Personality::attentionArc(unsigned long currentTime,
                                                   EmotionState current) {
  if (attentionStage_ >= 4) return {current, false};

  unsigned long timeSinceTouch = currentTime - lastTouchTime_;
  if (timeSinceTouch < nextStageThreshold_) return {current, false};

  // Advance stage
  attentionStage_++;

  // Set jittered threshold for next stage (if any)
  if (attentionStage_ < 4) {
    nextStageThreshold_ = jitter(stageBaseThreshold(attentionStage_ + 1));
  }

  EmotionState target;
  switch (attentionStage_) {
    case 1: target = EMOTION_BORED;    break;
    case 2: target = EMOTION_SAD;      break;
    case 3: target = EMOTION_CONFUSED; break;
    case 4: target = EMOTION_ANGRY;    break;
    default: return {current, false};
  }

  if (current == target) return {current, false};  // already there

  Serial.printf("[Personality] Attention stage %d → %s\n",
                attentionStage_,
                emotionRegistry.getName(target));
  return {target, true};
}

bool Personality::shouldMicroExpress() {
  return (int)random(0, 100) < MICRO_EXPRESSION_CHANCE;
}

Personality::Decision Personality::update(unsigned long currentTime,
                                           EmotionState currentEmotion) {
  // 1. If recovery is active and SHY timer has elapsed → HAPPY
  if (recoveryActive_ && currentTime >= recoveryEndTime_) {
    recoveryActive_ = false;
    Serial.println("[Personality] Recovery: SHY → HAPPY");
    return {EMOTION_HAPPY, true};
  }
  if (recoveryActive_) return {currentEmotion, false};

  // 2. Attention arc escalation
  Decision arc = attentionArc(currentTime, currentEmotion);
  if (arc.shouldChange) return arc;

  // 3. Mood drift
  if (currentTime - lastDriftTime_ >= nextDriftInterval_) {
    lastDriftTime_ = currentTime;
    nextDriftInterval_ = jitter(MOOD_DRIFT_INTERVAL_MS);

    // 4. Micro-expression: occasional BLINK then return
    if (shouldMicroExpress()) {
      return {EMOTION_BLINK, true};
    }

    EmotionState drifted = moodDrift(currentTime);
    if (drifted != currentEmotion) {
      Serial.printf("[Personality] Mood drift → %s\n", emotionRegistry.getName(drifted));
      return {drifted, true};
    }
  }

  return {currentEmotion, false};
}

bool Personality::onTouch(unsigned long currentTime, EmotionState currentEmotion) {
  bool wasNeglected = (attentionStage_ > 0);

  // Reset attention arc
  attentionStage_ = 0;
  lastTouchTime_ = currentTime;
  nextStageThreshold_ = jitter(ATTENTION_STAGE1_MS);

  if (wasNeglected) {
    // Start recovery arc: SHY plays, then HAPPY.
    // Timer accounts for blink transition (~1390ms) + SHY animation (~1800ms) + buffer.
    // Recovery must fire after currentEmotion becomes SHY, otherwise setTargetEmotion(HAPPY)
    // is a no-op when previousEmotion was already HAPPY (guard: currentEmotion != newEmotion).
    recoveryActive_ = true;
    recoveryEndTime_ = currentTime + 3300;
    Serial.println("[Personality] Touch during neglect → SHY recovery");
  }

  return wasNeglected;
}
