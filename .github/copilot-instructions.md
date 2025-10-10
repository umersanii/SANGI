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
