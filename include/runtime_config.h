#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#include <stdint.h>
#include "config.h"

// Runtime-editable personality settings, persisted to NVS.
// Defaults mirror the compile-time constants in config.h.
struct RuntimeConfig {
  unsigned long attentionStage1Ms;    // ms untouched → BORED
  unsigned long attentionStage2Ms;    // ms untouched → SAD
  unsigned long attentionStage3Ms;    // ms untouched → CONFUSED
  unsigned long attentionStage4Ms;    // ms untouched → ANGRY
  unsigned long moodDriftIntervalMs;  // base interval between mood drift checks
  uint8_t microExpressionChance;      // % chance of BLINK per drift check
  uint8_t jitterPercent;              // ±% variance applied to all timers
};

extern RuntimeConfig runtimeConfig;

void runtimeConfigLoad();   // Load from NVS; falls back to config.h defaults on first boot
void runtimeConfigSave();   // Persist current values to NVS
void runtimeConfigReset();  // Reset to config.h defaults and save

#endif // RUNTIME_CONFIG_H
