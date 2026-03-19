# SANGI Refactoring Plan — Phases 2–7

Phase 1 (Foundation) is complete. This document covers the remaining six phases with exact file changes, code patterns, and rationale.

---

## Phase 2: Animation Engine Rewrite

**Goal**: Eliminate 2,652 lines of near-identical switch-statement animation code. Make adding a new emotion require writing one draw function and one registry call — not touching 6 files.

### 2.1 — Consolidate Frame State

**Problem**: AnimationManager has 28 member variables — 14 `*Frame` counters and 14 `last*Anim` timestamps — all doing the same thing.

**File**: `include/animations.h`

Replace:
```cpp
unsigned long lastSleepyAnim;
unsigned long lastThinkAnim;
// ... 12 more
int sleepyFrame;
int thinkFrame;
// ... 12 more
```

With:
```cpp
struct AnimState {
  int frame;
  unsigned long lastTick;
};

AnimState states_[EMOTION_GITHUB_STATS + 1];  // Indexed by EmotionState
```

This collapses 28 variables into one array. `resetAnimation()` becomes:
```cpp
void AnimationManager::resetAnimation(EmotionState emotion) {
  states_[emotion].frame = 0;
  states_[emotion].lastTick = 0;
}
```

### 2.2 — Generic tick() Method

**Problem**: Every `animate*()` method has identical boilerplate:
```cpp
void AnimationManager::animateXxx() {
  unsigned long currentTime = millis();
  if (currentTime - lastXxxAnim > FRAME_DELAY) {
    displayManager.clearDisplay();
    switch(xxxFrame) { /* 50+ cases */ }
    displayManager.updateDisplay();
    xxxFrame = (xxxFrame + 1) % TOTAL_FRAMES;
    lastXxxAnim = currentTime;
  }
}
```

**Solution**: One generic `tick()` that looks up the emotion in the registry and calls its `DrawFrameFn`:

**File**: `include/animations.h` (new API)
```cpp
class AnimationManager {
public:
  AnimationManager();
  void resetAnimation(EmotionState emotion);

  // Generic: advances frame and calls the registered DrawFrameFn.
  // Returns true if a frame was drawn this call.
  bool tick(EmotionState emotion, ICanvas& canvas, const void* context = nullptr);

  // Legacy methods kept during migration (Phase 2 removes them one by one)
  void animateNotification(const char* title, const char* message);
  // ...

private:
  AnimState states_[EMOTION_GITHUB_STATS + 1];
};
```

**File**: `src/animations.cpp` (new `tick` implementation)
```cpp
bool AnimationManager::tick(EmotionState emotion, ICanvas& canvas,
                            const void* context) {
  const EmotionDef* def = emotionRegistry.get(emotion);
  if (!def || !def->drawFrame) return false;

  AnimState& s = states_[emotion];
  unsigned long now = millis();

  if (now - s.lastTick < def->frameDelay) return false;

  canvas.clear();
  def->drawFrame(canvas, s.frame, context);
  canvas.flush();

  // Advance frame
  if (def->loop == LOOP_RESTART) {
    s.frame = (s.frame + 1) % def->frameCount;
  } else {  // LOOP_ONCE
    if (s.frame < def->frameCount - 1) s.frame++;
  }
  s.lastTick = now;
  return true;
}
```

### 2.3 — Convert Each Animation to a Standalone DrawFrameFn

Each existing `animateXxx()` method's switch-statement body becomes a free function:

**File**: `src/emotion_draws.cpp` (new file, ~2000 lines but organized)
```cpp
#include "canvas.h"
#include "emotion_registry.h"

// Example: sleepy draw function
void drawSleepy(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 20);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 29, 88, 29, 16);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    // ... same pixel art, but using ICanvas instead of displayManager.getDisplay()
  }
}

// Static emotions (1 frame)
void drawIdle(ICanvas& canvas, int frame, const void* ctx) {
  canvas.drawEyes(40, 28, 88, 28, 20);
}

void drawBlink(ICanvas& canvas, int frame, const void* ctx) {
  canvas.drawEyes(40, 28, 88, 28, 4);
}

// ... one function per emotion
```

### 2.4 — Register DrawFrameFns in `registerEmotions()`

**File**: `src/main.cpp` — update registry calls:
```cpp
// Forward declarations (defined in emotion_draws.cpp)
void drawIdle(ICanvas&, int, const void*);
void drawHappy(ICanvas&, int, const void*);
void drawSleepy(ICanvas&, int, const void*);
// ...

void registerEmotions() {
  emotionRegistry.add({EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, drawIdle});
  emotionRegistry.add({EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, drawHappy});
  emotionRegistry.add({EMOTION_SLEEPY, "SLEEPY", 51, 30, LOOP_RESTART, false, drawSleepy});
  // ...
}
```

### 2.5 — Replace main.cpp Switch with `tick()`

The 30-case switch in `loop()` becomes:
```cpp
} else {
  EmotionState current = emotionManager.getCurrentEmotion();

  if (current == EMOTION_NOTIFICATION) {
    // Notification needs context (title + message)
    Notification* notif = networkManager.getCurrentNotification();
    const char* ctx[2] = {
      notif ? notif->title : offlineNotifTitle,
      notif ? notif->message : offlineNotifMessage
    };
    animationManager.tick(current, displayManager, ctx);
  } else {
    animationManager.tick(current, displayManager);
  }
}
```

### 2.6 — Extract Common Drawing Helpers

**Problem**: Repeated patterns across animations:
- Zzz floating text (sleepy) — 5 near-identical blocks
- Exclamation mark at various sizes (thinking) — 8 blocks
- Sparkle dots (excited, happy) — 12 blocks
- Question marks floating (confused) — 6 blocks
- Heart shapes (love) — drawn inline every frame

**Solution**: Add helpers to `ICanvas` or as free functions in `emotion_draws.cpp`:
```cpp
// In emotion_draws.cpp (local helpers)
static void drawZzz(ICanvas& c, int phase) {
  // phase 0-5 controls position progression
  c.setTextSize(1);
  int baseX = 85 + phase * 3;
  int baseY = 35 - phase * 4;
  for (int i = 0; i <= min(phase, 4); i++) {
    c.setCursor(baseX + i * 6, baseY - i * 4);
    c.print(i == 3 ? "Z" : "z");
  }
}

static void drawExclamation(ICanvas& c, int x, int stemW, int stemH) {
  c.fillRect(x, 6, stemW, stemH, COLOR_WHITE);
  c.fillRect(x, 6 + stemH + 2, stemW, stemW, COLOR_WHITE);
}

static void drawHeart(ICanvas& c, int cx, int cy, int r) {
  c.fillCircle(cx - r/2, cy - r/3, r/2, COLOR_WHITE);
  c.fillCircle(cx + r/2, cy - r/3, r/2, COLOR_WHITE);
  c.fillRect(cx - r, cy - r/3, 2 * r, r/3, COLOR_WHITE);
  c.fillTriangle(cx - r, cy, cx, cy + r, cx + r, cy, COLOR_WHITE);
}
```

### 2.7 — Delete Legacy Code

Once all emotions are migrated:
- Delete all `animateXxx()` methods from `animations.cpp`
- Remove individual frame/timing variables
- Remove `drawFace_*` methods from `display.cpp` (replaced by single-frame DrawFrameFns)
- Remove `drawEmotionFace()` switch (replaced by `tick()` with frameCount=1)

**Net result**: `animations.cpp` shrinks from ~2,652 lines to ~200 lines (generic `tick()` + `resetAnimation()`). Animation content moves to `emotion_draws.cpp` (~1,800 lines, but each function is self-contained and independently testable).

### 2.8 — Speaker Pattern Table

**File**: `src/speaker.cpp`

Replace the 14-case switch in `queueEmotionBeep()` with a lookup table:
```cpp
struct EmotionPattern {
  EmotionState emotion;
  const BeepTone* pattern;
  int length;
};

static const EmotionPattern EMOTION_PATTERNS[] = {
  {EMOTION_IDLE, PATTERN_IDLE, sizeof(PATTERN_IDLE) / sizeof(BeepTone)},
  {EMOTION_HAPPY, PATTERN_HAPPY, sizeof(PATTERN_HAPPY) / sizeof(BeepTone)},
  // ...
};
static const int NUM_PATTERNS = sizeof(EMOTION_PATTERNS) / sizeof(EmotionPattern);

void BeepManager::queueEmotionBeep(EmotionState emotion) {
  if (!ENABLE_EMOTION_BEEP || isActive) return;
  for (int i = 0; i < NUM_PATTERNS; i++) {
    if (EMOTION_PATTERNS[i].emotion == emotion) {
      startBeep(EMOTION_PATTERNS[i].pattern, EMOTION_PATTERNS[i].length);
      return;
    }
  }
}
```

### Files Changed (Phase 2)
| File | Action |
|------|--------|
| `include/animations.h` | Rewrite: AnimState array + tick() API |
| `src/animations.cpp` | Rewrite: generic tick(), remove animate*() methods |
| `src/emotion_draws.cpp` | **New**: all DrawFrameFn implementations |
| `src/emotion_draws.h` | **New**: forward declarations of draw functions |
| `include/display.h` | Remove drawFace_* declarations, drawEmotionFace() |
| `src/display.cpp` | Remove drawFace_* implementations (~250 lines) |
| `src/main.cpp` | Replace emotion switch with tick() call |
| `src/speaker.cpp` | Replace switch with lookup table |

---

## Phase 3: NetworkManager Decomposition

**Goal**: Split the 1,043-line god object into focused components. Add a subscription registry so new MQTT topics require zero changes to switch statements.

### 3.1 — Extract NotificationQueue

**File**: `include/notification_queue.h` (new)
```cpp
#ifndef NOTIFICATION_QUEUE_H
#define NOTIFICATION_QUEUE_H

#include <Arduino.h>

enum NotificationType {
  NOTIFY_GENERIC, NOTIFY_DISCORD, NOTIFY_SLACK,
  NOTIFY_EMAIL, NOTIFY_GITHUB, NOTIFY_CALENDAR, NOTIFY_SYSTEM
};

struct Notification {
  NotificationType type;
  char title[32];
  char message[64];
  unsigned long timestamp;
  bool active;
};

class NotificationQueue {
public:
  static const int MAX_SIZE = 5;

  NotificationQueue();

  bool add(NotificationType type, const char* title, const char* msg);
  Notification* current();
  void clearCurrent();
  bool hasItems() const { return count_ > 0; }
  int count() const { return count_; }

private:
  Notification items_[MAX_SIZE];
  int count_;
  int currentIdx_;
};

#endif
```

### 3.2 — Extract GitHubDataStore

**File**: `include/github_data.h` (new)
```cpp
#ifndef GITHUB_DATA_H
#define GITHUB_DATA_H

#include <Arduino.h>

struct GitHubContributionData {
  uint8_t contributions[52][7];
  int totalContributions;
  int currentStreak;
  int longestStreak;
  char username[32];
  bool dataLoaded;
};

struct GitHubStatsData {
  char username[32];
  int repos, followers, following;
  int contributions, commits, prs, issues, stars;
  unsigned long timestamp;
  bool dataLoaded;
};

class GitHubDataStore {
public:
  GitHubDataStore();

  void setContributions(const uint8_t grid[52][7], int total,
                        int streak, int longest, const char* user);
  void setStats(const char* user, int repos, int followers, int following,
                int contribs, int commits, int prs, int issues, int stars);

  const GitHubContributionData* getContributions() const;
  const GitHubStatsData* getStats() const;
  bool hasContributions() const { return contributions_.dataLoaded; }
  bool hasStats() const { return stats_.dataLoaded; }
  void clearContributions();

private:
  GitHubContributionData contributions_;
  GitHubStatsData stats_;
};

#endif
```

### 3.3 — MQTT Subscription Registry

**Problem**: `handleIncomingMessage()` is a 150-line if/else chain matching topic strings.

**File**: `include/mqtt_manager.h` (new — replaces network manager's MQTT role)
```cpp
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Callback type for incoming MQTT messages
typedef void (*MqttMessageHandler)(const char* topic, const char* payload);

struct MqttSubscription {
  const char* topic;
  MqttMessageHandler handler;
};

class MqttManager {
public:
  static const int MAX_SUBSCRIPTIONS = 10;

  MqttManager();

  bool init(const char* endpoint, int port,
            const char* ca, const char* cert, const char* key,
            const char* clientId);
  void update();

  // Subscribe to a topic with a handler — no hardcoded switch needed
  bool subscribe(const char* topic, MqttMessageHandler handler);

  bool publish(const char* topic, const char* payload);
  bool isConnected() const;

  // WiFi
  bool connectWiFi(const char* ssid, const char* password,
                   unsigned long timeoutMs);
  bool isWiFiConnected() const;
  const char* getConnectedSSID() const { return connectedSSID_; }
  int getRSSI() const;

private:
  WiFiClientSecure wifiClient_;
  PubSubClient mqttClient_;
  unsigned long lastReconnect_;
  char connectedSSID_[33];

  MqttSubscription subs_[MAX_SUBSCRIPTIONS];
  int subCount_;

  static void staticCallback(char* topic, byte* payload, unsigned int len);
  void dispatchMessage(const char* topic, const char* payload);
  bool connectMqtt(const char* clientId);
  void setupTime();
};

extern MqttManager mqttManager;

#endif
```

**Key change**: `subscribe()` stores `{topic, handler}` pairs. When a message arrives, `dispatchMessage()` iterates the registry instead of a switch:
```cpp
void MqttManager::dispatchMessage(const char* topic, const char* payload) {
  for (int i = 0; i < subCount_; i++) {
    if (strcmp(subs_[i].topic, topic) == 0) {
      subs_[i].handler(topic, payload);
      return;
    }
  }
  Serial.printf("Unhandled topic: %s\n", topic);
}
```

### 3.4 — Slim Down NetworkManager

After extraction, `NetworkManager` becomes a thin coordinator:
```cpp
class NetworkManager {
public:
  bool init();
  void update();

  MqttManager& mqtt() { return mqttManager; }
  NotificationQueue& notifications() { return notifications_; }
  GitHubDataStore& github() { return githubData_; }

  // Workspace mode
  bool isInWorkspaceMode() const;
  unsigned long getLastMQTTMessageTime() const;

  // Serial logging (keep here — it's MQTT-publishing logic)
  void log(const char* msg);
  void logInfo(const char* msg);
  void logWarn(const char* msg);
  void logError(const char* msg);

private:
  NotificationQueue notifications_;
  GitHubDataStore githubData_;
  bool workspaceMode_;
  unsigned long lastMQTTMessageTime_;
  // ... log buffer
};
```

### 3.5 — Register Topic Handlers in main.cpp

```cpp
void setup() {
  // ...
  networkManager.init();

  // Register MQTT topic handlers (no switch statement needed)
  mqttManager.subscribe(MQTT_TOPIC_EMOTION_SET, handleEmotionSet);
  mqttManager.subscribe("sangi/notification/push", handleNotificationPush);
  mqttManager.subscribe(MQTT_TOPIC_GITHUB_COMMITS, handleGitHubCommits);
  mqttManager.subscribe("sangi/github/stats", handleGitHubStats);
}

// Each handler is a focused function
void handleEmotionSet(const char* topic, const char* payload) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return;
  // SSID validation, emotion parsing...
}
```

### 3.6 — Decouple input.cpp

**File**: `include/input.h`

Add a callback so InputManager doesn't need to know about EmotionManager:
```cpp
typedef void (*TouchHandlerFn)(unsigned long currentTime);

class InputManager {
public:
  void init();
  bool isTouched();
  void handleTouchInteraction();
  void setOnTouch(TouchHandlerFn fn) { onTouch_ = fn; }
private:
  unsigned long lastInteraction_;
  TouchHandlerFn onTouch_;
};
```

**File**: `src/main.cpp`
```cpp
void onTouch(unsigned long currentTime) {
  int r = random(0, 100);
  emotionManager.setTargetEmotion(r < 50 ? EMOTION_EXCITED : EMOTION_SURPRISED);
}

void setup() {
  inputManager.init();
  inputManager.setOnTouch(onTouch);
}
```

### Files Changed (Phase 3)
| File | Action |
|------|--------|
| `include/notification_queue.h` | **New** |
| `src/notification_queue.cpp` | **New** |
| `include/github_data.h` | **New** |
| `src/github_data.cpp` | **New** |
| `include/mqtt_manager.h` | **New** |
| `src/mqtt_manager.cpp` | **New** |
| `include/network.h` | Rewrite: slim coordinator |
| `src/network.cpp` | Rewrite: ~400 lines removed |
| `include/input.h` | Add callback |
| `src/input.cpp` | Use callback instead of emotionManager global |
| `src/main.cpp` | Register topic handlers, wire input callback |

---

## Phase 4: main.cpp Final Cleanup

With Phases 2 and 3 complete, main.cpp gets its final polish.

### 4.1 — Remove `loadHardcodedGitHubData()`

Move the test data into a `#if DEBUG_MODE_ENABLED` block or a dedicated `test_data.cpp`. It's 90 lines of sample contribution grids that shouldn't be in production firmware.

### 4.2 — Unify Offline/Workspace Mode Logic

Extract the deeply nested `#if ENABLE_MQTT` / offline-mode / workspace-mode logic into a dedicated function:

```cpp
// Returns the emotion to set, or -1 if MQTT is controlling
EmotionState selectAutonomousEmotion(unsigned long currentTime) {
  // All the offline detection, random picking, notification generation
  // lives here instead of inline in loop()
}
```

### 4.3 — Error Emotion on Init Failure

Currently, display init failure halts forever:
```cpp
if (!displayManager.init()) {
  for(;;);
}
```

Instead, set a visible error state:
```cpp
if (!displayManager.init()) {
  Serial.println("FATAL: Display init failed");
  // Could blink onboard LED, or if display partially works:
  // show error emotion
  for(;;) { delay(1000); }
}
```

### 4.4 — const Correctness Sweep

Apply `const` throughout:
- `EmotionDef` members that don't change after registration
- `const char*` parameters on all string-receiving functions
- `const` on getter methods that aren't already const
- `const EmotionState&` where passing by value is wasteful (not really — enums are int-sized, skip this)

### Files Changed (Phase 4)
| File | Action |
|------|--------|
| `src/main.cpp` | Extract selectAutonomousEmotion(), remove test data |
| `src/test_data.cpp` | **New** (optional): hardcoded GitHub data |
| Various headers | Add `const` qualifiers |

---

## Phase 5: Testing

### 5.1 — MockCanvas

**File**: `test/mock_canvas.h` (new)
```cpp
#ifndef MOCK_CANVAS_H
#define MOCK_CANVAS_H

#include "canvas.h"
#include <stdio.h>
#include <string.h>

// Records drawing calls for assertion in tests.
// Can also dump to stdout for visual debugging.
struct DrawCall {
  enum Type { FILL_RRECT, FILL_CIRCLE, DRAW_LINE, TEXT, CLEAR, FLUSH, OTHER };
  Type type;
  int16_t x, y, w, h, r;
  uint16_t color;
  char text[32];
};

class MockCanvas : public ICanvas {
public:
  static const int MAX_CALLS = 200;

  MockCanvas() : callCount_(0), verbose_(false) { reset(); }

  void setVerbose(bool v) { verbose_ = v; }
  void reset() { callCount_ = 0; memset(calls_, 0, sizeof(calls_)); }

  int callCount() const { return callCount_; }
  const DrawCall& call(int i) const { return calls_[i]; }

  // Find first call of a given type
  int findCall(DrawCall::Type type, int startFrom = 0) const {
    for (int i = startFrom; i < callCount_; i++) {
      if (calls_[i].type == type) return i;
    }
    return -1;
  }

  // --- ICanvas implementation ---
  void clear() override { record(DrawCall::CLEAR); }
  void flush() override { record(DrawCall::FLUSH); }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     int16_t r, uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      calls_[callCount_] = {DrawCall::FILL_RRECT, x, y, w, h, r, color, ""};
      callCount_++;
    }
    if (verbose_) printf("fillRoundRect(%d,%d,%d,%d,r=%d)\n", x, y, w, h, r);
  }

  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      calls_[callCount_] = {DrawCall::FILL_CIRCLE, x, y, 0, 0, r, color, ""};
      callCount_++;
    }
    if (verbose_) printf("fillCircle(%d,%d,r=%d)\n", x, y, r);
  }

  // Remaining primitives: minimal recording for test assertions
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     int16_t r, uint16_t color) override { record(DrawCall::OTHER); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t c) override { record(DrawCall::OTHER); }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t c) override { record(DrawCall::OTHER); }
  void drawCircle(int16_t x, int16_t y, int16_t r,
                  uint16_t c) override { record(DrawCall::OTHER); }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                uint16_t c) override { record(DrawCall::OTHER); }
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t c) override {
    record(DrawCall::OTHER);
  }

  void setTextSize(uint8_t s) override {}
  void setCursor(int16_t x, int16_t y) override {}
  void setTextColor(uint16_t c) override {}
  void print(const char* t) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {DrawCall::TEXT, 0, 0, 0, 0, 0, 0, ""};
      strncpy(dc.text, t, 31);
      calls_[callCount_++] = dc;
    }
  }
  void println(const char* t) override { print(t); }

private:
  DrawCall calls_[MAX_CALLS];
  int callCount_;
  bool verbose_;

  void record(DrawCall::Type type) {
    if (callCount_ < MAX_CALLS) {
      calls_[callCount_] = {type, 0, 0, 0, 0, 0, 0, ""};
      callCount_++;
    }
  }
};

#endif
```

### 5.2 — Emotion Transition Tests

**File**: `test/test_emotion_transitions.cpp` (new)

Uses PlatformIO's Unity test framework:
```cpp
#include <unity.h>
#include "emotion.h"
#include "emotion_registry.h"

// --- Test helpers ---
static EmotionState lastCompletedEmotion = EMOTION_IDLE;
static EmotionState lastChangeFrom = EMOTION_IDLE;
static EmotionState lastChangeTo = EMOTION_IDLE;

void stubTransitionComplete(EmotionState e) { lastCompletedEmotion = e; }
void stubEmotionChange(EmotionState f, EmotionState t) {
  lastChangeFrom = f; lastChangeTo = t;
}

void setUp() {
  emotionManager.init(0);
  emotionManager.setOnTransitionComplete(stubTransitionComplete);
  emotionManager.setOnEmotionChange(stubEmotionChange);
  lastCompletedEmotion = EMOTION_IDLE;
}

void tearDown() {}

// --- Tests ---

void test_initial_state_is_idle() {
  TEST_ASSERT_EQUAL(EMOTION_IDLE, emotionManager.getCurrentEmotion());
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
}

void test_set_target_starts_transition() {
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  TEST_ASSERT_TRUE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(0, emotionManager.getTransitionFrame());
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, emotionManager.getTargetEmotion());
  // Current hasn't changed yet — transition in progress
  TEST_ASSERT_EQUAL(EMOTION_IDLE, emotionManager.getCurrentEmotion());
}

void test_advance_increments_frame() {
  emotionManager.setTargetEmotion(EMOTION_SAD);
  emotionManager.advanceTransition();
  TEST_ASSERT_EQUAL(1, emotionManager.getTransitionFrame());
  emotionManager.advanceTransition();
  TEST_ASSERT_EQUAL(2, emotionManager.getTransitionFrame());
}

void test_complete_sets_current_and_fires_callback() {
  emotionManager.setTargetEmotion(EMOTION_EXCITED);
  emotionManager.completeTransition();
  TEST_ASSERT_EQUAL(EMOTION_EXCITED, emotionManager.getCurrentEmotion());
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(EMOTION_EXCITED, lastCompletedEmotion);
}

void test_same_emotion_does_not_transition() {
  emotionManager.setTargetEmotion(EMOTION_IDLE);
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
}

void test_invalid_emotion_rejected() {
  emotionManager.setTargetEmotion((EmotionState)99);
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(EMOTION_IDLE, emotionManager.getCurrentEmotion());
}

void test_change_callback_receives_from_and_to() {
  emotionManager.setTargetEmotion(EMOTION_LOVE);
  TEST_ASSERT_EQUAL(EMOTION_IDLE, lastChangeFrom);
  TEST_ASSERT_EQUAL(EMOTION_LOVE, lastChangeTo);
}

void test_previous_emotion_tracks_history() {
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  emotionManager.completeTransition();
  emotionManager.setTargetEmotion(EMOTION_SAD);
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, emotionManager.getPreviousEmotion());
}

// --- Registry tests ---

void test_registry_add_and_get() {
  EmotionRegistry reg;
  EmotionDef def = {EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr};
  TEST_ASSERT_TRUE(reg.add(def));
  const EmotionDef* got = reg.get(EMOTION_IDLE);
  TEST_ASSERT_NOT_NULL(got);
  TEST_ASSERT_EQUAL_STRING("IDLE", got->name);
}

void test_registry_rejects_duplicate() {
  EmotionRegistry reg;
  EmotionDef def = {EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr};
  reg.add(def);
  TEST_ASSERT_FALSE(reg.add(def));
}

void test_registry_get_name_unknown() {
  EmotionRegistry reg;
  TEST_ASSERT_EQUAL_STRING("UNKNOWN", reg.getName(EMOTION_HAPPY));
}

void test_registry_cyclable_excludes_blink() {
  EmotionRegistry reg;
  reg.add({EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr});
  reg.add({EMOTION_BLINK, "BLINK", 1, 0, LOOP_RESTART, false, nullptr});
  reg.add({EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, nullptr});

  EmotionState out[10];
  int n = reg.getCyclable(out, 10);
  TEST_ASSERT_EQUAL(2, n);
  TEST_ASSERT_EQUAL(EMOTION_IDLE, out[0]);
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, out[1]);
}

// --- Runner ---
int main(int argc, char** argv) {
  UNITY_BEGIN();
  RUN_TEST(test_initial_state_is_idle);
  RUN_TEST(test_set_target_starts_transition);
  RUN_TEST(test_advance_increments_frame);
  RUN_TEST(test_complete_sets_current_and_fires_callback);
  RUN_TEST(test_same_emotion_does_not_transition);
  RUN_TEST(test_invalid_emotion_rejected);
  RUN_TEST(test_change_callback_receives_from_and_to);
  RUN_TEST(test_previous_emotion_tracks_history);
  RUN_TEST(test_registry_add_and_get);
  RUN_TEST(test_registry_rejects_duplicate);
  RUN_TEST(test_registry_get_name_unknown);
  RUN_TEST(test_registry_cyclable_excludes_blink);
  return UNITY_END();
}
```

### 5.3 — PlatformIO Test Config

**File**: `platformio.ini` — add native test environment:
```ini
[env:native]
platform = native
build_flags = -std=c++14
test_build_src = true
test_framework = unity
build_src_filter =
  +<emotion.cpp>
  +<emotion_registry.cpp>
lib_ignore =
  Adafruit SSD1306
  Adafruit GFX Library
  PubSubClient
  ArduinoJson
```

Note: native tests require a minimal `Arduino.h` stub for `Serial.printf` etc. A lightweight approach:

**File**: `test/arduino_stub.h` (new)
```cpp
// Minimal Arduino API stub for native testing
#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PROGMEM

struct SerialStub {
  void begin(int) {}
  void print(const char* s) { printf("%s", s); }
  void println(const char* s = "") { printf("%s\n", s); }
  void println(int v) { printf("%d\n", v); }
  void print(int v) { printf("%d", v); }
  void printf(const char* fmt, ...) {
    va_list args; va_start(args, fmt);
    vprintf(fmt, args); va_end(args);
  }
  void flush() { fflush(stdout); }
};
extern SerialStub Serial;

inline unsigned long millis() { return 0; }

#endif
```

### 5.4 — Animation Draw Function Tests

After Phase 2, individual emotions can be tested:
```cpp
void test_sleepy_frame0_draws_eyes() {
  MockCanvas canvas;
  drawSleepy(canvas, 0, nullptr);
  // Frame 0 should draw two eyes (fillRoundRect calls) at expected positions
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT(idx >= 0);
  // Left eye at x=30 (40-10), y=18 (28-20/2), w=20, h=20
  TEST_ASSERT_EQUAL(30, canvas.call(idx).x);
}
```

### Files Changed (Phase 5)
| File | Action |
|------|--------|
| `test/mock_canvas.h` | **New** |
| `test/test_emotion_transitions.cpp` | **New** |
| `test/arduino_stub.h` | **New** |
| `platformio.ini` | Add `[env:native]` test target |

---

## Phase 6: Pi Notification Service Restructure

**Goal**: Convert `pi-setup/` from a loose script collection into a proper Python package with clean separation of concerns.

### 6.1 — Package Structure

```
pi-setup/
├── pyproject.toml                 # NEW: package metadata + dependencies
├── sangi_notify/                  # NEW: proper package directory
│   ├── __init__.py               # Exports NotificationService
│   ├── __main__.py               # Entry point: python -m sangi_notify
│   ├── config.py                 # NEW: config loading + validation
│   ├── service.py                # Renamed from notification_service.py
│   ├── mqtt/                     # NEW: MQTT subpackage
│   │   ├── __init__.py
│   │   ├── client.py             # Renamed from mqtt_publisher.py
│   │   └── topics.py             # NEW: topic constants
│   ├── monitors/                 # NEW: monitor subpackage
│   │   ├── __init__.py
│   │   ├── base.py               # NEW: abstract Monitor base class
│   │   ├── github_notifications.py  # Renamed from github_monitor.py
│   │   ├── github_stats.py       # Moved from lib/
│   │   ├── discord.py            # Renamed from discord_monitor.py
│   │   ├── dbus_notify.py        # Renamed from notification_monitor.py
│   │   └── random_trigger.py     # Renamed from random_stats_trigger.py
│   └── util/
│       ├── __init__.py
│       └── logging.py            # NEW: shared logging setup
├── tests/
│   ├── test_github_stats.py      # Moved
│   ├── test_service.py           # Moved
│   └── conftest.py               # NEW: pytest fixtures
├── config.json.template
├── certs/
├── setup.sh
└── README.md
```

### 6.2 — pyproject.toml

```toml
[build-system]
requires = ["setuptools>=68.0"]
build-backend = "setuptools.backends._legacy:_Backend"

[project]
name = "sangi-notify"
version = "1.0.0"
description = "SANGI robot notification service"
requires-python = ">=3.9"
dependencies = [
    "awscrt>=0.16.0",
    "awsiotsdk>=1.11.0",
    "requests>=2.28.0",
]

[project.optional-dependencies]
discord = ["discord.py-self>=1.9.2"]
dbus = []  # System packages: python3-gi, python3-dbus

[project.scripts]
sangi-notify = "sangi_notify.__main__:main"
```

### 6.3 — Abstract Monitor Base Class

**File**: `sangi_notify/monitors/base.py`
```python
from abc import ABC, abstractmethod
import logging
import threading


class Monitor(ABC):
    """Base class for all notification monitors."""

    def __init__(self, config: dict, name: str):
        self.config = config
        self.logger = logging.getLogger(name)
        self._stop_event = threading.Event()

    @abstractmethod
    def poll(self) -> None:
        """Execute one polling cycle."""
        ...

    @property
    @abstractmethod
    def interval(self) -> float:
        """Seconds between poll() calls."""
        ...

    def start(self) -> None:
        """Blocking polling loop. Call from a daemon thread."""
        self.logger.info(f"{self.__class__.__name__} started")
        while not self._stop_event.is_set():
            try:
                self.poll()
            except Exception:
                self.logger.exception("Error in poll cycle")
            self._stop_event.wait(self.interval)

    def stop(self) -> None:
        self._stop_event.set()
```

All monitors inherit from `Monitor`, implementing `poll()` and `interval`. The `DiscordMonitor` overrides `start()` to use `asyncio.run()` instead of the threading loop.

### 6.4 — Config Validation

**File**: `sangi_notify/config.py`
```python
import json
import sys
from pathlib import Path

REQUIRED_MQTT_KEYS = ["endpoint", "certificate_path", "private_key_path",
                      "root_ca_path"]

def load_config(path: str = "config.json") -> dict:
    p = Path(path)
    if not p.exists():
        print(f"ERROR: {path} not found. Copy config.json.template")
        sys.exit(1)

    with open(p) as f:
        config = json.load(f)

    # Validate MQTT section
    mqtt = config.get("mqtt", {})
    missing = [k for k in REQUIRED_MQTT_KEYS if k not in mqtt]
    if missing:
        print(f"ERROR: Missing MQTT config keys: {missing}")
        sys.exit(1)

    # Validate cert files exist
    for key in ["certificate_path", "private_key_path", "root_ca_path"]:
        cert_path = Path(mqtt[key])
        if not cert_path.exists():
            print(f"ERROR: Certificate not found: {cert_path}")
            sys.exit(1)

    return config
```

### 6.5 — Remove sys.path hack

Current `notification_service.py`:
```python
sys.path.insert(0, str(Path(__file__).parent / 'lib'))
```

With proper package structure, this is unnecessary. Imports become:
```python
from sangi_notify.mqtt.client import MQTTClient
from sangi_notify.monitors.github_notifications import GitHubNotificationMonitor
```

### 6.6 — Topic Constants

**File**: `sangi_notify/mqtt/topics.py`
```python
# MQTT topics — single source of truth shared with firmware config.h
EMOTION_SET = "sangi/emotion/set"
NOTIFICATION_PUSH = "sangi/notification/push"
GITHUB_COMMITS = "sangi/github/commits"
GITHUB_STATS = "sangi/github/stats"
DISCORD_STATS = "sangi/discord/stats"
STATUS = "sangi/status"
```

### 6.7 — GitHub API Client Deduplication

Both `github_monitor.py` and `github_stats.py` construct similar `requests.get()` calls with auth headers. Extract:

**File**: `sangi_notify/monitors/github_api.py` (new)
```python
import requests
import logging

class GitHubAPI:
    BASE_URL = "https://api.github.com"
    GRAPHQL_URL = "https://api.github.com/graphql"

    def __init__(self, token: str):
        self.session = requests.Session()
        self.session.headers.update({
            "Authorization": f"token {token}",
            "Accept": "application/vnd.github.v3+json",
        })
        self.logger = logging.getLogger("GitHubAPI")

    def get(self, path: str, **kwargs) -> requests.Response:
        url = f"{self.BASE_URL}{path}"
        resp = self.session.get(url, timeout=30, **kwargs)
        resp.raise_for_status()
        return resp

    def graphql(self, query: str, variables: dict = None) -> dict:
        payload = {"query": query}
        if variables:
            payload["variables"] = variables
        resp = self.session.post(self.GRAPHQL_URL, json=payload, timeout=30)
        resp.raise_for_status()
        return resp.json()
```

Both `GitHubNotificationMonitor` and `GitHubStatsMonitor` receive a shared `GitHubAPI` instance.

### Files Changed (Phase 6)
| File | Action |
|------|--------|
| `pi-setup/sangi_notify/` | **New** package directory |
| `pi-setup/pyproject.toml` | **New** |
| `pi-setup/sangi_notify/__init__.py` | **New** |
| `pi-setup/sangi_notify/__main__.py` | **New** |
| `pi-setup/sangi_notify/config.py` | **New** |
| `pi-setup/sangi_notify/service.py` | Migrated from `notification_service.py` |
| `pi-setup/sangi_notify/mqtt/client.py` | Migrated from `lib/mqtt_publisher.py` |
| `pi-setup/sangi_notify/mqtt/topics.py` | **New** |
| `pi-setup/sangi_notify/monitors/base.py` | **New** |
| `pi-setup/sangi_notify/monitors/github_api.py` | **New** |
| `pi-setup/sangi_notify/monitors/github_notifications.py` | Migrated |
| `pi-setup/sangi_notify/monitors/github_stats.py` | Migrated |
| `pi-setup/sangi_notify/monitors/discord.py` | Migrated |
| `pi-setup/sangi_notify/monitors/dbus_notify.py` | Migrated |
| `pi-setup/sangi_notify/monitors/random_trigger.py` | Migrated |
| `pi-setup/lib/` | **Delete** (replaced by package) |
| `pi-setup/notification_service.py` | **Delete** (replaced by service.py) |

---

## Phase 7: Documentation — NEW_EMOTION_HOWTO.md

After all phases are complete, adding a 17th emotion requires:

**File**: `NEW_EMOTION_HOWTO.md` (new)

```markdown
# How to Add a New Emotion to SANGI

After the Phase 2 refactoring, adding a new emotion requires changes
in exactly **2 files** and **under 10 lines of setup code** (plus your
animation art).

## Step 1: Add the enum value

**File: `include/emotion.h`**

Add your emotion before `EMOTION_GITHUB_STATS`:

```cpp
  EMOTION_CODING,
  EMOTION_GITHUB_STATS,
  EMOTION_DIZZY,          // <-- add here
```

## Step 2: Write the draw function

**File: `src/emotion_draws.cpp`**

```cpp
void drawDizzy(ICanvas& canvas, int frame, const void* ctx) {
  // Spiral eyes + wobbling
  int wobble = (frame % 10) - 5;
  canvas.drawEyes(40 + wobble, 28, 88 - wobble, 28, 18);
  canvas.drawCircle(40 + wobble, 28, 6, COLOR_WHITE);
  canvas.drawCircle(88 - wobble, 28, 6, COLOR_WHITE);
  canvas.drawMouth(52, 50, 24, 5);
}
```

## Step 3: Register it

**File: `src/main.cpp`** — in `registerEmotions()`:

```cpp
emotionRegistry.add({EMOTION_DIZZY, "DIZZY", 51, 30, LOOP_RESTART, true, drawDizzy});
```

## Step 4 (optional): Add a beep pattern

**File: `src/speaker.cpp`** — add to the lookup table:

```cpp
static const BeepTone PATTERN_DIZZY[] = {{800,80},{600,80},{400,80}};
// ...
{EMOTION_DIZZY, PATTERN_DIZZY, 3},
```

## That's it.

No switch statements to update. No frame variables to declare.
No display.cpp changes. No animations.h changes.
The registry, tick engine, and ICanvas handle everything.
```

---

## Summary: Lines of Code Impact

| Phase | Files Added | Files Modified | Estimated LOC Removed | Estimated LOC Added | Net |
|-------|-------------|----------------|----------------------|--------------------|----|
| 1 (done) | 3 | 5 | ~50 | ~250 | +200 |
| 2 | 2 | 5 | ~2,900 | ~2,100 | -800 |
| 3 | 6 | 4 | ~600 | ~500 | -100 |
| 4 | 0-1 | 3 | ~120 | ~30 | -90 |
| 5 | 3 | 1 | 0 | ~300 | +300 |
| 6 | 12 | 0 (delete 8) | ~900 | ~600 | -300 |
| 7 | 1 | 0 | 0 | ~50 | +50 |
| **Total** | **27** | **18** | **~4,570** | **~3,830** | **-740** |

Net reduction of ~740 lines while adding an ICanvas abstraction, test harness, subscription registry, proper Python packaging, and a data-driven emotion system that makes adding emotion #17 a 10-line task.
