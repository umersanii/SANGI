#include "runtime_config.h"

// Initialized with compile-time defaults so personality works even before runtimeConfigLoad() is called.
RuntimeConfig runtimeConfig = {
  ATTENTION_STAGE1_MS,
  ATTENTION_STAGE2_MS,
  ATTENTION_STAGE3_MS,
  ATTENTION_STAGE4_MS,
  MOOD_DRIFT_INTERVAL_MS,
  MICRO_EXPRESSION_CHANCE,
  JITTER_PERCENT
};

#ifdef NATIVE_BUILD

void runtimeConfigLoad()  {}  // Already initialized with defaults above
void runtimeConfigSave()  {}
void runtimeConfigReset() {}

#else

#include <Preferences.h>

static Preferences prefs;
static const char* NVS_NS = "sangi_cfg";

void runtimeConfigLoad() {
  prefs.begin(NVS_NS, true);
  runtimeConfig.attentionStage1Ms   = prefs.getULong("attn1",  ATTENTION_STAGE1_MS);
  runtimeConfig.attentionStage2Ms   = prefs.getULong("attn2",  ATTENTION_STAGE2_MS);
  runtimeConfig.attentionStage3Ms   = prefs.getULong("attn3",  ATTENTION_STAGE3_MS);
  runtimeConfig.attentionStage4Ms   = prefs.getULong("attn4",  ATTENTION_STAGE4_MS);
  runtimeConfig.moodDriftIntervalMs = prefs.getULong("drift",  MOOD_DRIFT_INTERVAL_MS);
  runtimeConfig.microExpressionChance = prefs.getUChar("micro", MICRO_EXPRESSION_CHANCE);
  runtimeConfig.jitterPercent       = prefs.getUChar("jitter", JITTER_PERCENT);
  prefs.end();
}

void runtimeConfigSave() {
  prefs.begin(NVS_NS, false);
  prefs.putULong("attn1",  runtimeConfig.attentionStage1Ms);
  prefs.putULong("attn2",  runtimeConfig.attentionStage2Ms);
  prefs.putULong("attn3",  runtimeConfig.attentionStage3Ms);
  prefs.putULong("attn4",  runtimeConfig.attentionStage4Ms);
  prefs.putULong("drift",  runtimeConfig.moodDriftIntervalMs);
  prefs.putUChar("micro",  runtimeConfig.microExpressionChance);
  prefs.putUChar("jitter", runtimeConfig.jitterPercent);
  prefs.end();
}

void runtimeConfigReset() {
  runtimeConfig = {
    ATTENTION_STAGE1_MS,
    ATTENTION_STAGE2_MS,
    ATTENTION_STAGE3_MS,
    ATTENTION_STAGE4_MS,
    MOOD_DRIFT_INTERVAL_MS,
    MICRO_EXPRESSION_CHANCE,
    JITTER_PERCENT
  };
  runtimeConfigSave();
}

#endif // NATIVE_BUILD
