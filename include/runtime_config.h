#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <stdint.h>
#include "config.h"

// Runtime-editable settings, persisted to NVS.
// Defaults mirror the compile-time constants in config.h.
struct RuntimeConfig {
  // Personality
  unsigned long attentionStage0Ms;    // ms untouched → NEEDY
  unsigned long attentionStage1Ms;    // ms untouched → BORED
  unsigned long attentionStage2Ms;    // ms untouched → SAD
  unsigned long attentionStage3Ms;    // ms untouched → GRUMPY
  unsigned long attentionStage4Ms;    // ms untouched → ANGRY
  unsigned long moodDriftIntervalMs;  // base interval between mood drift checks
  uint8_t microExpressionChance;      // % chance of BLINK per drift check
  uint8_t jitterPercent;              // ±% variance applied to all timers
  // Input
  unsigned long longPressMs;          // hold duration to trigger LONG_PRESS
  unsigned long doubleTapWindowMs;    // window after first tap to detect DOUBLE_TAP
  // Speaker
  bool enableEmotionBeep;             // play beep on emotion change
  uint8_t speakerVolume;              // PWM duty cycle volume (0–255)
  // WiFi / NTP (empty staSsid = AP-only mode, no NTP)
  char staSsid[33];
  char staPassword[65];
};

extern RuntimeConfig runtimeConfig;

void runtimeConfigLoad();   // Load from NVS; falls back to config.h defaults on first boot
void runtimeConfigSave();   // Persist current values to NVS
void runtimeConfigReset();  // Reset to config.h defaults and save

#endif // RUNTIME_CONFIG_H
