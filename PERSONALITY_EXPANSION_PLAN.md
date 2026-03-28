# SANGI Personality Expansion — Implementation Plan

## Executive Summary

Five expansions replace discrete timer-based emotion switching with a system that feels internally driven, environmentally aware, and shaped by history. Each step leaves the firmware in a flashable, fully-passing-tests state.

---

## Priority 1: Continuous Mood Architecture

### Motivation

The current system hard-snaps between named emotions via discrete timer thresholds. A continuous float `moodValue` makes emotions emerge organically from internal state drift and creates a single numeric surface that all other subsystems can write to.

### Architecture Decisions

**`moodValue_`** is a `float` in `[-1.0, 1.0]` living in `Personality`. Negative = negative affect; zero = neutral; positive = positive affect. Clamped after every mutation.

**Passive decay** pulls `moodValue_` toward `0.0f` at `MOOD_DECAY_RATE = 0.00002f` per ms. At the 50ms loop rate: `0.001f` per tick → full ±1.0 swing settles to neutral in ~50 seconds.

**Attention arc** becomes `applyAttentionPull(dtMs)` — modifies `moodValue_` directly instead of returning an emotion:

```
moodValue_ += (target - moodValue_) * pullStrength * dtMs
```

| Stage | Emotion | moodValue target | Pull strength |
|-------|---------|-----------------|---------------|
| 1 | BORED | -0.35 | 0.00005/ms |
| 2 | SAD | -0.60 | 0.00007/ms |
| 3 | CONFUSED | -0.55 | 0.00006/ms |
| 4 | ANGRY | -0.85 | 0.00010/ms |

**Mood drift** becomes `moodDriftDelta()` returning a `float` nudge:

| Category | Nudge |
|----------|-------|
| SLEEPY | -0.15 |
| IDLE | `moodValue_ *= 0.8` (push toward neutral) |
| HAPPY/EXCITED | +0.20 |
| LOVE | +0.25 |
| SAD | -0.20 |
| THINKING/CONFUSED | random(−0.10, +0.10) |
| SURPRISED | +0.30 |

**Threshold mapping** — sorted lookup, first match wins:

```
moodValue_ >= 0.75  →  EXCITED
moodValue_ >= 0.50  →  LOVE
moodValue_ >= 0.25  →  HAPPY
moodValue_ >= -0.10 →  IDLE
moodValue_ >= -0.30 →  THINKING
moodValue_ >= -0.45 →  BORED
moodValue_ >= -0.65 →  SAD
moodValue_ >= -0.75 →  CONFUSED
moodValue_ <  -0.75 →  ANGRY
```

SLEEPY is time-gated (late-night, P4). SURPRISED is event-triggered (P5). DEAD is battery-driven.

**Hysteresis.** `MOOD_HYSTERESIS = 0.05f` dead-band. Before emitting a new emotion, check `abs(moodValue_ - lastMappedThreshold_) > MOOD_HYSTERESIS`. Prevents rapid oscillation at threshold boundaries.

**dtMs cap.** Clamp to 200ms before applying pulls — prevents overshooting if BLE or display stalls the loop.

**New private fields in `Personality`:**
```cpp
float moodValue_;
float lastMappedThreshold_;
unsigned long lastUpdateMs_;
```

**New `config.h` constants:**
```cpp
#define MOOD_DECAY_RATE   0.00002f
#define MOOD_HYSTERESIS   0.05f
```

### Integration Points

- `personality.h` — add fields, `getMoodValue()`, `seedMood(float)`, `applyMoodNudge(float)`
- `personality.cpp` — rewrite `update()`: decay → arc pull → drift nudge → threshold mapping → hysteresis
- `config.h` — add two constants
- `main.cpp` — no changes (signature of `personality.update()` unchanged)
- `emotion.h` / `emotion.cpp` — no changes

### Risks

Pull constants tuned at 50ms loop. dtMs cap at 200ms handles scheduling jitter. If tempo feels wrong on hardware, `MOOD_DECAY_RATE` is the primary control knob.

### Definition of Done

- All existing tests pass
- New: `test_mood_decays_toward_neutral`, `test_attention_arc_pulls_mood_negative`, `test_mood_drift_nudges_mood`, `test_mood_threshold_mapping_returns_correct_emotion`, `test_hysteresis_prevents_rapid_flip`
- Device: Serial shows `moodValue` drifting smoothly over 10+ minutes without interaction

---

## Priority 2: Recovery Arc Humanisation

### Motivation

`onTouch()` snaps to HAPPY regardless of neglect depth. Staged recovery — requiring intermediate states and timing — is the core of making SANGI feel emotionally consistent.

### Architecture Decisions

**New private fields in `Personality`:**
```cpp
int recoveryStage_;               // 0 = not recovering, 1..3 = intermediate
EmotionState recoveryOrigin_;     // negative emotion we're recovering from
unsigned long recoveryStageStartMs_;
bool angryRequiresSecondTouch_;
```

**Recovery paths by neglect depth:**

| Origin | Stage 1 | Stage 2 | Stage 3 | Complete |
|--------|---------|---------|---------|----------|
| BORED | IDLE (3s) | — | — | HAPPY |
| SAD | BORED (4s) | IDLE (3s) | — | HAPPY |
| CONFUSED | SURPRISED (3s) | IDLE (3s) | — | HAPPY |
| ANGRY | BORED (5s) | THINKING (4s) | IDLE (3s) | HAPPY on 2nd touch only |

All timers jittered with standard `jitter()`. Additional touches during recovery skip stages. ANGRY requires a second touch after reaching IDLE before HAPPY is reachable — `angryRequiresSecondTouch_` gate.

**moodValue interaction.** During recovery, passive decay is replaced: `moodValue_` pulled toward the current stage's threshold at `MOOD_DECAY_RATE × 3.0f`.

**Changed `onTouch()` signature:**
```cpp
struct TouchResult {
    bool wasNeglected;
    EmotionState immediateEmotion;  // first recovery stage to show now
};
TouchResult onTouch(unsigned long currentTime, EmotionState currentEmotion);
```

**New public methods:**
```cpp
Decision advanceRecovery(unsigned long currentTime);
bool isRecovering() const { return recoveryStage_ > 0; }
```

**New `config.h` constants:**
```cpp
#define RECOVERY_BORED_STAGE1_MS    3000
#define RECOVERY_SAD_STAGE1_MS      4000
#define RECOVERY_SAD_STAGE2_MS      3000
#define RECOVERY_CONFUSED_STAGE1_MS 3000
#define RECOVERY_CONFUSED_STAGE2_MS 3000
#define RECOVERY_ANGRY_STAGE1_MS    5000
#define RECOVERY_ANGRY_STAGE2_MS    4000
#define RECOVERY_ANGRY_STAGE3_MS    3000
#define RECOVERY_FINAL_MS           2000
```

### Integration Points

- `personality.h` — add fields, `TouchResult`, `isRecovering()`, `advanceRecovery()`, new `onTouch()` signature
- `personality.cpp` — recovery path table and advance logic; moodValue pull override
- `main.cpp` `onGesture()`:
  ```cpp
  if (personality.isRecovering()) {
      Decision d = personality.advanceRecovery(currentTime);
      if (d.shouldChange) emotionManager.setTargetEmotion(d.emotion);
      return;
  }
  TouchResult r = personality.onTouch(currentTime, currentEmotion);
  if (r.wasNeglected) {
      emotionManager.setTargetEmotion(r.immediateEmotion);
      return;
  }
  // TAP→HAPPY / LONG_PRESS→LOVE / DOUBLE_TAP→EXCITED
  ```
- `personality.update()` — check `isRecovering()` first; delegate to `advanceRecovery()` if active

### Risks

Ignored during recovery → robot stays in intermediate states indefinitely. This is correct, intentional behaviour. Log stage in Serial.

### Definition of Done

- New: `test_bored_recovery_reaches_happy_in_two_stages`, `test_angry_recovery_requires_second_touch`, `test_recovery_advances_on_additional_touch`, `test_recovery_overrides_mood_drift`
- Existing `onTouch()` tests updated for new signature
- All tests pass
- Device: neglect 5+ min, touch → staged recovery in Serial; neglect 15+ min, one touch → no HAPPY, second touch → HAPPY

---

## Priority 3: NVS Memory — Two Values

### Motivation

Reboots wipe all personality state. Two persisted values give the boot sequence enough context to produce emotionally appropriate startup behaviour.

### Architecture Decisions

**NVS namespace:** `"sangi"`
**Keys:** `"health"` (float, 0.0–1.0, default 0.5) and `"aloneSec"` (uint32_t, seconds, default 0)

**New files:** `include/nvs_memory.h` + `src/nvs_memory.cpp` — hardware boundary, excluded from native `build_src_filter`.

```cpp
class NvsMemory {
public:
    void load();
    void saveHealth(float health);
    void saveAloneTime(uint32_t seconds);
    float getHealth() const;
    uint32_t getAloneSec() const;
    bool isFirstBoot() const;
};
extern NvsMemory nvsMemory;
```

**Health update rules:**
- `+0.05f` per positive touch (not neglected) — capped at 1.0
- `-0.03f` per neglect stage advance (stages 2, 3, 4 only) — capped at 0.0
- Written to NVS immediately via `onHealthChange` callback (keeps `personality.cpp` free of NVS dependency)

**AloneTime:** Reset to 0 on every successful touch. Read at boot as elapsed alone-time since last touch.

**Boot behaviour matrix:**

| health | aloneSec | Boot emotion | moodValue seed | Serial tag |
|--------|----------|--------------|----------------|------------|
| ≥ 0.7 | < 3600s | HAPPY → normal | +0.20 | "Warm greeting" |
| ≥ 0.7 | ≥ 3600s | SAD 4s → HAPPY | +0.10 | "Missed you" |
| < 0.4 | < 3600s | IDLE | -0.10 | "Reserved" |
| < 0.4 | ≥ 3600s | DEAD 5s → IDLE | -0.20 | "Cold" |
| 0.4–0.7 | any | IDLE | 0.0 | "Neutral" |

Boot emotion transitions handled via one-shot timer in `loop()` — not a blocking delay.

**moodValue seed:**
```cpp
personality.seedMood((nvsMemory.getHealth() - 0.5f) * 0.4f);
// health 0.0 → -0.20 | health 0.5 → 0.0 | health 1.0 → +0.20
```

**Native stub:** `test/arduino_stub/Preferences.h` — no-op class satisfying transitive includes (NVS never exercised in native tests).

**New `config.h` constants:**
```cpp
#define NVS_ALONE_THRESHOLD_S  3600
#define NVS_HEALTH_HIGH        0.7f
#define NVS_HEALTH_LOW         0.4f
#define NVS_HEALTH_TOUCH_INC   0.05f
#define NVS_HEALTH_NEGLECT_DEC 0.03f
```

### Integration Points

- New: `include/nvs_memory.h`, `src/nvs_memory.cpp`, `test/arduino_stub/Preferences.h`
- `personality.h` — add `health_` field, `getHealth()`, `seedMood()`, `onHealthChange` callback setter
- `personality.cpp` — increment/decrement `health_`, fire callback
- `main.cpp` — `nvsMemory.load()` at top of `setup()`, boot matrix, wire callback

### Risks

Frequent NVS writes acceptable at prototype scale (NVS wear-levelling). Note in comment.

### Definition of Done

- No new native tests (hardware boundary)
- Serial shows `[NVS] health=0.50 alone=0s` on first boot
- Health rises on touch, falls on neglect
- Forced low health + high alone → DEAD on boot

---

## Priority 4: Time-of-Day Arc Branching

### Motivation

Neglect always follows BORED→SAD→CONFUSED→ANGRY. At night, ANGRY is behaviorally wrong — SLEEPY is natural.

### Architecture Decisions

**`TimeContext` enum (in `personality.h`):**
```cpp
enum TimeContext {
    TIME_CONTEXT_LATE_NIGHT,  // 0–5h uptime
    TIME_CONTEXT_DAYTIME,     // 6–17h uptime
    TIME_CONTEXT_EVENING      // 18–23h uptime
};
```

**`getTimeContext()` promoted to `public`** (needed by `main.cpp` for mic silence gating in P5).

**Arc paths by context:**

| Context | Stage 1 | Stage 2 | Stage 3 | Max stage |
|---------|---------|---------|---------|-----------|
| LATE_NIGHT | SLEEPY (5min) | SAD (12.5min) | SLEEPY (deep) | 3, no ANGRY |
| DAYTIME | BORED (5min) | SAD (10min) | CONFUSED (12.5min) | 4 → ANGRY |
| EVENING | BORED (5min) | SAD (8min) | SLEEPY (11min) | 3, no ANGRY |

**Late-night drift doubling:** SLEEPY-mapped drift nudge doubled to `-0.30f` (vs `-0.15f` daytime).

**`stageBaseThreshold()` gains `TimeContext` parameter.**

**New `config.h` constants:**
```cpp
#define TIME_CONTEXT_LATE_NIGHT_END_H  6
#define TIME_CONTEXT_DAYTIME_END_H     18
#define ATTENTION_EVENING_STAGE2_MS    480000  // 8 min
#define ATTENTION_EVENING_STAGE3_MS    660000  // 11 min
```

### Integration Points

- `personality.h` — `TimeContext` enum, `getTimeContext()` public
- `personality.cpp` — `applyAttentionPull()` and `stageBaseThreshold()` take context; drift doubling in `moodDriftDelta()`
- `config.h` — new constants

### Risks

TimeContext is uptime-based, not real-time. Known limitation; no RTC workaround needed.

### Definition of Done

- New: `test_late_night_context_caps_at_stage3`, `test_late_night_stage1_is_sleepy`, `test_evening_stage2_uses_shorter_threshold`, `test_evening_stage3_is_sleepy`
- Device: at simulated hour 2, neglect caps at SLEEPY; ANGRY never appears

---

## Priority 5: Microphone Integration — SparkFun Electret Breakout

### Motivation

Touch is the only input. Ambient sound gives SANGI passive environmental awareness without speech recognition or FFT — amplitude envelope only.

### Architecture Decisions

**Pin:** `GPIO 4` (ADC1_CH4). GPIO 5 is a strapping pin — avoid it. GPIO 2 (battery, ADC1_CH2) and GPIO 4 (ADC1_CH4) are sequential reads, no conflict.

**Wiring:** SparkFun electret AUD → GPIO 4, VCC → 3.3V, GND → GND.

**New files:** `include/mic_manager.h` + `src/mic_manager.cpp` — excluded from native `build_src_filter`.

```cpp
class MicManager {
public:
    enum SoundEvent { SOUND_NONE, SOUND_SPIKE, SOUND_SUSTAINED_LOUD, SOUND_SILENCE };
    void init();                           // 50-sample baseline calibration (1s, blocking in setup())
    void tick(unsigned long currentTime);  // non-blocking, call every loop()
    SoundEvent pollEvent();                // returns and clears last pending event
private:
    int baseline_;
    int rollingAvg_;        // integer EMA: avg = (avg*9 + sample) / 10
    unsigned long loudStartMs_;
    unsigned long silenceStartMs_;
    unsigned long lastSampleMs_;
    SoundEvent pendingEvent_;
};
extern MicManager micManager;
```

**Sampling:** `tick()` reads once if `currentTime - lastSampleMs_ >= MIC_SAMPLE_INTERVAL_MS` (20ms). One `analogRead` ≈ 10µs — cannot starve the main loop.

**Event detection:**

| Event | Condition | Behaviour |
|-------|-----------|-----------|
| Spike | `newSample - rollingAvg_ > 500` | Set `SOUND_SPIKE` immediately |
| Sustained loud | `rollingAvg_ - baseline_ > 300` for ≥ 500ms | Set `SOUND_SUSTAINED_LOUD`; reset to prevent re-fire |
| Prolonged silence | `rollingAvg_ < baseline_ + 50` for ≥ 30s | Set `SOUND_SILENCE`; reset |

**moodValue mapping (in `main.cpp`):**
```cpp
MicManager::SoundEvent ev = micManager.pollEvent();
if (ev == MicManager::SOUND_SPIKE) {
    personality.applyMoodNudge(0.30f);
    if (emotionManager.getCurrentEmotion() != EMOTION_SURPRISED)
        emotionManager.setTargetEmotion(EMOTION_SURPRISED);  // immediate visual
} else if (ev == MicManager::SOUND_SUSTAINED_LOUD) {
    personality.applyMoodNudge(0.15f);
} else if (ev == MicManager::SOUND_SILENCE) {
    if (personality.getTimeContext(currentTime) == TIME_CONTEXT_LATE_NIGHT)
        personality.applyMoodNudge(-0.20f);
}
```

`MicManager` is never included in `personality.h`. Nudges pass through `applyMoodNudge(float)` which is the correct general hook for all future nudge sources.

**New `config.h` constants:**
```cpp
#define MIC_PIN                    4
#define MIC_SAMPLE_INTERVAL_MS     20     // 2-3 samples per 50ms loop tick
#define MIC_BASELINE_SAMPLES       50     // 1s calibration at boot
#define MIC_LOUD_THRESHOLD_DELTA   300    // ADC units above baseline (tune per environment)
#define MIC_SPIKE_THRESHOLD_DELTA  500    // ADC units above rolling avg
#define MIC_SUSTAINED_LOUD_MS      500
#define MIC_SILENCE_DURATION_MS    30000  // 30s
```

### Integration Points

- New: `include/mic_manager.h`, `src/mic_manager.cpp`
- `personality.h` — `applyMoodNudge(float)` (added in P1), `getTimeContext()` public (added in P4)
- `main.cpp` — `micManager.init()` in `setup()`, `micManager.tick()` + event handler in `loop()`
- `config.h` — all `MIC_*` constants
- `test/arduino_stub/Arduino.h` — add `stubSetAnalogRead()` no-op for future use

### Risks

`MIC_LOUD_THRESHOLD_DELTA = 300` calibrated for a quiet desk. Comment in `config.h` instructs tuning. Noisy boot skews baseline — reboot corrects it.

### Definition of Done

- New native test: `test_mood_nudge_from_spike` — calls `applyMoodNudge(0.30f)`, verifies `getMoodValue()` increases
- Device wired: Serial shows `[Mic] Baseline: NNN` at boot
- Clap → SURPRISED within one loop tick
- Sustained music → moodValue rises in Serial
- Late-night silence → SLEEPY drift visible

---

## Sequenced Implementation Order

Each step: all tests pass, `pio run` succeeds, device flashes and runs.

| Step | Covers | New Tests | Running Total | Observable on Device |
|------|--------|-----------|---------------|----------------------|
| 1 | P1 scaffolding — moodValue, decay, `seedMood`, `applyMoodNudge` | 2 | 66 | `moodValue: 0.00` in Serial |
| 2 | P1 arc + P4 base — `applyAttentionPull`, `TimeContext`, threshold mapping, hysteresis | 5 | 71 | Emotions shift by mood |
| 3 | P1 drift + P4 full — `moodDriftDelta`, late-night doubling, evening arc | 4 | 75 | Natural mood float |
| 4 | P2 — Recovery arc, `TouchResult`, `advanceRecovery`, `isRecovering` | 4 | 79 | Staged recovery |
| 5 | P3 — NVS files, health tracking, boot matrix | 0 (HW) | 79 | Boot matrix in Serial |
| 6 | P5 — MicManager, sound events, event handler in main | 1 | 80 | Sound reactions |

### Critical Files

| File | Changes |
|------|---------|
| `src/personality.cpp` | Core rewrite: moodValue, arc pulls, recovery, drift nudges, time context |
| `include/personality.h` | New fields, `TouchResult`, changed signatures, new public methods |
| `src/main.cpp` | Gesture handler, boot matrix, mic polling, NVS wiring |
| `include/config.h` | All new constants |
| `test/test_sangi.cpp` | Update `onTouch()` callers; add 16 new tests |
| `include/nvs_memory.h` + `src/nvs_memory.cpp` | New: NVS persistence |
| `include/mic_manager.h` + `src/mic_manager.cpp` | New: analog mic |
| `test/arduino_stub/Preferences.h` | New: no-op stub |
