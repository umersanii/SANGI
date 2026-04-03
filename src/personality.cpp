#include "personality.h"
#include "config.h"
#include "runtime_config.h"
#include "emotion_registry.h"

Personality personality;

Personality::Personality()
  : lastTouchTime_(0),
    lastDriftTime_(0),
    nextDriftInterval_(runtimeConfig.moodDriftIntervalMs),
    nextStageThreshold_(runtimeConfig.attentionStage0Ms),
    attentionStage_(0),
    timeProvider_(nullptr),
    warmthWindowStart_(0),
    touchCountRecent_(0),
    warmthActive_(false),
    warmthDriftCyclesLeft_(0),
    glowCycles_(0),
    lastDriftEmotion_(EMOTION_IDLE),
    consecutiveSameDrifts_(0),
    touchesToForgive_(0),
    nightCycleActive_(false) {
}

// Seeds all timers with the given start time and applies initial jitter to intervals.
void Personality::init(unsigned long currentTime) {
  lastTouchTime_       = currentTime;
  lastDriftTime_       = currentTime;
  nextDriftInterval_   = jitter(runtimeConfig.moodDriftIntervalMs);
  nextStageThreshold_  = jitter(runtimeConfig.attentionStage0Ms);
  attentionStage_      = 0;
  warmthWindowStart_   = currentTime;
  touchCountRecent_    = 0;
  warmthActive_        = false;
  warmthDriftCyclesLeft_ = 0;
  glowCycles_          = 0;
  lastDriftEmotion_    = EMOTION_IDLE;
  consecutiveSameDrifts_ = 0;
  touchesToForgive_    = 0;
  nightCycleActive_    = false;
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

// Returns the base attention threshold in milliseconds for the given neglect stage (1–5).
unsigned long Personality::stageBaseThreshold(int stage) {
  switch (stage) {
    case 1: return runtimeConfig.attentionStage0Ms;   // → NEEDY
    case 2: return runtimeConfig.attentionStage1Ms;   // → BORED
    case 3: return runtimeConfig.attentionStage2Ms;   // → SAD
    case 4: return runtimeConfig.attentionStage3Ms;   // → GRUMPY
    case 5: return runtimeConfig.attentionStage4Ms;   // → ANGRY
    default: return runtimeConfig.attentionStage0Ms;
  }
}

// Returns wall-clock hour (0–23). Uses injected time provider if set; falls back to millis().
int Personality::getTimeOfDayHour(unsigned long currentTime) const {
  if (timeProvider_) return timeProvider_();
  return (int)((currentTime / HOUR_IN_MILLIS) % 24);
}

// Classify an emotion into its mood cluster.
MoodCluster Personality::clusterOf(EmotionState e) {
  switch (e) {
    case EMOTION_HAPPY:
    case EMOTION_EXCITED:
    case EMOTION_LOVE:
    case EMOTION_PLAYFUL:
    case EMOTION_CONTENT:
      return CLUSTER_POSITIVE;
    case EMOTION_SAD:
    case EMOTION_BORED:
    case EMOTION_NEEDY:
    case EMOTION_SLEEPY:
    case EMOTION_GRUMPY:
    case EMOTION_ANGRY:
    case EMOTION_DEAD:
      return CLUSTER_NEGATIVE;
    default:
      return CLUSTER_NEUTRAL;
  }
}

// Pick a random emotion from a specific cluster.
EmotionState Personality::clusterDrift(MoodCluster cluster) {
  switch (cluster) {
    case CLUSTER_POSITIVE: {
      static const EmotionState pool[] = {EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_LOVE, EMOTION_PLAYFUL, EMOTION_CONTENT};
      return pool[random(0, 5)];
    }
    case CLUSTER_NEGATIVE: {
      static const EmotionState pool[] = {EMOTION_SAD, EMOTION_BORED, EMOTION_NEEDY, EMOTION_SLEEPY};
      return pool[random(0, 4)];
    }
    default: {
      static const EmotionState pool[] = {EMOTION_IDLE, EMOTION_THINKING, EMOTION_CONFUSED, EMOTION_SURPRISED};
      return pool[random(0, 4)];
    }
  }
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
    // Morning: HAPPY 30%, EXCITED 20%, PLAYFUL 15%, IDLE 20%, THINKING 10%, CONTENT 5%
    if (r < 30) return EMOTION_HAPPY;
    if (r < 50) return EMOTION_EXCITED;
    if (r < 65) return EMOTION_PLAYFUL;
    if (r < 85) return EMOTION_IDLE;
    if (r < 95) return EMOTION_THINKING;
    return EMOTION_CONTENT;
  } else if (hour < 18) {
    // Afternoon: THINKING 25%, IDLE 25%, HAPPY 15%, CONTENT 10%, CONFUSED 10%, SURPRISED 10%, PLAYFUL 5%
    if (r < 25) return EMOTION_THINKING;
    if (r < 50) return EMOTION_IDLE;
    if (r < 65) return EMOTION_HAPPY;
    if (r < 75) return EMOTION_CONTENT;
    if (r < 85) return EMOTION_CONFUSED;
    if (r < 95) return EMOTION_SURPRISED;
    return EMOTION_PLAYFUL;
  } else {
    // Evening: IDLE 25%, SLEEPY 30%, SAD 10%, LOVE 15%, CONTENT 10%, THINKING 10%
    if (r < 25) return EMOTION_IDLE;
    if (r < 55) return EMOTION_SLEEPY;
    if (r < 65) return EMOTION_SAD;
    if (r < 80) return EMOTION_LOVE;
    if (r < 90) return EMOTION_CONTENT;
    return EMOTION_THINKING;
  }
}

// Positive drift pool for post-interaction glow (any touch).
EmotionState Personality::moodDriftGlow() {
  static const EmotionState pool[] = {EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_LOVE, EMOTION_SURPRISED, EMOTION_CONTENT, EMOTION_PLAYFUL};
  return pool[random(0, 6)];
}

// Positive drift pool for sustained warmth arc (frequent interaction).
EmotionState Personality::moodDriftWarmed() {
  static const EmotionState pool[] = {EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_LOVE, EMOTION_CONTENT};
  return pool[random(0, 4)];
}

// Mood gravity: 65% chance to stay in current cluster, 35% normal time-of-day drift.
EmotionState Personality::moodDriftGravity(unsigned long currentTime, EmotionState currentEmotion) {
  int r = (int)random(0, 100);
  if (r < 65) {
    MoodCluster cluster = clusterOf(currentEmotion);
    return clusterDrift(cluster);
  }
  return moodDrift(currentTime);
}

// Picks a random driftable emotion from the variety pool, excluding one emotion.
// Used by habituation to force variety after consecutive same-emotion drifts.
EmotionState Personality::randomEmotionExcluding(EmotionState excluded) {
  static const EmotionState pool[] = {
    EMOTION_HAPPY, EMOTION_EXCITED, EMOTION_THINKING, EMOTION_IDLE,
    EMOTION_SURPRISED, EMOTION_LOVE, EMOTION_SLEEPY, EMOTION_CONFUSED,
    EMOTION_CONTENT, EMOTION_PLAYFUL
  };
  static const int poolSize = 10;
  int idx;
  do { idx = (int)random(0, poolSize); } while (pool[idx] == excluded);
  return pool[idx];
}

// 5-stage attention arc: NEEDY → BORED → SAD → GRUMPY → ANGRY
Personality::Decision Personality::attentionArc(unsigned long currentTime,
                                                   EmotionState current) {
  if (attentionStage_ >= 5) return {current, false};

  unsigned long timeSinceTouch = currentTime - lastTouchTime_;
  if (timeSinceTouch < nextStageThreshold_) return {current, false};

  attentionStage_++;

  if (attentionStage_ < 5) {
    nextStageThreshold_ = jitter(stageBaseThreshold(attentionStage_ + 1));
  }

  EmotionState target;
  switch (attentionStage_) {
    case 1: target = EMOTION_NEEDY;   break;
    case 2: target = EMOTION_BORED;   break;
    case 3: target = EMOTION_SAD;     break;
    case 4: target = EMOTION_GRUMPY;  break;
    case 5: target = EMOTION_ANGRY;   break;
    default: return {current, false};
  }

  if (current == target) return {current, false};

  Serial.printf("[Personality] Attention stage %d → %s\n",
                attentionStage_,
                emotionRegistry.getName(target));
  return {target, true};
}

// Night cycle (2-4 AM): restless light sleep with random wake moments.
// Only activates when NTP time provider is set and returns hours 2 or 3.
Personality::Decision Personality::nightCycle(unsigned long currentTime,
                                                EmotionState current) {
  if (!timeProvider_) return {current, false};
  int hour = timeProvider_();
  bool inWindow = (hour == 2 || hour == 3);

  if (inWindow && !nightCycleActive_) {
    nightCycleActive_ = true;
    Serial.println("[Personality] Night cycle active (2-4 AM)");
  } else if (!inWindow && nightCycleActive_) {
    nightCycleActive_ = false;
    Serial.println("[Personality] Night cycle ended");
  }

  if (!nightCycleActive_) return {current, false};

  // During night cycle, override drift interval to ~45s for restless feel
  if (currentTime - lastDriftTime_ < jitter(45000)) return {current, false};
  lastDriftTime_ = currentTime;

  int r = (int)random(0, 100);
  EmotionState target;
  if (r < 70) {
    target = EMOTION_SLEEPY;
  } else if (r < 90) {
    target = EMOTION_BLINK;
  } else {
    target = EMOTION_IDLE;   // brief "wake" moment
  }

  if (target == current) return {current, false};

  Serial.printf("[Personality] Night cycle → %s\n", emotionRegistry.getName(target));
  return {target, true};
}

// Returns true MICRO_EXPRESSION_CHANCE% of the time.
bool Personality::shouldMicroExpress() {
  return (int)random(0, 100) < runtimeConfig.microExpressionChance;
}

// Evaluates all personality subsystems and returns the next emotion Decision.
Personality::Decision Personality::update(unsigned long currentTime,
                                           EmotionState currentEmotion) {
  // 0. Night cycle (highest priority when active, overrides normal drift)
  if (nightCycleActive_ || (timeProvider_ && (timeProvider_() == 2 || timeProvider_() == 3))) {
    Decision nc = nightCycle(currentTime, currentEmotion);
    if (nc.shouldChange) return nc;
    if (nightCycleActive_) return {currentEmotion, false};  // suppress normal drift during night
  }

  // 1. Attention arc escalation
  Decision arc = attentionArc(currentTime, currentEmotion);
  if (arc.shouldChange) return arc;

  // 2. Mood drift (glow → warmth → gravity, checked on interval)
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
      // Mood gravity: bias toward current cluster
      drifted = moodDriftGravity(currentTime, currentEmotion);
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
// Multi-touch forgiveness: GRUMPY/ANGRY (stages 4-5) require FORGIVENESS_TOUCHES to fully recover.
// Returns true if the device was previously neglected (stage > 0).
bool Personality::onTouch(unsigned long currentTime, EmotionState currentEmotion) {
  bool wasNeglected = (attentionStage_ > 0);

  // Multi-touch forgiveness for deep neglect
  if (attentionStage_ >= 4 && touchesToForgive_ == 0) {
    // First touch on a grumpy/angry SANGI — start forgiveness counter
    touchesToForgive_ = FORGIVENESS_TOUCHES;
    lastTouchTime_ = currentTime;
    Serial.printf("[Personality] Deep neglect — %d more touches to forgive\n", touchesToForgive_);
    return wasNeglected;
  }

  if (touchesToForgive_ > 0) {
    touchesToForgive_--;
    lastTouchTime_ = currentTime;
    if (touchesToForgive_ > 0) {
      Serial.printf("[Personality] Forgiving... %d touches left\n", touchesToForgive_);
      return wasNeglected;
    }
    // Forgiveness complete — fall through to full reset
    Serial.println("[Personality] Forgiven — entering recovery");
  }

  // Reset attention arc
  attentionStage_      = 0;
  lastTouchTime_       = currentTime;
  nextStageThreshold_  = jitter(runtimeConfig.attentionStage0Ms);

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
