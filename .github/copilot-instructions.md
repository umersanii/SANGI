<!-- SANGI: AI coding agent instructions (concise, actionable). -->
# SANGI — Copilot / AI Agent Instructions

These notes give an AI coding agent the concrete, repository-specific knowledge needed to be productive quickly.

## Big picture
- Single micro-controller project (ESP32-C3) implementing a pocket companion robot with OLED face animations.
- Clear modular singleton manager pattern: each subsystem exposes a single global manager instance (e.g. `EmotionManager`, `DisplayManager`, `AnimationManager`, `BatteryManager`, `InputManager`). See `include/*.h` and `src/*.cpp` for implementations.
- Responsibility boundaries:
  - `emotion.*` (state machine & transitions) — `include/emotion.h`, `src/emotion.cpp`
  - `display.*` (OLED primitives & draws) — `include/display.h`, `src/display.cpp`
  - `animations.*` (multi-frame sequences) — `include/animations.h`, `src/animations.cpp`
  - `battery.*` (ADC read & calibration) — `include/battery.h`, `src/battery.cpp`
  - `input.*` (touch sensor handling) — `include/input.h`, `src/input.cpp`
  - `network.*` (WiFi & MQTT/AWS IoT) — `include/network.h`, `src/network.cpp`

## Architecture rules the agent must follow
- Never instantiate managers locally. Use the global instances (declared `extern` in headers). Example: `extern EmotionManager emotionManager;` (see `include/README`).
- All hardware pin numbers and timing constants live in `include/config.h`. Do not hardcode pins or magic numbers — add new constants to `config.h`.
- Transitions: `EmotionManager` drives transitions. Display/animation code must not manage emotion transition state directly — call `emotionManager.isTransitionActive()` when needed.

## Project-specific conventions & patterns
- Naming: headers in `include/` use snake-lowercase (e.g. `display.h`), implementations in `src/` match names (`display.cpp`).
- Enums & public interfaces live in headers (e.g., `EmotionState` in `include/emotion.h`).
- Animation loops use static locals for frame state and `millis()` timing. Example pattern in `animations.cpp`:

  void AnimationManager::animateX() {
    static unsigned long lastFrame = 0;
    static int frameIndex = 0;
    if (millis() - lastFrame > FRAME_DELAY) { ... }
  }

- Display updates follow: `displayManager.clearDisplay(); draw...; displayManager.updateDisplay();`
- I2C/OLED: SSD1306 at `0x3C` (check `scanI2C()` when unsure). Screen is 128x64 monochrome.

## Critical developer workflows
- Build & upload (PlatformIO is the ONLY supported build system):

  pio run --target upload

  Serial monitor:

  pio device monitor

  On Linux you may need to allow USB access first:

  sudo chmod 666 /dev/ttyUSB0

- Testing animations: `main.cpp` supports two modes:
  1. Animation test mode (cycles emotions every 10s) — fast visual iteration
  2. Autonomous mode — production state machine using battery/time/touch
  Switch by editing `src/main.cpp::loop()` (look for the comment near the top of `loop()`).

## Integration points & hardware constraints
- ESP32-C3 specifics:
  - I2C only on GPIO6/7 (project assumes this)
  - ADC1 used for battery; default uses GPIO2
  - Deep sleep wake uses `gpio_wakeup_enable()` (not ext0)
- OLED driver: `Adafruit_SSD1306` style calls are used; avoid calling `display.begin()` twice.
- Battery reading requires calibration (voltage divider) — formula and constants live in `battery.cpp`/`include/config.h`.

## Cross-component patterns
- Emotion flow: EmotionManager sets target → Display checks `isTransitionActive()` → If transition active: 3-frame transition (current → blink → target) → otherwise either static face (drawFace_X) or call `animationManager.animateX()` for complex animations.
- AnimationManager methods are fully responsible for multi-frame sequences; they must not touch emotion state directly.

## Files to inspect for examples
- `include/config.h` — hardware pins and timing constants (first stop for any magic numbers)
- `src/main.cpp` — orchestration and mode switching (animation test vs autonomous)
- `src/display.cpp` / `include/display.h` — drawing primitives and update pattern
- `src/animations.cpp` — multi-frame animation examples
- `src/emotion.cpp` — state machine & transitions

## Quick tips for edits and PRs
- Small, targeted changes: update `include/config.h` for any new hardware constants.
- When adding an emotion:
  1. Add enum value to `include/emotion.h`
  2. For static faces: implement `drawFace_NewEmotion()` in `src/display.cpp`
  3. For animated faces: implement `animateNewEmotion()` in `src/animations.cpp`
  4. Wire in the `loop()` switch in `src/main.cpp`
- Always build locally with `pio run` before opening a PR.

## What not to change
- Do not call `display.begin()` outside `DisplayManager::init()` (it can hang I2C).
- Do not create additional global manager instances — follow singleton pattern.

## Network/Communication ✅ MQTT OPERATIONAL (October 2025)
- **Status**: Successfully deployed with AWS IoT Core integration
- **Implementation**: `network.h/cpp` module following singleton manager pattern
- **Configuration**: WiFi & MQTT settings in `include/secrets.h` (gitignored)
- **Topics**:
  - Subscribe: `sangi/emotion/set` - Remote emotion control
  - Publish: `sangi/status` - Connection status updates
  - Publish: `sangi/battery` - Battery telemetry (every 30s)
  - Publish: `sangi/emotion/current` - Current emotion state (every 30s)
- **Features**:
  - Certificate-based AWS IoT Core authentication (TLS 1.2)
  - Automatic fallback to autonomous mode when network unavailable
  - NTP time synchronization for accurate timestamps
  - Reconnection logic with 5s interval
- **Setup**: See `docs/MQTT_SETUP.md` for complete configuration guide
- **Enable/Disable**: Set `ENABLE_MQTT` in `include/config.h`

## When something's missing
- If hardware pin mapping or a timing constant is not in `include/config.h`, add it there and document usage with a short comment.
- If you need to change startup/init order, search for `init()` calls in `src/main.cpp` to keep manager initialization centralized.

---
If any section is unclear or you'd like me to expand examples (e.g., show exact `config.h` constants or a sample animation implementation), tell me which file or behaviour and I'll update the instruction file.
# SANGI Robot - AI Agent Instructions

## Project Context
ESP32-C3 companion robot with animated OLED expressions. Built on **modular singleton architecture** with clean separation between hardware control, emotion logic, and animation rendering. Target: autonomous "pocket pet" that can later integrate with workspace monitoring systems.

## Critical Architecture Patterns

### Manager Singleton Pattern
Every major subsystem uses a global manager instance:
```cpp
extern EmotionManager emotionManager;    // State machine & transitions
extern DisplayManager displayManager;    // OLED rendering & I2C
extern AnimationManager animationManager; // Complex multi-frame animations
extern BatteryManager batteryManager;    // ADC voltage monitoring
extern InputManager inputManager;        // Touch sensor handling
extern NetworkManager networkManager;    // WiFi & MQTT communication (AWS IoT Core)
```

**Rule**: Never instantiate managers locally. Always use the global instance. Initialization happens in `main.cpp::setup()`, usage in `main.cpp::loop()`.

### Configuration-First Approach
ALL hardware pins and timing constants live in `include/config.h`:
```cpp
#define I2C_SDA 6
#define SCREEN_WIDTH 128
#define EMOTION_CHANGE_INTERVAL 30000
```
**Never hardcode pins or magic numbers** in implementation files. Add new constants to `config.h` first.

### Emotion State Flow
```
EmotionManager sets target → DisplayManager checks isTransitionActive() 
→ If true: 3-frame transition (current → blink → target)
→ If false: Static face OR call AnimationManager for complex emotions
```

**Key**: `EMOTION_SLEEPY`, `EMOTION_THINKING`, `EMOTION_EXCITED`, etc. have dedicated `animationManager.animateX()` functions with multi-frame sequences. Other emotions use static `displayManager.drawFace_X()`.

## Development Workflows

### Building & Uploading
```bash
# PlatformIO is the ONLY build system (no Arduino IDE)
pio run --target upload          # Build and upload
pio device monitor               # Serial output (115200 baud)
sudo chmod 666 /dev/ttyUSB0      # Linux: Grant USB access first
```

### Testing Animation Changes
`main.cpp` has **two modes**:
1. **Animation test mode** (current): Cycles through all emotions every 10s
2. **Autonomous mode** (production): Time-based + battery + touch triggers

Switch by modifying the `loop()` logic. Test mode bypasses state machine for rapid visual iteration.

### Adding New Emotions
1. Add to `EmotionState` enum in `emotion.h`
2. Static face: Implement `drawFace_NewEmotion()` in `display.cpp`
3. Animated face: Implement `animateNewEmotion()` in `animations.cpp`
4. Wire into `loop()` switch statement in `main.cpp`
5. Update time/battery/touch logic in respective managers if needed

## Hardware-Specific Constraints

**ESP32-C3 Limitations**:
- **I2C only on GPIO6/7** (hardcoded, not configurable)
- **ADC1 for battery** (GPIO0-4): Use GPIO2 by default
- **Deep sleep wake** uses `gpio_wakeup_enable()` not `ext0_wakeup` (different from ESP32)

**OLED Display**:
- SSD1306 at address `0x3C` (sometimes `0x3D` - check with `scanI2C()`)
- 128x64 pixels, 1-bit color
- Animations use `clearDisplay()` → draw → `updateDisplay()` pattern
- **Frame rate**: ~50ms delay between frames (20 FPS) balances smoothness vs power

## Project-Specific Conventions

### File Naming
- Headers: `include/module_name.h` (lowercase, descriptive)
- Implementation: `src/module_name.cpp` (matches header)
- Main: `src/main.cpp` (orchestration layer ONLY, ~150 lines)

### Code Organization
- **Enums & interfaces**: Headers (e.g., `EmotionState` in `emotion.h`)
- **Drawing primitives**: `display.cpp` (anything that touches Adafruit_SSD1306)
- **Multi-frame sequences**: `animations.cpp` (>3 frame animations with timing)
- **State logic**: `emotion.cpp` (when to transition, time-based selection)

### Animation Frame Structure
Complex animations follow this pattern:
```cpp
void AnimationManager::animateEmotion() {
  static unsigned long lastFrame = 0;
  static int frameIndex = 0;
  
  if (millis() - lastFrame > FRAME_DELAY) {
    displayManager.clearDisplay();
    // Draw frame based on frameIndex
    displayManager.updateDisplay();
    frameIndex = (frameIndex + 1) % TOTAL_FRAMES;
    lastFrame = millis();
  }
}
```
**Key**: Static variables for frame state, modulo operator for looping.

## Integration Points

### Power Management (Currently Disabled)
`checkSleepConditions()` in `main.cpp` is **commented out** until touch sensor is connected. When enabling:
- Requires GPIO wakeup configuration (ESP32-C3 specific)
- 5-minute timeout defined in `config.h`
- Displays sleepy animation before deep sleep

### Future Network Layer (Not Yet Implemented)
Architecture designed for HTTP/MQTT integration:
- Emotion state will be settable via `/api/emotion/{state}` endpoint
- Fallback to autonomous mode when network unavailable
- See `.github/copilot.md` "Feature Tier 2" for full spec

**UPDATE (Oct 2025)**: MQTT via AWS IoT Core is now fully operational. See `network.h/cpp` and `docs/MQTT_SETUP.md`.

## Common Pitfalls

1. **Don't call `display.begin()` twice** - Causes I2C hang. Only in `DisplayManager::init()`
2. **Transition state is managed by EmotionManager** - Don't manually set frames in display code
3. **Battery voltage needs calibration** - Default assumes 3.3V direct read. LiPo requires voltage divider and formula adjustment
4. **Touch threshold varies by hardware** - TTP223 module vs bare capacitive pad needs different `TOUCH_THRESHOLD` values
5. **Animation loops must update static frameIndex** - Forgetting causes frozen frames

## Debug Output Patterns
```cpp
Serial.printf("Battery: %.2fV | Emotion: %d | Uptime: %lus\n", 
              voltage, emotionManager.getCurrentEmotion(), uptime);
```
- Battery voltage: Should read ~3.3V on USB, 3.0-4.2V on LiPo
- Emotion number: Maps to `EmotionState` enum (0=IDLE, 1=HAPPY, etc.)
- Uptime: Seconds since boot - validates `millis()` overflow handling

## When Making Changes

**Before editing**:
1. Check if functionality exists in a manager - don't duplicate
2. Verify hardware pin availability in `config.h`
3. Consider power consumption impact (animations drain battery fast)

**After editing**:
1. Build with `pio run` before uploading to catch syntax errors
2. Check serial output for state machine behavior
3. Verify no memory leaks (static allocations preferred over dynamic)

## Key Files to Reference

- **`COPILOT.md`**: Full refactoring rationale and modular architecture benefits
- **`.github/copilot.md`**: Long-term vision with 3-tier feature roadmap
- **`REFACTORING_SUMMARY.md`**: Quick module responsibility reference
- **`HARDWARE_WIRING.md`**: Pin connections and circuit diagrams
- **`PHASE1_IMPLEMENTATION.md`**: Current implementation status and testing guide

## Code Style
- `camelCase` for variables/functions, `PascalCase` for classes/enums
- `UPPER_CASE` for #defines
- Comments: `//` for single-line, `/* */` for multi-line blocks
- Section headers: `// ===== SECTION NAME =====` (5 equals signs)

## Quick Reference

**Test current emotion**: `emotionManager.getCurrentEmotion()`  
**Force emotion change**: `emotionManager.setTargetEmotion(EMOTION_HAPPY)`  
**Check if animating**: `emotionManager.isTransitionActive()`  
**Read battery**: `batteryManager.readVoltage()`  
**Detect touch**: `inputManager.handleTouchInteraction()`  
**Clear screen**: `displayManager.clearDisplay(); displayManager.updateDisplay();`  
**Check MQTT status**: `networkManager.isConnected()`  
**Send MQTT message**: Publish to `sangi/emotion/set` with JSON `{"emotion": 1}`
