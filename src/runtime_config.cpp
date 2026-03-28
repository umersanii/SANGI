#include "runtime_config.h"

// Initialized with compile-time defaults so personality works even before runtimeConfigLoad() is called.
RuntimeConfig runtimeConfig = {
  ATTENTION_STAGE1_MS,
  ATTENTION_STAGE2_MS,
  ATTENTION_STAGE3_MS,
  ATTENTION_STAGE4_MS,
  MOOD_DRIFT_INTERVAL_MS,
  MICRO_EXPRESSION_CHANCE,
  JITTER_PERCENT,
  LONG_PRESS_MS,
  DOUBLE_TAP_WINDOW_MS,
  ENABLE_EMOTION_BEEP,
  SPEAKER_VOLUME,
  WIFI_STA_SSID,      // staSsid    — compile-time default; NVS overrides after first save
  WIFI_STA_PASSWORD   // staPassword
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
  runtimeConfig.attentionStage1Ms     = prefs.getULong("attn1",     ATTENTION_STAGE1_MS);
  runtimeConfig.attentionStage2Ms     = prefs.getULong("attn2",     ATTENTION_STAGE2_MS);
  runtimeConfig.attentionStage3Ms     = prefs.getULong("attn3",     ATTENTION_STAGE3_MS);
  runtimeConfig.attentionStage4Ms     = prefs.getULong("attn4",     ATTENTION_STAGE4_MS);
  runtimeConfig.moodDriftIntervalMs   = prefs.getULong("drift",     MOOD_DRIFT_INTERVAL_MS);
  runtimeConfig.microExpressionChance = prefs.getUChar("micro",     MICRO_EXPRESSION_CHANCE);
  runtimeConfig.jitterPercent         = prefs.getUChar("jitter",    JITTER_PERCENT);
  runtimeConfig.longPressMs           = prefs.getULong("longpress", LONG_PRESS_MS);
  runtimeConfig.doubleTapWindowMs     = prefs.getULong("doubletap", DOUBLE_TAP_WINDOW_MS);
  runtimeConfig.enableEmotionBeep     = prefs.getBool ("beep",      ENABLE_EMOTION_BEEP);
  runtimeConfig.speakerVolume         = prefs.getUChar("volume",    SPEAKER_VOLUME);
  String ssid = prefs.getString("stassid", WIFI_STA_SSID);
  strlcpy(runtimeConfig.staSsid,     ssid.c_str(), sizeof(runtimeConfig.staSsid));
  String pass = prefs.getString("stapass", WIFI_STA_PASSWORD);
  strlcpy(runtimeConfig.staPassword, pass.c_str(), sizeof(runtimeConfig.staPassword));
  prefs.end();
}

void runtimeConfigSave() {
  prefs.begin(NVS_NS, false);
  prefs.putULong("attn1",     runtimeConfig.attentionStage1Ms);
  prefs.putULong("attn2",     runtimeConfig.attentionStage2Ms);
  prefs.putULong("attn3",     runtimeConfig.attentionStage3Ms);
  prefs.putULong("attn4",     runtimeConfig.attentionStage4Ms);
  prefs.putULong("drift",     runtimeConfig.moodDriftIntervalMs);
  prefs.putUChar("micro",     runtimeConfig.microExpressionChance);
  prefs.putUChar("jitter",    runtimeConfig.jitterPercent);
  prefs.putULong("longpress", runtimeConfig.longPressMs);
  prefs.putULong("doubletap", runtimeConfig.doubleTapWindowMs);
  prefs.putBool  ("beep",     runtimeConfig.enableEmotionBeep);
  prefs.putUChar ("volume",   runtimeConfig.speakerVolume);
  prefs.putString("stassid",  runtimeConfig.staSsid);
  prefs.putString("stapass",  runtimeConfig.staPassword);
  prefs.end();
}

void runtimeConfigReset() {
  runtimeConfig.attentionStage1Ms    = ATTENTION_STAGE1_MS;
  runtimeConfig.attentionStage2Ms    = ATTENTION_STAGE2_MS;
  runtimeConfig.attentionStage3Ms    = ATTENTION_STAGE3_MS;
  runtimeConfig.attentionStage4Ms    = ATTENTION_STAGE4_MS;
  runtimeConfig.moodDriftIntervalMs  = MOOD_DRIFT_INTERVAL_MS;
  runtimeConfig.microExpressionChance = MICRO_EXPRESSION_CHANCE;
  runtimeConfig.jitterPercent        = JITTER_PERCENT;
  runtimeConfig.longPressMs          = LONG_PRESS_MS;
  runtimeConfig.doubleTapWindowMs    = DOUBLE_TAP_WINDOW_MS;
  runtimeConfig.enableEmotionBeep    = ENABLE_EMOTION_BEEP;
  runtimeConfig.speakerVolume        = SPEAKER_VOLUME;
  strlcpy(runtimeConfig.staSsid,     WIFI_STA_SSID,     sizeof(runtimeConfig.staSsid));
  strlcpy(runtimeConfig.staPassword, WIFI_STA_PASSWORD, sizeof(runtimeConfig.staPassword));
  runtimeConfigSave();
}

#endif // NATIVE_BUILD
