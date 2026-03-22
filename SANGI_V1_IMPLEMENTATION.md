# SANGI v1 — Implementation Plan

## Module 1: Strip Networking

### Scope
Remove all MQTT, WiFi, and cloud infrastructure from the firmware. Delete source files, headers, library dependencies, and all references in `main.cpp`, `config.h`, and `platformio.ini`. After this module, SANGI compiles and runs as a standalone device with autonomous emotion cycling, no network.

### Files

**Delete:**
- `src/network.cpp`
- `include/network.h`
- `src/mqtt_manager.cpp`
- `include/mqtt_manager.h`
- `src/notification_queue.cpp`
- `include/notification_queue.h`
- `src/github_data.cpp`
- `include/github_data.h`
- `include/secrets.h` (if present)

**Modify:**

**`platformio.ini`** — Remove from `[env:airm2m_core_esp32c3]`:
- `knolleary/PubSubClient@^2.8` from `lib_deps`
- `bblanchon/ArduinoJson@^6.21.3` from `lib_deps`

Remove from `[env:native]` `build_src_filter`:
- `+<notification_queue.cpp>`
- `+<github_data.cpp>`

**`include/config.h`** — Delete lines 56–94 entirely (everything from `// ===== NETWORK CONFIGURATION =====` through `OFFLINE_EMOTION_INTERVAL`). Keep `ENABLE_MQTT` but hardcode to `false`. Actually, simpler: delete `ENABLE_MQTT` and all `#if ENABLE_MQTT` blocks in other files, since MQTT is permanently gone. Add:
```cpp
#define EMOTION_CHANGE_INTERVAL_BASE 30000  // 30s base, personality will jitter this
```

**`src/main.cpp`** — Gut rewrite. Remove:
- `#include <ArduinoJson.h>`
- `#include "network.h"`
- All four `handle*` MQTT topic handler functions (lines 161–327)
- All `#if ENABLE_MQTT` / `#if !ENABLE_MQTT` conditional blocks — keep only the non-MQTT paths
- `selectAutonomousEmotion()` — will be replaced by personality engine in Module 5
- `offlineNotifTitle`, `offlineNotifMessage`, `generateOfflineNotification()` — gone with NOTIFICATION emotion
- `loadHardcodedGitHubData` forward decl and call
- MQTT subscribe calls and `networkManager.*` calls in `setup()` and `loop()`
- The `EMOTION_NOTIFICATION` and `EMOTION_GITHUB_STATS` context-dispatch blocks in the render section of `loop()`

Simplified `loop()` after this module:
```cpp
void loop() {
  unsigned long currentTime = millis();
  beepManager.update();
  emotionManager.update(currentTime);

  // Autonomous cycling (temporary — replaced by personality in Module 5)
  static unsigned long lastSwitch = 0;
  static int idx = 0;
  static EmotionState cyclable[EmotionRegistry::MAX_EMOTIONS];
  static int numCyclable = 0;
  static bool loaded = false;
  if (!loaded) {
    numCyclable = emotionRegistry.getCyclable(cyclable, EmotionRegistry::MAX_EMOTIONS);
    loaded = true;
  }
  if (currentTime - lastSwitch > EMOTION_CHANGE_INTERVAL_BASE) {
    emotionManager.setTargetEmotion(cyclable[random(0, numCyclable)]);
    lastSwitch = currentTime;
  }

  inputManager.handleTouchInteraction();

  if (emotionManager.isTransitionActive()) {
    TransitionResult r = displayManager.performTransitionFrame(
        emotionManager.getTransitionFrame(),
        emotionManager.getCurrentEmotion(),
        emotionManager.getTargetEmotion());
    if (r == TR_COMPLETE) emotionManager.completeTransition();
    else emotionManager.advanceTransition();
  } else {
    animationManager.tick(emotionManager.getCurrentEmotion(), displayManager);
  }

  // Debug output every 10s
  static unsigned long lastDebug = 0;
  if (currentTime - lastDebug > 10000) {
    float voltage = batteryManager.readVoltage();
    Serial.printf("Battery: %.2fV | Emotion: %s | Uptime: %lus\n",
                  voltage,
                  emotionRegistry.getName(emotionManager.getCurrentEmotion()),
                  (currentTime - bootTime) / 1000);
    lastDebug = currentTime;
  }
  delay(50);
}
```

**`src/emotion.cpp`** — In `setTargetEmotion()`, change range check from `EMOTION_GITHUB_STATS` to a dynamic check using `emotionRegistry.get(newEmotion) != nullptr`. This decouples the range from the enum ordering and future-proofs it for BORED/SHY additions.

**`include/emotion.h`** — Remove `#include "config.h"` if it was only needed for MQTT-related constants. Check: it's also needed for `HOUR_IN_MILLIS` used in `getTimeBasedEmotion()` — keep the include.

### Implementation notes
- `AnimState states_[]` in `animations.h` is sized as `EMOTION_GITHUB_STATS + 1`. After removing GITHUB_STATS from the enum, this needs to reference the new last enum value. Better: change to `MAX_EMOTIONS` from the registry (`EmotionRegistry::MAX_EMOTIONS = 20`), which is already defined and stable. Change line in `animations.h`:
  ```cpp
  AnimState states_[EmotionRegistry::MAX_EMOTIONS];
  ```
  And update the `for` loop and bounds check in `animations.cpp` similarly.
- `emotion.cpp` line 29: `if (newEmotion > EMOTION_GITHUB_STATS)` — change to registry lookup validation.

### Definition of done
- `pio run` succeeds with zero errors, zero warnings related to removed symbols.
- `pio test -e native` — all tests that don't reference `NotificationQueue`, `NotificationContext`, `GitHubStatsContext`, `drawNotification`, `drawGitHubStats`, `drawCoding`, or `drawMusic` pass. Tests referencing deleted types are removed in this module.
- Flash to device: display shows boot screen, then cycles through emotions every ~30s. Touch triggers EXCITED/SURPRISED. Speaker beeps on each emotion change. Serial output shows battery voltage and emotion name.

### Integration test
Remove from `test_sangi.cpp`:
- `#include "notification_queue.h"` and all 5 `test_notification_*` tests
- `test_draw_notification_uses_context` and `test_draw_github_stats_with_data`
- Remove `drawNotification`, `drawGitHubStats`, `drawCoding`, `drawMusic` from `test_all_emotions_draw_without_crash` array

Update `test_registry_count` to assert `12` (remaining emotions after removing NOTIFICATION, CODING, GITHUB_STATS, MUSIC).

Update `registerTestEmotions()` to exclude the four removed emotions.

New test:
```cpp
void test_emotion_validation_uses_registry() {
  emotionManager.setTargetEmotion((EmotionState)99);
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  // Valid registered emotion works
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  TEST_ASSERT_TRUE(emotionManager.isTransitionActive());
}
```

Run: `pio test -e native` — all remaining tests pass.

---

## Module 2: Remove Cut Emotions

### Scope
Remove NOTIFICATION, CODING, GITHUB_STATS, and MUSIC from the enum, registry, draw functions, speaker patterns, and all references. The enum gets 4 entries shorter. `emotion_draws.cpp` loses ~550 LOC.

### Files

**Modify:**

**`include/emotion.h`** — Remove `EMOTION_MUSIC`, `EMOTION_NOTIFICATION`, `EMOTION_CODING`, `EMOTION_GITHUB_STATS` from the enum.

**`src/emotion_draws.cpp`** — Delete:
- `drawMusicNote()` helper (lines 42–58)
- `drawMusic()` (lines 1110–1270, ~160 LOC)
- `drawNotification()` (lines 1353–1489, ~137 LOC)
- `drawCoding()` (lines 1490–1550, ~61 LOC)
- `drawGitHubStats()` (lines 1551–1755, ~204 LOC)
- `drawGlasses()` helper (lines 27–39, used only by CODING)

**`include/emotion_draws.h`** — Remove:
- `NotificationContext` struct
- `GitHubStatsContext` struct
- `drawMusic()`, `drawNotification()`, `drawCoding()`, `drawGitHubStats()` declarations

**`src/speaker.cpp`** — Remove `PATTERN_MUSIC`, `PATTERN_NOTIFICATION`, `PATTERN_CODING` arrays and their `PATTERN_ENTRY` lines from `EMOTION_PATTERNS[]`.

**`src/main.cpp`** — Remove the `EMOTION_MUSIC` registration from `registerEmotions()`. Remove all three special animation registrations (NOTIFICATION, CODING, GITHUB_STATS).

**`platformio.ini`** — `[env:native]` `build_src_filter`: already no `notification_queue.cpp` or `github_data.cpp` (removed in Module 1). No changes needed here.

### Implementation notes
- After removing these enum values, the remaining enum values shift numerically. This is fine — there's no persisted state depending on numeric values. BLE (Module 4) will use the new numeric values.
- `getCyclable()` already excludes BLINK by ID check, not by numeric value, so it needs no change.
- `getTimeBasedEmotion()` references only HAPPY, THINKING, IDLE, SLEEPY — all retained.

### Definition of done
- Enum has 12 values: IDLE, HAPPY, SLEEPY, EXCITED, SAD, ANGRY, CONFUSED, THINKING, LOVE, SURPRISED, DEAD, BLINK.
- `pio run` clean.
- `pio test -e native` passes. `test_registry_count` asserts 12.
- `emotion_draws.cpp` is under 1,200 LOC.
- Flash to device: all 12 emotions render correctly, speaker beeps for each.

### Integration test
```cpp
void test_all_retained_emotions_render() {
  MockCanvas canvas;
  EmotionState retained[] = {
    EMOTION_IDLE, EMOTION_HAPPY, EMOTION_SLEEPY, EMOTION_EXCITED,
    EMOTION_SAD, EMOTION_ANGRY, EMOTION_CONFUSED, EMOTION_THINKING,
    EMOTION_LOVE, EMOTION_SURPRISED, EMOTION_DEAD, EMOTION_BLINK
  };
  for (int e = 0; e < 12; e++) {
    for (int frame = 0; frame < 51; frame++) {
      canvas.reset();
      const EmotionDef* def = emotionRegistry.get(retained[e]);
      TEST_ASSERT_NOT_NULL(def);
      TEST_ASSERT_NOT_NULL(def->drawFrame);
      def->drawFrame(canvas, frame, nullptr);
      if (frame == 0) TEST_ASSERT_TRUE(canvas.callCount() > 0);
    }
  }
}
```

---

## Module 3: Add BORED and SHY Emotions

### Scope
Add two new enum values, two new draw functions, two new beep patterns, and register them. BORED and SHY are self-contained animations with no external data dependencies.

### Files

**Modify:**

**`include/emotion.h`** — Add to enum (after DEAD, before BLINK):
```cpp
  EMOTION_DEAD,
  EMOTION_BORED,
  EMOTION_SHY,
  EMOTION_BLINK
```
BLINK stays last among the "special" non-cyclable emotions. BORED and SHY are cyclable (blinkable: true for BORED since it's a slow idle variant; SHY is brief transitional so blinkable: false).

**`include/emotion_draws.h`** — Add declarations:
```cpp
void drawBored(ICanvas& canvas, int frame, const void* ctx);
void drawShy(ICanvas& canvas, int frame, const void* ctx);
```

**`src/emotion_draws.cpp`** — Add implementations:

`drawBored`: Droopy, slow-blink animation. 51 frames @ 60ms (slower than standard 30ms for lethargic feel).
- Frames 0–15: eyes slowly droop from full height (18) to half (9), eyelids half-closed
- Frames 16–25: hold half-closed, occasional slow blink (full close frame 20–22)
- Frames 26–40: eyes drift slightly downward (y offset +2), mouth becomes flat line
- Frames 41–50: reset back up, slow re-open
- Key visual: half-lidded eyes, flat mouth, slight head tilt via asymmetric eye Y positions (left eye Y = 28, right eye Y = 30)

`drawShy`: Brief bashful animation. 30 frames @ 40ms (~1.2s total, short transitional).
- Frames 0–7: eyes shrink inward (eye spacing narrows from 48px to 30px gap), blush circles appear
- Frames 8–15: eyes look down (Y shifts from 28 to 35), small curved mouth
- Frames 16–22: one eye peeks up slightly (right eye Y back to 30), blush intensifies
- Frames 23–29: both eyes slowly return to normal position, mouth becomes small smile
- Key visual: uses `drawBlush()` helper from ICanvas, narrowed eye spacing, downcast gaze

**`src/speaker.cpp`** — Add:
```cpp
static const BeepTone PATTERN_BORED[] = {
  {400, 300}, {0, 200}, {300, 400}
};
static const BeepTone PATTERN_SHY[] = {
  {1000, 40}, {0, 30}, {800, 40}, {0, 30}, {600, 60}
};
```
And add `PATTERN_ENTRY` lines to `EMOTION_PATTERNS[]`.

**`src/main.cpp`** — Add to `registerEmotions()`:
```cpp
emotionRegistry.add({EMOTION_BORED, "BORED", 51, 60, LOOP_RESTART, true, drawBored});
emotionRegistry.add({EMOTION_SHY, "SHY", 30, 40, LOOP_ONCE, false, drawShy});
```
SHY uses `LOOP_ONCE` because it's a transitional emotion — plays once, then the personality engine transitions to HAPPY.

### Implementation notes
- `AnimState states_[]` is sized `MAX_EMOTIONS = 20`, so adding 2 more emotions (now 14 total) is fine.
- `getCyclable()` will include BORED (it's not BLINK). It will also include SHY. The personality engine (Module 5) will be the only thing that sets SHY — autonomous cycling may occasionally land on it, which is acceptable (it plays once and holds).
- Both draw functions must use only ICanvas primitives already available in `canvas.h`. No new primitives needed.

### Definition of done
- Enum has 14 values.
- `pio run` clean.
- `pio test -e native` passes. `test_registry_count` asserts 14.
- Flash to device: cycling eventually shows BORED (slow droopy eyes) and SHY (bashful shrink then peek). Both have distinct beep patterns.
- BORED visually distinct from SLEEPY: SLEEPY fully closes eyes, BORED keeps them half-open with flat expression.

### Integration test
```cpp
void test_bored_draws_half_lidded_eyes() {
  MockCanvas canvas;
  drawBored(canvas, 20, nullptr);  // Mid-animation, eyes should be half-closed
  // Should have fillRoundRect calls for eyes with reduced height
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  // Half-lidded: eye height should be less than full 18
  TEST_ASSERT_TRUE(canvas.call(idx).h < 18);
}

void test_shy_uses_blush() {
  MockCanvas canvas;
  drawShy(canvas, 10, nullptr);  // Mid-animation, blush should be visible
  // Blush = fillCircle calls (drawBlush uses fillCircle internally)
  int idx = canvas.findCall(DrawCall::FILL_CIRCLE);
  TEST_ASSERT_TRUE(idx >= 0);
}
```

---

## Module 4: BLE Control

### Scope
Add NimBLE-Arduino BLE server with one service and one writable characteristic. Writing a single byte (emotion ID 0–13) sets the target emotion. Reading returns the current emotion. BLE advertising name: "SANGI".

### Files

**Create:**
- `include/ble_control.h`
- `src/ble_control.cpp`

**Modify:**

**`platformio.ini`** — Add to `[env:airm2m_core_esp32c3]` `lib_deps`:
```
h2zero/NimBLE-Arduino@^1.4.0
```

**`include/ble_control.h`**:
```cpp
#ifndef BLE_CONTROL_H
#define BLE_CONTROL_H

#include <Arduino.h>
#include "emotion.h"

// Callback for BLE-initiated emotion changes
typedef void (*BleEmotionFn)(EmotionState emotion);

class BleControl {
public:
  void init(BleEmotionFn onEmotion);
  void updateCurrentEmotion(uint8_t emotionId);
private:
  BleEmotionFn onEmotion_;
};

extern BleControl bleControl;

#endif
```

**`src/ble_control.cpp`**:
- Service UUID: `"face0001-0000-1000-8000-00805f9b34fb"` (custom, "face" prefix for discoverability)
- Characteristic UUID: `"face0002-0000-1000-8000-00805f9b34fb"`
- Properties: READ + WRITE
- `onWrite` callback: read single byte, validate against registry (`emotionRegistry.get()`), call `onEmotion_` if valid
- `onRead` callback: return current emotion ID byte
- Advertising: name "SANGI", include service UUID
- Stack: NimBLE uses ~2KB less RAM than Arduino BLE library — critical for ESP32-C3's 400KB SRAM

```cpp
#include "ble_control.h"
#include "emotion_registry.h"
#include <NimBLEDevice.h>

BleControl bleControl;

static BleEmotionFn s_callback = nullptr;
static uint8_t s_currentEmotion = 0;

#define SERVICE_UUID "face0001-0000-1000-8000-00805f9b34fb"
#define CHAR_UUID    "face0002-0000-1000-8000-00805f9b34fb"

class EmotionCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar) override {
    std::string val = pChar->getValue();
    if (val.length() >= 1) {
      uint8_t id = (uint8_t)val[0];
      if (emotionRegistry.get((EmotionState)id) != nullptr && s_callback) {
        s_callback((EmotionState)id);
      }
    }
  }
  void onRead(NimBLECharacteristic* pChar) override {
    pChar->setValue(&s_currentEmotion, 1);
  }
};

static EmotionCallbacks s_charCallbacks;

void BleControl::init(BleEmotionFn onEmotion) {
  onEmotion_ = onEmotion;
  s_callback = onEmotion;

  NimBLEDevice::init("SANGI");
  NimBLEDevice::setPower(ESP_PWR_LVL_P3);  // +3dBm, ~10m range

  NimBLEServer* pServer = NimBLEDevice::createServer();
  NimBLEService* pService = pServer->createService(SERVICE_UUID);
  NimBLECharacteristic* pChar = pService->createCharacteristic(
      CHAR_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  pChar->setCallbacks(&s_charCallbacks);
  pService->start();

  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->setScanResponse(true);
  pAdv->start();

  Serial.println("BLE: advertising as 'SANGI'");
}

void BleControl::updateCurrentEmotion(uint8_t emotionId) {
  s_currentEmotion = emotionId;
}
```

**`src/main.cpp`** — Add:
```cpp
#include "ble_control.h"

void onBleEmotion(EmotionState e) {
  emotionManager.setTargetEmotion(e);
  Serial.printf("BLE: emotion set to %s\n", emotionRegistry.getName(e));
}
```
In `setup()`: `bleControl.init(onBleEmotion);`
In `loop()` after `emotionManager.update()`: `bleControl.updateCurrentEmotion((uint8_t)emotionManager.getCurrentEmotion());`

### Implementation notes
- NimBLE-Arduino is preferred over the default ESP32 BLE library: ~60% less flash, ~50% less RAM. ESP32-C3 only has BLE (no classic Bluetooth), so NimBLE is the right choice.
- `ESP_PWR_LVL_P3` gives ~10m range. Desk gadget doesn't need more.
- BLE callbacks run on the NimBLE task, not `loop()`. The `emotionManager.setTargetEmotion()` call is thread-safe in practice because the emotion manager only reads/writes simple value types and the ESP32-C3 is single-core (no true concurrency, just task switching). Still, it's worth noting.
- The service UUID uses a custom base with `face` prefix so it's easy to find in scanner apps.

### Definition of done
- `pio run` succeeds (NimBLE compiles).
- Flash to device. Open nRF Connect on phone → scan → see "SANGI" → connect → write `0x01` to characteristic → robot shows HAPPY → write `0x04` to characteristic → robot shows SAD.
- Read characteristic returns current emotion byte.
- Invalid values (e.g., `0xFF`) silently ignored — no crash, no emotion change.

### Integration test
BLE can't be tested natively (NimBLE requires ESP-IDF). Instead, verify the validation logic:

Add to `test_sangi.cpp`:
```cpp
void test_ble_emotion_validation_rejects_unregistered() {
  // Simulate what BLE onWrite does: check registry before calling callback
  EmotionState invalid = (EmotionState)99;
  TEST_ASSERT_NULL(emotionRegistry.get(invalid));
  // Valid emotion
  TEST_ASSERT_NOT_NULL(emotionRegistry.get(EMOTION_HAPPY));
}
```

For on-device integration: document a manual test script using nRF Connect.

---

## Module 5: Touch Gesture Detection

### Scope
Replace the current single-event touch handler with gesture detection: tap, long press, double tap. The current `InputManager` debounces at 50ms and fires a single callback. This module adds timing-based gesture classification while keeping the `TouchHandlerFn` callback pattern.

### Files

**Modify:**

**`include/input.h`** — Add gesture types and extended callback:
```cpp
enum TouchGesture {
  GESTURE_NONE,
  GESTURE_TAP,
  GESTURE_LONG_PRESS,
  GESTURE_DOUBLE_TAP
};

typedef void (*GestureHandlerFn)(TouchGesture gesture, unsigned long currentTime);
```

Add to `InputManager`:
```cpp
  void setOnGesture(GestureHandlerFn fn) { onGesture_ = fn; }
private:
  GestureHandlerFn onGesture_;
  unsigned long touchStartTime_;
  unsigned long lastTapTime_;
  bool touchActive_;
  bool longPressFired_;
```

**`src/input.cpp`** — Rewrite `handleTouchInteraction()`:
- Track press start time on LOW→HIGH transition
- On release (HIGH→LOW):
  - Duration > 600ms → `GESTURE_LONG_PRESS`
  - Duration < 600ms and within 300ms of last tap → `GESTURE_DOUBLE_TAP`
  - Duration < 600ms otherwise → `GESTURE_TAP` (but delay 300ms before firing to check for double-tap; use a pending flag)
- Long press fires on threshold crossing while still held, not on release
- All timing constants live in `config.h`:
  ```cpp
  #define LONG_PRESS_MS 600
  #define DOUBLE_TAP_WINDOW_MS 300
  ```

**`src/main.cpp`** — Replace `onTouch` with:
```cpp
void onGesture(TouchGesture gesture, unsigned long currentTime) {
  switch (gesture) {
    case GESTURE_TAP:
      emotionManager.setTargetEmotion(EMOTION_HAPPY);
      break;
    case GESTURE_LONG_PRESS:
      emotionManager.setTargetEmotion(EMOTION_LOVE);
      break;
    case GESTURE_DOUBLE_TAP:
      emotionManager.setTargetEmotion(EMOTION_EXCITED);
      break;
    default: break;
  }
}
```
Wire: `inputManager.setOnGesture(onGesture);`

Remove old `onTouch` callback and `setOnTouch` wiring. Keep `setOnTouch` in header for backward compat if desired, or remove it.

### Implementation notes
- The pending-tap pattern: on release with duration < 600ms, set `pendingTap = true` and `pendingTapTime = currentTime`. On next `handleTouchInteraction()` call, if `pendingTap && currentTime - pendingTapTime > DOUBLE_TAP_WINDOW_MS`, fire `GESTURE_TAP`. If a new touch starts within the window, fire `GESTURE_DOUBLE_TAP` instead and clear `pendingTap`.
- The 50ms `delay()` debounce in the current code must be removed — it blocks the entire loop. Replace with a timestamp-based debounce (ignore transitions within 30ms of last transition).
- `handleTouchInteraction()` is called every `loop()` iteration (every ~50ms from the `delay(50)` at the end of loop). This is fast enough for gesture detection.

### Definition of done
- `pio run` clean.
- Flash to device:
  - Quick tap → HAPPY with beep
  - Hold 1s → LOVE with beep (fires while still holding, not on release)
  - Two quick taps → EXCITED with beep
- Serial output logs gesture type.

### Integration test
```cpp
void test_gesture_tap_detected() {
  // Simulate: touch at t=0, release at t=100ms, wait 350ms
  // This requires stubbing digitalRead — add to Arduino.h stub:
  //   inline int& _stubDigitalReadRef() { static int val = HIGH; return val; }
  //   inline int digitalRead(uint8_t) { return _stubDigitalReadRef(); }
  //   inline void stubSetDigitalRead(int v) { _stubDigitalReadRef() = v; }

  TouchGesture lastGesture = GESTURE_NONE;
  inputManager.setOnGesture([](TouchGesture g, unsigned long) {
    // Can't capture in C callback — use a global instead
  });
  // Due to C function pointer limitations, test gesture timing logic directly
  // by checking state transitions rather than callback invocation.
  // Verify tap timing: 100ms press < 600ms threshold = TAP candidate
  TEST_ASSERT_TRUE(100 < LONG_PRESS_MS);
}
```

More meaningful test: add a `classifyGesture(unsigned long pressDuration, unsigned long sincePrevTap)` static function to `input.cpp` and test that directly:
```cpp
void test_classify_gesture_tap() {
  TEST_ASSERT_EQUAL(GESTURE_TAP, classifyGesture(100, 999));
}
void test_classify_gesture_long_press() {
  TEST_ASSERT_EQUAL(GESTURE_LONG_PRESS, classifyGesture(700, 999));
}
void test_classify_gesture_double_tap() {
  TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, classifyGesture(100, 200));
}
```

Expose `classifyGesture` in `input.h` for testability (or put it in a `touch_util.h`).

---

## Module 6: Personality Engine

### Scope
The core autonomous behavior system. Replaces the flat random cycling in `loop()` with four subsystems: mood drift, attention arc, touch response integration, and micro-expressions. All timing uses jittered intervals.

### Files

**Create:**
- `include/personality.h`
- `src/personality.cpp`

**Modify:**
- `src/main.cpp` — remove autonomous cycling logic, wire personality
- `include/config.h` — add personality timing constants
- `platformio.ini` `[env:native]` — add `+<personality.cpp>` to `build_src_filter`

**`include/config.h`** — Add:
```cpp
// ===== PERSONALITY CONFIGURATION =====
#define ATTENTION_STAGE1_MS 300000    // 5 min → BORED (base, ±20% jitter)
#define ATTENTION_STAGE2_MS 600000    // 10 min → SAD
#define ATTENTION_STAGE3_MS 750000    // 12.5 min → CONFUSED
#define ATTENTION_STAGE4_MS 900000    // 15 min → ANGRY
#define MOOD_DRIFT_INTERVAL_MS 120000 // 2 min between mood drift checks (base)
#define MICRO_EXPRESSION_CHANCE 15    // % chance per drift check to do a micro-expression
#define JITTER_PERCENT 20             // ±20% applied to all personality timings
```

**`include/personality.h`**:
```cpp
#ifndef PERSONALITY_H
#define PERSONALITY_H

#include <Arduino.h>
#include "emotion.h"

class Personality {
public:
  Personality();
  void init(unsigned long currentTime);

  // Call every loop(). Returns emotion to set, or EMOTION_IDLE with
  // shouldChange=false if no change needed.
  struct Decision {
    EmotionState emotion;
    bool shouldChange;
  };
  Decision update(unsigned long currentTime, EmotionState currentEmotion);

  // Call when touch occurs — resets attention timer, handles recovery arc
  void onTouch(unsigned long currentTime, EmotionState currentEmotion);

  // For testing: inspect internal state
  int getAttentionStage() const { return attentionStage_; }
  unsigned long getLastTouchTime() const { return lastTouchTime_; }

private:
  unsigned long lastTouchTime_;
  unsigned long lastDriftTime_;
  unsigned long nextDriftInterval_;    // jittered
  unsigned long attentionStageTime_;   // when current stage was entered
  int attentionStage_;                 // 0=none, 1=BORED, 2=SAD, 3=CONFUSED, 4=ANGRY

  unsigned long jitter(unsigned long base);
  EmotionState moodDrift(unsigned long currentTime);
  Decision attentionArc(unsigned long currentTime, EmotionState current);
  Decision touchRecovery(EmotionState current);
  bool shouldMicroExpress();
};

extern Personality personality;

#endif
```

**`src/personality.cpp`**:

`jitter(base)`: returns `base + random(-base*JITTER_PERCENT/100, base*JITTER_PERCENT/100)`.

`moodDrift(currentTime)`: Uses hour-of-uptime (or RTC if available, but for prototype use uptime):
- Hour 0–5 (night): weighted toward SLEEPY (60%), IDLE (30%), THINKING (10%)
- Hour 6–11 (morning): weighted toward HAPPY (40%), EXCITED (30%), IDLE (20%), THINKING (10%)
- Hour 12–17 (afternoon): weighted toward THINKING (30%), IDLE (30%), HAPPY (20%), CONFUSED (10%), SURPRISED (10%)
- Hour 18–23 (evening): weighted toward IDLE (30%), SLEEPY (30%), SAD (15%), LOVE (15%), THINKING (10%)

Weighted random: generate `r = random(0, 100)`, accumulate weights, return matching emotion.

`attentionArc(currentTime, current)`: Check `currentTime - lastTouchTime_` against jittered thresholds. Each stage transition jitters independently — don't jitter once at the start, jitter each threshold crossing separately. If `attentionStage_` should advance, set `shouldChange = true` and return the appropriate emotion. Don't re-trigger if already at the target emotion.

`touchRecovery(current)`: If `attentionStage_ > 0` (was being neglected), reset `attentionStage_ = 0` and return `EMOTION_SHY`. The caller (main.cpp) handles the SHY → HAPPY transition. If `attentionStage_ == 0`, return based on gesture (but gesture mapping is in main.cpp, not here). Set a flag `recoveryActive_` so that on the *next* `update()` call after SHY finishes, we transition to HAPPY.

Actually, cleaner: `onTouch` returns a `Decision`. Main.cpp's gesture handler calls `personality.onTouch()` first. If the personality returns `shouldChange = true` (recovery arc), use that. Otherwise, fall through to the gesture mapping.

`shouldMicroExpress()`: `random(0, 100) < MICRO_EXPRESSION_CHANCE`. If true, return a brief blink variation or SURPRISED flash. Implemented as: set BLINK for one cycle, then back to current.

`update()` logic:
1. If recovery is active and current emotion is SHY, check if SHY animation completed (frame reached end of LOOP_ONCE). If so, return HAPPY. *Problem*: personality doesn't know the animation frame. *Solution*: use a timer. SHY is 30 frames × 40ms = 1.2s. After `onTouch` sets SHY, set `recoveryEndTime_ = currentTime + 1300` (with margin). When `currentTime > recoveryEndTime_`, transition to HAPPY.
2. Check attention arc — if escalation needed, return it.
3. Check mood drift timer — if enough time passed, return a drifted emotion.
4. Check micro-expression — random chance to insert a blink.
5. Otherwise, `shouldChange = false`.

**`src/main.cpp`** — Replace autonomous cycling with:
```cpp
// In loop():
Personality::Decision d = personality.update(currentTime, emotionManager.getCurrentEmotion());
if (d.shouldChange) {
  emotionManager.setTargetEmotion(d.emotion);
}
```

Gesture handler becomes:
```cpp
void onGesture(TouchGesture gesture, unsigned long currentTime) {
  personality.onTouch(currentTime, emotionManager.getCurrentEmotion());

  // If personality handled it (recovery arc), it already decided the emotion.
  // Otherwise, apply gesture mapping.
  if (personality.getAttentionStage() == 0) {
    // No recovery needed — normal gesture response
    // But onTouch already reset the timer, so just map gesture:
    switch (gesture) {
      case GESTURE_TAP:
        emotionManager.setTargetEmotion(EMOTION_HAPPY);
        break;
      case GESTURE_LONG_PRESS:
        emotionManager.setTargetEmotion(EMOTION_LOVE);
        break;
      case GESTURE_DOUBLE_TAP:
        emotionManager.setTargetEmotion(EMOTION_EXCITED);
        break;
      default: break;
    }
  }
}
```

Wait — this has a logic issue. `onTouch` resets `attentionStage_` to 0, so by the time we check `getAttentionStage()`, it's already 0. Fix: `onTouch` returns a bool indicating whether recovery was triggered:
```cpp
bool wasNeglected = personality.onTouch(currentTime, emotionManager.getCurrentEmotion());
if (wasNeglected) {
  emotionManager.setTargetEmotion(EMOTION_SHY);
} else {
  // gesture mapping
}
```

### Implementation notes
- `Personality::update()` must be called every loop iteration, but mood drift only fires every ~2 minutes. The function is cheap — just timestamp comparisons.
- The jitter function must handle unsigned underflow: if `base * JITTER_PERCENT / 100` > `base`, clamp to 0.
- Attention arc uses real-time thresholds (5/10/12.5/15 min defaults), but each threshold is re-jittered when the stage is entered. Store `nextStageThreshold_` and recalculate on each stage transition.
- SHY → HAPPY auto-transition uses a fixed timer (1.3s), not animation frame inspection. This is intentionally decoupled from the animation system.

### Definition of done
- `pio run` clean.
- `pio test -e native` passes.
- Flash to device:
  - Leave untouched for 5+ minutes: emotion changes to BORED. Continue ignoring: SAD → CONFUSED → ANGRY.
  - Touch during ANGRY: SHY appears briefly (~1.2s), then HAPPY.
  - During normal operation: emotions drift naturally, not the same emotion every time.
  - Time-of-day influence visible: leave running overnight, morning emotions skew HAPPY/EXCITED.
- Serial output logs personality decisions: `[Personality] Attention stage 1 → BORED`, `[Personality] Recovery: SHY → HAPPY`.

### Integration test
```cpp
void test_attention_arc_escalates() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);  // Reset timer at t=0

  // Simulate 5+ minutes passing (beyond ATTENTION_STAGE1_MS with max jitter)
  unsigned long t = ATTENTION_STAGE1_MS + ATTENTION_STAGE1_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  Personality::Decision d = p.update(t, EMOTION_IDLE);
  // Should have escalated to at least stage 1
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);
}

void test_touch_during_neglect_triggers_recovery() {
  Personality p;
  p.init(0);
  // Force into neglect state
  unsigned long t = ATTENTION_STAGE2_MS + ATTENTION_STAGE2_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);  // Should be at stage 2 (SAD)
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);

  // Touch — should trigger recovery
  bool wasNeglected = p.onTouch(t, EMOTION_SAD);
  TEST_ASSERT_TRUE(wasNeglected);
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());
}

void test_jitter_produces_variance() {
  Personality p;
  p.init(0);
  // Call jitter 20 times, verify not all results are identical
  unsigned long results[20];
  for (int i = 0; i < 20; i++) {
    results[i] = p.jitter(100000);  // Make jitter public for test, or use a test friend
  }
  bool allSame = true;
  for (int i = 1; i < 20; i++) {
    if (results[i] != results[0]) { allSame = false; break; }
  }
  TEST_ASSERT_FALSE(allSame);
}
```

Note on `jitter` testability: either make it `public` (it's a utility), or add a static free function `jitterValue(unsigned long base, int percent)` in `personality.cpp` and expose it in the header for testing.

---

## Module 7: Captive Portal (Optional)

### Scope
ESP32 hosts a WiFi AP with a minimal web page for controlling emotions. No internet required. Single HTML page served from PROGMEM. This is the "show it to a friend" feature — they connect to SANGI's WiFi and tap buttons.

### Files

**Create:**
- `include/web_control.h`
- `src/web_control.cpp`

**Modify:**
- `src/main.cpp` — init and update
- `include/config.h` — add AP settings

**`include/config.h`**:
```cpp
// ===== CAPTIVE PORTAL =====
#define AP_SSID "SANGI"
#define AP_PASSWORD ""  // Open network for easy access
#define WEB_SERVER_PORT 80
```

**`src/web_control.cpp`**:
- `WiFi.softAP(AP_SSID, AP_PASSWORD)`
- Single `WebServer` on port 80
- Route `/` serves a self-contained HTML page with emotion buttons (inline CSS, no external deps)
- Route `/emotion?id=N` sets emotion via callback, returns 200
- Route `/status` returns JSON: `{"emotion": "HAPPY", "battery": 3.85, "uptime": 1234}`
- HTML page: grid of emotion buttons, each sends fetch to `/emotion?id=N`. Auto-refreshes status every 5s. Dark theme, large touch-friendly buttons.
- Total HTML: ~2KB in PROGMEM (ESP32-C3 has 4MB flash, this is negligible).

**`src/main.cpp`**:
```cpp
#include "web_control.h"
// In setup(): webControl.init(onBleEmotion);  // reuse same callback
// In loop(): webControl.update();  // handles HTTP client
```

### Implementation notes
- `WiFi.softAP()` and NimBLE coexist on ESP32-C3 — both use the radio but on different protocols. No conflict.
- `WebServer` library is built into ESP32 Arduino core — no additional dependency.
- The `webServer.handleClient()` call in `loop()` is non-blocking (returns immediately if no pending request).
- No DNS server / captive portal redirect needed for the prototype — users just navigate to `192.168.4.1` after connecting to the AP. If desired later, add `DNSServer` for auto-redirect.

### Definition of done
- `pio run` clean.
- Flash to device. Phone connects to "SANGI" WiFi. Navigate to `192.168.4.1`. Page loads with emotion buttons. Tap "HAPPY" → robot face changes. Status shows battery and uptime.
- BLE still works simultaneously.

### Integration test
No native test possible (WiFi/WebServer need ESP-IDF). Manual test procedure:
1. Flash firmware
2. Connect phone to "SANGI" WiFi
3. Open `192.168.4.1` in browser
4. Tap each of the 14 emotion buttons — verify face changes
5. Verify `/status` returns valid JSON
6. Verify BLE still works (connect with nRF Connect while web page is open)

---

## Final Integration Test Suite

These tests run via `pio test -e native`. They cover the full personality arc, emotion system integrity, and variance validation.

### Test 1: Full attention arc progression
```cpp
void test_full_attention_arc_idle_to_angry() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  EmotionState expected[] = {EMOTION_BORED, EMOTION_SAD, EMOTION_CONFUSED, EMOTION_ANGRY};
  unsigned long thresholds[] = {ATTENTION_STAGE1_MS, ATTENTION_STAGE2_MS,
                                 ATTENTION_STAGE3_MS, ATTENTION_STAGE4_MS};

  for (int stage = 0; stage < 4; stage++) {
    // Jump past the max-jittered threshold
    unsigned long t = thresholds[stage] + thresholds[stage] * JITTER_PERCENT / 100 + 5000;
    stubSetMillis(t);
    Personality::Decision d = p.update(t, (stage == 0) ? EMOTION_IDLE : expected[stage - 1]);
    TEST_ASSERT_EQUAL(stage + 1, p.getAttentionStage());
  }
}
```

### Test 2: Touch recovery from any neglect stage
```cpp
void test_touch_recovery_from_any_stage() {
  for (int targetStage = 1; targetStage <= 4; targetStage++) {
    Personality p;
    p.init(0);
    p.onTouch(0, EMOTION_IDLE);

    // Fast-forward to target stage
    unsigned long t = 0;
    for (int s = 0; s < targetStage; s++) {
      unsigned long thresholds[] = {ATTENTION_STAGE1_MS, ATTENTION_STAGE2_MS,
                                     ATTENTION_STAGE3_MS, ATTENTION_STAGE4_MS};
      t = thresholds[s] + thresholds[s] * JITTER_PERCENT / 100 + 5000;
      stubSetMillis(t);
      p.update(t, EMOTION_IDLE);
    }
    TEST_ASSERT_EQUAL(targetStage, p.getAttentionStage());

    // Touch — recovery
    bool wasNeglected = p.onTouch(t, EMOTION_ANGRY);
    TEST_ASSERT_TRUE(wasNeglected);
    TEST_ASSERT_EQUAL(0, p.getAttentionStage());
  }
}
```

### Test 3: SHY auto-transitions to HAPPY
```cpp
void test_shy_auto_transitions_to_happy() {
  Personality p;
  p.init(0);
  // Simulate neglect then recovery
  unsigned long t = ATTENTION_STAGE1_MS * 2;
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);
  p.onTouch(t, EMOTION_BORED);  // triggers SHY

  // 1.5s later, update should produce HAPPY
  t += 1500;
  stubSetMillis(t);
  Personality::Decision d = p.update(t, EMOTION_SHY);
  TEST_ASSERT_TRUE(d.shouldChange);
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, d.emotion);
}
```

### Test 4: Touch resets attention timer
```cpp
void test_touch_resets_attention_timer() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Advance to just before stage 1 threshold
  unsigned long t = ATTENTION_STAGE1_MS - 10000;
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());

  // Touch — resets timer
  p.onTouch(t, EMOTION_IDLE);

  // Advance same duration again — should NOT trigger stage 1 because timer was reset
  t += ATTENTION_STAGE1_MS - 10000;
  stubSetMillis(t);
  Personality::Decision d = p.update(t, EMOTION_IDLE);
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());
}
```

### Test 5: Gesture mapping correctness
```cpp
void test_gesture_classification() {
  TEST_ASSERT_EQUAL(GESTURE_TAP, classifyGesture(100, 999));
  TEST_ASSERT_EQUAL(GESTURE_TAP, classifyGesture(500, 999));
  TEST_ASSERT_EQUAL(GESTURE_LONG_PRESS, classifyGesture(700, 999));
  TEST_ASSERT_EQUAL(GESTURE_LONG_PRESS, classifyGesture(2000, 999));
  TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, classifyGesture(100, 200));
  TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, classifyGesture(80, 50));
  // Edge: exactly at threshold
  TEST_ASSERT_EQUAL(GESTURE_LONG_PRESS, classifyGesture(LONG_PRESS_MS, 999));
  TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, classifyGesture(100, DOUBLE_TAP_WINDOW_MS));
}
```

### Test 6: BLE validation rejects unregistered emotions
```cpp
void test_ble_rejects_out_of_range() {
  for (int i = 14; i < 256; i++) {
    TEST_ASSERT_NULL(emotionRegistry.get((EmotionState)i));
  }
  // All 14 registered emotions are valid
  for (int i = 0; i < 14; i++) {
    // Can't iterate enum directly — check known values
  }
  TEST_ASSERT_NOT_NULL(emotionRegistry.get(EMOTION_IDLE));
  TEST_ASSERT_NOT_NULL(emotionRegistry.get(EMOTION_BORED));
  TEST_ASSERT_NOT_NULL(emotionRegistry.get(EMOTION_SHY));
  TEST_ASSERT_NOT_NULL(emotionRegistry.get(EMOTION_BLINK));
}
```

### Test 7: Emotion transition integrity
```cpp
void test_transition_preserves_state_through_full_cycle() {
  emotionManager.init(0);
  EmotionState sequence[] = {EMOTION_HAPPY, EMOTION_BORED, EMOTION_SHY,
                              EMOTION_ANGRY, EMOTION_LOVE, EMOTION_IDLE};
  for (int i = 0; i < 6; i++) {
    EmotionState prev = emotionManager.getCurrentEmotion();
    emotionManager.setTargetEmotion(sequence[i]);
    TEST_ASSERT_TRUE(emotionManager.isTransitionActive());
    emotionManager.completeTransition();
    TEST_ASSERT_EQUAL(sequence[i], emotionManager.getCurrentEmotion());
    TEST_ASSERT_EQUAL(prev, emotionManager.getPreviousEmotion());
  }
}
```

### Test 8: BORED and SHY draw functions don't crash across all frames
```cpp
void test_bored_shy_full_frame_range() {
  MockCanvas canvas;
  // BORED: 51 frames
  for (int f = 0; f < 51; f++) {
    canvas.reset();
    drawBored(canvas, f, nullptr);
    TEST_ASSERT_TRUE(canvas.callCount() > 0);
  }
  // SHY: 30 frames
  for (int f = 0; f < 30; f++) {
    canvas.reset();
    drawShy(canvas, f, nullptr);
    TEST_ASSERT_TRUE(canvas.callCount() > 0);
  }
}
```

### Test 9: Mood drift produces varied output (variance validation)
```cpp
void test_mood_drift_variance() {
  // Run 100 mood drift decisions, verify distribution is not degenerate
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  int emotionCounts[14] = {};
  for (int i = 0; i < 100; i++) {
    // Set time to trigger drift each iteration
    unsigned long t = (unsigned long)(i + 1) * (MOOD_DRIFT_INTERVAL_MS + 1000);
    stubSetMillis(t);
    Personality::Decision d = p.update(t, EMOTION_IDLE);
    if (d.shouldChange) {
      int idx = (int)d.emotion;
      if (idx >= 0 && idx < 14) emotionCounts[idx]++;
    }
  }

  // At least 3 different emotions should appear in 100 samples
  int uniqueEmotions = 0;
  for (int i = 0; i < 14; i++) {
    if (emotionCounts[i] > 0) uniqueEmotions++;
  }
  TEST_ASSERT_TRUE(uniqueEmotions >= 3);

  // No single emotion should dominate >60% (with reasonable probability)
  int maxCount = 0;
  for (int i = 0; i < 14; i++) {
    if (emotionCounts[i] > maxCount) maxCount = emotionCounts[i];
  }
  // Total decisions that changed:
  int totalChanged = 0;
  for (int i = 0; i < 14; i++) totalChanged += emotionCounts[i];
  if (totalChanged > 10) {
    TEST_ASSERT_TRUE(maxCount < totalChanged * 60 / 100);
  }
}
```

### Test 10: Jitter variance validation
```cpp
void test_jitter_distribution() {
  // 50 samples of jitter(100000) should:
  // 1. All fall within [80000, 120000] (±20%)
  // 2. Not all be identical
  // 3. Have a spread (max - min) > 5% of base
  Personality p;
  p.init(0);
  unsigned long samples[50];
  unsigned long minVal = ULONG_MAX, maxVal = 0;

  for (int i = 0; i < 50; i++) {
    samples[i] = p.jitter(100000);
    if (samples[i] < minVal) minVal = samples[i];
    if (samples[i] > maxVal) maxVal = samples[i];
    // Bounds check
    TEST_ASSERT_TRUE(samples[i] >= 80000);
    TEST_ASSERT_TRUE(samples[i] <= 120000);
  }

  // Spread check
  unsigned long spread = maxVal - minVal;
  TEST_ASSERT_TRUE(spread > 5000);  // > 5% of 100000
}
```

### Test 11: Attention arc doesn't fire prematurely
```cpp
void test_no_premature_attention_escalation() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Check at 50% of stage 1 threshold (even with max jitter, shouldn't fire)
  unsigned long safeTime = (ATTENTION_STAGE1_MS * (100 - JITTER_PERCENT)) / 200;
  stubSetMillis(safeTime);
  Personality::Decision d = p.update(safeTime, EMOTION_IDLE);
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());
}
```

### Test 12: Registry integrity after all modifications
```cpp
void test_final_registry_integrity() {
  TEST_ASSERT_EQUAL(14, emotionRegistry.count());

  // Every registered emotion has a non-null draw function
  EmotionState all[] = {
    EMOTION_IDLE, EMOTION_HAPPY, EMOTION_SLEEPY, EMOTION_EXCITED,
    EMOTION_SAD, EMOTION_ANGRY, EMOTION_CONFUSED, EMOTION_THINKING,
    EMOTION_LOVE, EMOTION_SURPRISED, EMOTION_DEAD, EMOTION_BORED,
    EMOTION_SHY, EMOTION_BLINK
  };
  for (int i = 0; i < 14; i++) {
    const EmotionDef* def = emotionRegistry.get(all[i]);
    TEST_ASSERT_NOT_NULL_MESSAGE(def, emotionRegistry.getName(all[i]));
    TEST_ASSERT_NOT_NULL(def->drawFrame);
    TEST_ASSERT_TRUE(def->frameCount > 0);
  }

  // getCyclable excludes only BLINK
  EmotionState cyclable[20];
  int n = emotionRegistry.getCyclable(cyclable, 20);
  TEST_ASSERT_EQUAL(13, n);  // 14 total minus BLINK
  for (int i = 0; i < n; i++) {
    TEST_ASSERT_NOT_EQUAL(EMOTION_BLINK, cyclable[i]);
  }
}
```
