#include "personality.h"
#include "config.h"
#include "runtime_config.h"
#include "emotion_registry.h"

Personality personality;

Personality::Personality()
  : lastTouchTime_(0),
    lastDriftTime_(0),
    nextDriftInterval_(runtimeConfig.moodDriftIntervalMs),
    nextStageThreshold_(runtimeConfig.attentionStage1Ms),
    attentionStage_(0),
    timeProvider_(nullptr),
    warmthWindowStart_(0),
    touchCountRecent_(0),
    warmthActive_(false),
    warmthDriftCyclesLeft_(0),
    glowCycles_(0),
    lastDriftEmotion_(EMOTION_IDLE),
    consecutiveSameDrifts_(0) {
}

// Seeds all timers with the given start time and applies initial jitter to intervals.
void Personality::init(unsigned long currentTime) {
  lastTouchTime_       = currentTime;
  lastDriftTime_       = currentTime;
  nextDriftInterval_   = jitter(runtimeConfig.moodDriftIntervalMs);
  nextStageThreshold_  = jitter(runtimeConfig.attentionStage1Ms);
  attentionStage_      = 0;
  warmthWindowStart_   = currentTime;
  touchCountRecent_    = 0;
  warmthActive_        = false;
  warmthDriftCyclesLeft_ = 0;
  glowCycles_          = 0;
  lastDriftEmotion_    = EMOTION_IDLE;
  consecutiveSameDrifts_ = 0;
}

// Returns base ± JITTER_PERCENT%, clamped to [base/2, base*2].
unsigned long Personality::jitter(unsigned long base) {
  unsigned long jitterAmt = base * runtimeConfig.jitterPercent / 100;
  if (jitterAmt == 0) return base;
  long delta = (long)random(0, (long)(jitterAmt * 2 + 1)) - (long)jitterAmt;
  long result = (long)base + delta;
  if (result < (long)(base / 2)) result = (long)(base / 2);
  return (unsigned long)result;
}

// Returns the base attention threshold in milliseconds for the given neglect stage (1–4).
unsigned long Personality::stageBaseThreshold(int stage) {
  switch (stage) {
    case 1: return runtimeConfig.attentionStage1Ms;
    case 2: return runtimeConfig.attentionStage2Ms;
    case 3: return runtimeConfig.attentionStage3Ms;
    case 4: return runtimeConfig.attentionStage4Ms;
    default: return runtimeConfig.attentionStage1Ms;
  }
}

// Returns wall-clock hour (0–23). Uses injected time provider if set; falls back to millis().
int Personality::getTimeOfDayHour(unsigned long currentTime) const {
  if (timeProvider_) return timeProvider_();
  return (int)((currentTime / HOUR_IN_MILLIS) % 24);
}

// Hour-of-day weighted random emotion for normal (unbiased) drift.
EmotionState Personality::moodDrift(unsigned long currentTime) {
  int hour = getTimeOfDayHour(currentTime);
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

// Positive drift pool for post-interaction glow (any touch).
EmotionState Personality::moodDriftGlow() {
  static const EmotionState pool[] = {EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_LOVE, EMOTION_SURPRISED};
  return pool[random(0, 4)];
}

// Positive drift pool for sustained warmth arc (frequent interaction).
EmotionState Personality::moodDriftWarmed() {
  static const EmotionState pool[] = {EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_LOVE};
  return pool[random(0, 3)];
}

// Picks a random driftable emotion from the variety pool, excluding one emotion.
// Used by habituation to force variety after consecutive same-emotion drifts.
EmotionState Personality::randomEmotionExcluding(EmotionState excluded) {
  static const EmotionState pool[] = {
    EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_THINKING, EMOTION_IDLE,
    EMOTION_SURPRISED, EMOTION_LOVE, EMOTION_SLEEPY, EMOTION_CONFUSED
  };
  static const int poolSize = 8;
  int idx;
  do { idx = (int)random(0, poolSize); } while (pool[idx] == excluded);
  return pool[idx];
}

// Checks whether the neglect timer has crossed the next stage threshold and escalates the attention arc.
Personality::Decision Personality::attentionArc(unsigned long currentTime,
                                                   EmotionState current) {
  if (attentionStage_ >= 4) return {current, false};

  unsigned long timeSinceTouch = currentTime - lastTouchTime_;
  if (timeSinceTouch < nextStageThreshold_) return {current, false};

  attentionStage_++;

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

  if (current == target) return {current, false};

  Serial.printf("[Personality] Attention stage %d → %s\n",
                attentionStage_,
                emotionRegistry.getName(target));
  return {target, true};
}

// Returns true MICRO_EXPRESSION_CHANCE% of the time.
bool Personality::shouldMicroExpress() {
  return (int)random(0, 100) < runtimeConfig.microExpressionChance;
}

// Evaluates all personality subsystems and returns the next emotion Decision.
Personality::Decision Personality::update(unsigned long currentTime,
                                           EmotionState currentEmotion) {
  // 1. Attention arc escalation (highest priority)
  Decision arc = attentionArc(currentTime, currentEmotion);
  if (arc.shouldChange) return arc;

  // 2. Mood drift (glow → warmth → normal, checked on interval)
  if (currentTime - lastDriftTime_ >= nextDriftInterval_) {
    lastDriftTime_     = currentTime;
    nextDriftInterval_ = jitter(runtimeConfig.moodDriftIntervalMs);

    // Micro-expression: occasional BLINK regardless of warmth state
    if (shouldMicroExpress()) {
      return {EMOTION_BLINK, true};
    }

    // Select drift emotion based on current bias
    EmotionState drifted;
    if (glowCycles_ > 0) {
      glowCycles_--;
      drifted = moodDriftGlow();
    } else if (warmthActive_) {
      drifted = moodDriftWarmed();
      warmthDriftCyclesLeft_--;
      if (warmthDriftCyclesLeft_ <= 0) {
        warmthActive_ = false;
        Serial.println("[Personality] Warmth arc ended");
      }
    } else {
      drifted = moodDrift(currentTime);
    }

    // Habituation: track consecutive same-emotion drift selections
    if (drifted == lastDriftEmotion_) {
      consecutiveSameDrifts_++;
    } else {
      consecutiveSameDrifts_ = 0;
    }

    if (consecutiveSameDrifts_ >= HABITUATION_THRESHOLD) {
      drifted = randomEmotionExcluding(lastDriftEmotion_);
      consecutiveSameDrifts_ = 0;
      Serial.printf("[Personality] Habituation: forced variety → %s\n",
                    emotionRegistry.getName(drifted));
    }

    lastDriftEmotion_ = drifted;

    if (drifted != currentEmotion) {
      Serial.printf("[Personality] Mood drift → %s\n", emotionRegistry.getName(drifted));
      return {drifted, true};
    }
  }

  return {currentEmotion, false};
}

// Resets the attention arc on any touch.
// Also tracks reciprocal warmth and sets immediate post-touch glow.
// Returns true if the device was previously neglected (stage > 0).
bool Personality::onTouch(unsigned long currentTime, EmotionState currentEmotion) {
  bool wasNeglected = (attentionStage_ > 0);

  // Reset attention arc
  attentionStage_      = 0;
  lastTouchTime_       = currentTime;
  nextStageThreshold_  = jitter(runtimeConfig.attentionStage1Ms);

  // Post-interaction glow: next GLOW_DRIFT_CYCLES drifts are positive
  glowCycles_ = GLOW_DRIFT_CYCLES;

  // Warmth arc: count touches in rolling window
  if (currentTime - warmthWindowStart_ > WARMTH_WINDOW_MS) {
    warmthWindowStart_ = currentTime;
    touchCountRecent_  = 0;
  }
  touchCountRecent_++;
  if (!warmthActive_ && touchCountRecent_ >= WARMTH_TOUCH_THRESHOLD) {
    warmthActive_          = true;
    warmthDriftCyclesLeft_ = WARMTH_DRIFT_CYCLES;
    touchCountRecent_      = 0;
    warmthWindowStart_     = currentTime;
    Serial.println("[Personality] Warmth arc activated");
  }

  return wasNeglected;
}
