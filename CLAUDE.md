# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

**Scope:** Developer-facing documentation for code architecture, testing, and development workflow.
**See also:** `README.md` (user/feature documentation), `include/config.h` (hardware pins & timing constants)

## v1 Status

✅ **Modules 1–6 complete.** Standalone ESP32-C3 robot with personality engine, gesture detection, and BLE control. All 64 tests passing.

**Next:** Module 7 (Captive Portal WiFi) not yet started.

---

## Build & Test Commands

### Hardware Build (ESP32-C3)
```bash
# Build firmware for hardware
~/.platformio/penv/bin/pio run

# Upload to device
~/.platformio/penv/bin/pio run --target upload

# Monitor serial output (adjust port as needed)
~/.platformio/penv/bin/pio device monitor --port /dev/ttyUSB1 --baud 115200
```

**What runs:** Full firmware including display, BLE, input, speaker, battery ADC. Uses real Adafruit and NimBLE libraries.

### Native Tests (Host Machine)
```bash
# Run all 64 native unit tests (no hardware required)
~/.platformio/penv/bin/pio test -e native

# Run a single test file
~/.platformio/penv/bin/pio test -e native -f test_sangi

# Verbose output with test names
~/.platformio/penv/bin/pio test -e native -vvv
```

**What runs:** Core logic (emotion, registry, animation, gesture, personality) against stubbed hardware APIs and MockCanvas. No ESP32-C3, no real OLED, no BLE stack needed.

---

## Architecture

**Single-device standalone design.** No WiFi, no cloud, no external dependencies.

### Layers

1. **ESP32-C3 firmware** (`src/`, `include/`)
   - SSD1306 OLED display driver (I2C)
   - 14 animated emotions with personality engine
   - Touch gesture recognition (TAP, LONG_PRESS, DOUBLE_TAP)
   - BLE server for remote emotion control
   - Autonomous mood cycling with time-of-day weighting
   - Battery monitoring via ADC

2. **Hardware only**
   - No Raspberry Pi service
   - No cloud or MQTT
   - No web dashboard
   - Fully offline, fully autonomous

### Core Modules

| Module | Status | Purpose |
|--------|--------|---------|
| **1. Strip Networking** | ✅ | Removed MQTT/AWS IoT, made fully autonomous |
| **2. Remove Cut Emotions** | ✅ | Eliminated MUSIC, NOTIFICATION, CODING, GITHUB_STATS; kept 14 core emotions |
| **3. Add Personality** | ✅ | Added BORED + SHY emotions; personality engine with attention arc |
| **4. BLE Control** | ✅ | NimBLE-Arduino server with emotion write/read characteristic |
| **5. Gesture Detection** | ✅ | State machine: TAP (<600ms), LONG_PRESS (≥600ms), DOUBLE_TAP (within 300ms) |
| **6. Personality Engine** | ✅ | Attention arc (5/10/12.5/15 min stages), mood drift, micro-expressions, jitter ±20% |

---

## Key Patterns

### ICanvas Interface
`include/canvas.h` — Abstract display interface.
- `DisplayManager` implements for real hardware (SSD1306)
- `MockCanvas` (`test/mock_canvas.h`) records draw calls for unit testing
- **Rule:** All emotion rendering goes through ICanvas; never call OLED library directly

### EmotionRegistry Pattern
`include/emotion_registry.h` — Data-driven emotion definitions.
- Each emotion registered via `emotionRegistry.add({enum, name, frameCount, frameDelay, loopMode, blinkable, drawFn})`
- Eliminates switch statements
- **Global singleton:** `emotionRegistry` is declared `extern` in the header, defined in `emotion_registry.cpp`
- **Initialization:** Called in `registerEmotions()` in `main.cpp` during setup; all emotions must be registered before first use
- **Adding a new emotion:** Enum in `emotion.h`, draw function in `emotion_draws.cpp`, one `registry.add()` call in `main.cpp`

### EmotionManager State Machine
`include/emotion.h` — Transitions between emotions via 7-frame blink animation.
- 3 frames closing eyes + 3 frames opening + 1 frame of target emotion
- Callbacks `onTransitionComplete()` and `onEmotionChange()` injected by `main.cpp`
- Eliminates circular dependencies between display, audio, and state modules

### Callback Injection Pattern
All modules communicate via callbacks, not global references:
- `EmotionManager` knows nothing about beeps, animations, or BLE
- `main.cpp` wires callbacks: `setOnEmotionChange()`, `setOnTransitionComplete()`, `setOnGesture()`
- Result: **Zero circular dependencies**, easy to test in isolation

### Module Responsibilities

| Module | Responsibility | Tests Included |
|--------|-----------------|-----------------|
| **emotion** | State machine, 7-frame blink transition | ✅ Transition timing, frame caching |
| **emotion_registry** | Emotion lookup, metadata storage | ✅ Registry add/lookup |
| **emotion_draws** | 14 emotion animations (51 frames each) | ✅ Frame advance, loop behavior |
| **animations** | Generic frame ticker, frame caching | ✅ Animation timing, frame limits |
| **input** | Gesture detection state machine | ✅ TAP/LONG_PRESS/DOUBLE_TAP timing |
| **personality** | Attention arc, mood drift, micro-expressions | ✅ Stage transitions, jitter, recovery |
| **display** | OLED rendering via ICanvas | ✅ MockCanvas draw call recording |
| **speaker** | PWM beep patterns | ✅ Beep generation (stubbed in tests) |
| **ble_control** | NimBLE server, emotion control | ✅ Emotion validation (call stubs) |
| **battery** | ADC voltage reading | ✅ Voltage range validation |

All core modules tested; hardware-specific I/O (GPIO, I2C, BLE stack) stubbed in native tests.

### Personality Engine
`include/personality.h` — Three independent subsystems:
1. **Attention Arc** — Time-based emotional degradation (5–15 min neglect)
   - Stages: BORED (5 min) → SAD (10 min) → CONFUSED (12.5 min) → ANGRY (15 min)
   - Resets on touch interaction (enters SHY recovery state)
2. **Mood Drift** — Hour-of-day weighted random emotions every ~2 min
   - Emits random emotion with weights biased by time-of-day (no external time, uses `millis()` to estimate)
   - Each drift is independent; doesn't interrupt attention arc
3. **Micro-expressions** — 15% chance of random BLINK per drift
   - Adds subtle personality: random blink on ~every 7th mood drift event

**Jitter System:** All timing intervals ±20% variance applied independently:
```cpp
#define JITTER_PERCENT 20  // Each timer gets ±20% random offset
```
Examples:
- 300,000ms (5 min) → 240,000ms to 360,000ms range
- 120,000ms (2 min) → 96,000ms to 144,000ms range

Prevents predictable loops and makes behavior feel more organic/lifelike.

---

## Hardware Configuration

All pin assignments and timing constants live in `include/config.h`.

### Fixed I2C Pins (ESP32-C3)
- **GPIO 6** = SDA (OLED display)
- **GPIO 7** = SCL (OLED display)
- I2C address: 0x3C (standard for SSD1306)

### Other Pins
- **GPIO 3** = Touch sensor (capacitive button)
- **GPIO 10** = Speaker (PWM beeper) — *GPIO 9 conflicts with USB*
- **GPIO 2** = Battery ADC (voltage monitoring)

### Timing Constants (Customizable in config.h)
```cpp
#define LONG_PRESS_MS 600                 // Threshold for LONG_PRESS gesture
#define DOUBLE_TAP_WINDOW_MS 300          // Window for DOUBLE_TAP detection
#define ATTENTION_STAGE1_MS 300000        // 5 min → BORED
#define ATTENTION_STAGE2_MS 600000        // 10 min → SAD
#define ATTENTION_STAGE3_MS 750000        // 12.5 min → CONFUSED
#define ATTENTION_STAGE4_MS 900000        // 15 min → ANGRY
#define MOOD_DRIFT_INTERVAL_MS 120000     // ~2 min between mood changes
#define MICRO_EXPRESSION_CHANCE 15        // % chance for random BLINK
#define JITTER_PERCENT 20                 // ±20% variance on all timings
```

---

## Native Testing

Tests in `test/test_sangi.cpp` use Unity framework. Arduino API is stubbed in `test/arduino_stub/Arduino.h`:

- `millis()` — Shared across translation units via inline+static-local pattern
- `Serial` — printf-compatible, output captured by test framework
- `GPIO` — No-ops (no actual hardware)
- `String` — Basic Arduino String implementation
- `Adafruit_GFX`, `Adafruit_SSD1306` — Stubs return void

### Build Filter (Native vs. Hardware)

The `[env:native]` section in `platformio.ini` specifies `build_src_filter`:
```ini
build_src_filter =
    +<emotion.cpp>
    +<emotion_registry.cpp>
    +<animations.cpp>
    +<emotion_draws.cpp>
    +<input.cpp>
    +<personality.cpp>
```

**Included modules** — Core logic, no hardware dependencies:
- Emotion state machine, registry, animation tick, gesture detection, personality engine

**Excluded modules** — Hardware-specific, stubbed instead:
- `display.cpp` — Uses Adafruit OLED library (stubbed)
- `speaker.cpp` — Uses GPIO PWM (stubbed)
- `ble_control.cpp` — Uses NimBLE (not compiled for tests)
- `battery.cpp` — Uses ADC (stubbed in tests)

This keeps native tests fast and hardware-agnostic.

### Test Coverage
- Emotion state transitions and registry lookups
- Animation frame timing and frame caching
- Display rendering via MockCanvas
- Gesture classification state machine (TAP/LONG_PRESS/DOUBLE_TAP with timing)
- Personality engine (attention arc, mood drift, recovery)
- BLE emotion validation (call stubs, not real BLE)
- Battery monitoring

**64/64 tests passing, zero warnings.**

---

## Writing Tests

Tests compile and run on the host machine, not on ESP32-C3. Use `test/test_sangi.cpp` as reference.

### Mock Time in Tests
```cpp
// In test/arduino_stub/Arduino.h, millis() uses a static counter:
extern void resetMillis();       // Reset to 0 before each test
extern void advanceMillis(unsigned long ms);  // Step time forward
```

Example test:
```cpp
void test_attention_arc_triggers_at_5_minutes() {
  resetMillis();
  personality.tick();  // Start at t=0

  advanceMillis(300001);  // Jump to t=5:00:01
  personality.tick();     // Check emotion state

  TEST_ASSERT_EQUAL(EMOTION_BORED, emotionManager.currentEmotion());
}
```

### Display Testing with MockCanvas
Instead of real SSD1306, tests use `MockCanvas` (`test/mock_canvas.h`):
```cpp
MockCanvas canvas;
emotionManager.tick(canvas);  // Pass mock, not DisplayManager
// canvas.calls records all draw operations for assertions
```

### Hardware-Specific Code in Tests
- GPIO operations → Stubbed to no-ops
- Serial output → Captured by Unity framework (seen in test output)
- BLE calls → Stubbed; NimBLE not compiled in native environment

---

## Critical Rules

### Singletons & Globals
- All manager instances (`emotionManager`, `displayManager`, `inputManager`, etc.) are `extern` singletons
- Never instantiate manager classes locally; always use the global instance
- Defined in their respective `.cpp` files; declared `extern` in headers

### Display & I2C
- **Never call `display.begin()` twice** — causes I2C hang. Init happens once in `DisplayManager::init()`
- All drawing goes through `ICanvas` interface, never raw Adafruit library calls
- `MockCanvas` used in tests to verify draw logic without hardware

### Animation State
- Draw functions use `static` local variables to persist state across `tick()` calls
- Example: `drawHappy()` uses `static int frame = 0;` to track animation progress
- Each call to `tick()` advances the frame; loop behavior controlled by registry

### Compile Flags
- `DEBUG_MODE_ENABLED` — If 1, displays fixed `DEBUG_MODE_EMOTION` and skips personality engine
- `ENABLE_EMOTION_BEEP` — If 1, plays beeps when emotions change
- Both default to 0 for normal operation; native tests disable both

---

## Adding a New Emotion

1. **Add enum** in `include/emotion.h`:
   ```cpp
   enum EmotionState {
     EMOTION_IDLE,
     EMOTION_HAPPY,
     // ... existing emotions ...
     EMOTION_NEW_EMOTION,  // ← Add here
     EMOTION_COUNT
   };
   ```

2. **Draw function** in `src/emotion_draws.cpp`:
   ```cpp
   void drawNewEmotion(ICanvas& canvas) {
     static int frame = 0;
     // Draw logic using canvas methods (fillCircle, drawLine, etc.)
     frame = (frame + 1) % FRAME_COUNT;
   }
   ```

3. **Declare** in `include/emotion_draws.h`:
   ```cpp
   void drawNewEmotion(ICanvas& canvas);
   ```

4. **Register** in `src/main.cpp` `registerEmotions()` function:
   ```cpp
   emotionRegistry.add({
     EMOTION_NEW_EMOTION,
     "NEW_EMOTION",
     51,              // frameCount
     30,              // frameDelayMs
     LOOP_RESTART,    // loopMode
     true,            // isBlinkable
     drawNewEmotion   // drawFn
   });
   ```

5. **Test** — Add test in `test/test_sangi.cpp`, then run `~/.platformio/penv/bin/pio test -e native`

---

## Common Debugging

### Serial Monitor Not Showing Output
- Check port: `ls /dev/ttyUSB*`
- Try different port: `platformio device monitor --port /dev/ttyUSB0`
- Verify baud rate: 115200

### Display Not Initializing
- Verify I2C wiring (GPIO 6 = SDA, GPIO 7 = SCL)
- Check I2C address: Should be 0x3C for SSD1306 (hardcoded in code)
- Enable debug in `DisplayManager::init()` to see I2C scan output

### Gesture Not Detected
- Touch on capacitive button (GPIO 3)
- Check gesture thresholds in `config.h`: `LONG_PRESS_MS`, `DOUBLE_TAP_WINDOW_MS`
- Verify gesture state machine in `src/input.cpp` with serial debug output

### BLE Connection Issues
- Device should advertise as "SANGI" — check serial monitor for "BLE: advertising"
- Use **nRF Connect** app (free, iOS/Android) to scan and connect
- Service UUID: `face0001-...`, Characteristic: `face0002-...` (dynamically generated)

---

## Dependencies

```ini
adafruit/Adafruit SSD1306@^2.5.7
adafruit/Adafruit GFX Library@^1.11.3
h2zero/NimBLE-Arduino@^1.4.0
```

All specified in `platformio.ini`. Zero network, cloud, or external service dependencies.
