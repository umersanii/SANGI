# SANGI — Claude Code Rules

General rules for Claude Code when working in this repository. These apply to every task unless explicitly overridden by the user.

---

## Hard Prohibitions (Never Do These)

- **Never use `delay()`** — it blocks the tick loop and breaks all timing-dependent subsystems
- **Never call Adafruit library methods directly** — all rendering must go through `ICanvas`; raw `display.fillCircle()` etc. are forbidden
- **Never instantiate manager classes locally** — always use the global extern singleton (`emotionManager`, `displayManager`, etc.)
- **Never call `display.begin()` more than once** — causes I2C hang; init happens once in `DisplayManager::init()`
- **Never use `new` or `malloc`** — heap fragmentation is unsafe on ESP32-C3; prefer stack allocation and static locals
- **Never add a new emotion via a `switch` statement** — always use `emotionRegistry.add()` in `registerEmotions()`
- **Never add global state outside of the established singleton pattern** — no new file-level globals
- **Never write to hardware registers directly** — use the abstraction layer

---

## Code Style

- **Indentation:** 2 spaces (no tabs)
- **Braces:** Allman style — opening brace on its own line for functions and classes; same line for control flow
- **Naming:**
  - Classes: `PascalCase` (e.g. `EmotionManager`)
  - Methods & functions: `camelCase` (e.g. `getCurrentEmotion()`)
  - Constants & macros: `UPPER_SNAKE_CASE` (e.g. `LONG_PRESS_MS`)
  - Local variables: `camelCase`
  - File names: `snake_case.cpp` / `snake_case.h`
- **`const` everywhere possible** — prefer `const` for variables that don't change
- **No magic numbers in logic** — all timing and threshold values must come from `include/config.h`

---

## Architecture Rules

### ICanvas Interface
- All drawing calls go through `ICanvas&` — never through a concrete type
- Draw functions must accept `ICanvas& canvas` as their only parameter
- `MockCanvas` is used in tests; `DisplayManager` is used in firmware — they are interchangeable

### EmotionRegistry Pattern
- New emotions require changes to exactly **4 files in sync:**
  1. `include/emotion.h` — add enum value before `EMOTION_COUNT`
  2. `src/emotion_draws.cpp` — add draw function with static frame state
  3. `include/emotion_draws.h` — declare the draw function
  4. `src/main.cpp` — call `emotionRegistry.add(...)` inside `registerEmotions()`
- Forgetting any one of these four will cause a compile error or silent misbehavior

### Callback Injection Pattern
- Modules must not hold references to each other
- All cross-module communication goes through callbacks set in `main.cpp`
- Example: `emotionManager.setOnEmotionChange(...)` — never `emotionManager.speaker.beep()`

### Animation State
- Draw functions use `static` local variables to persist frame state across `tick()` calls
- Frame counter example: `static int frame = 0;`
- Loop behavior is controlled by the `loopMode` field in the registry, not inside the draw function

### config.h is the Single Source of Truth
- Never hardcode timing values (ms thresholds, intervals, percentages) in `.cpp` files
- If you add a new tunable constant, it goes in `include/config.h` with a comment explaining units and purpose
- Changing a constant in `config.h` may require updating matching thresholds in test assertions — always check

---

## Testing Rules

- All new features require at least one native unit test before hardware upload
- Every new emotion requires tests covering: frame advance, loop behavior, and `MockCanvas` draw call recording
- Tests run on host machine — no ESP32-C3, no real OLED, no BLE stack
- Use `resetMillis()` before each test and `advanceMillis(ms)` to simulate time — never sleep or busy-wait
- Do not test hardware I/O (GPIO, I2C, BLE) directly — use stubs
- All 64 tests must pass before any PR or upload: `~/.platformio/penv/bin/pio test -e native`
- Test function names must describe the behavior being tested: `test_attention_arc_triggers_at_5_minutes()` not `test_personality_1()`

---

## Serial Debug Style

- Use `Serial.printf()` — never `Serial.print()` + `Serial.println()` chains
- Prefix all debug output with a bracketed module tag:
  - `[BLE]`, `[INPUT]`, `[PERSONALITY]`, `[DISPLAY]`, `[BATTERY]`, `[EMOTION]`
- Example: `Serial.printf("[PERSONALITY] Attention stage 2: SAD after %lums\n", elapsed);`
- Debug output must be gated behind `DEBUG_MODE_ENABLED` or a module-level flag — no unconditional spam in production builds

---

## Build & Environment

- Always build with: `~/.platformio/penv/bin/pio run`
- Always test with: `~/.platformio/penv/bin/pio test -e native`
- Never modify `platformio.ini` `build_src_filter` without understanding which files are excluded from native tests and why
- Hardware-specific modules (`display.cpp`, `speaker.cpp`, `ble_control.cpp`, `battery.cpp`) are excluded from native builds — stub them in tests, never add hardware calls to testable modules

---

## Adding a New Emotion — Checklist

1. [ ] Add enum in `include/emotion.h` (before `EMOTION_COUNT`)
2. [ ] Write draw function in `src/emotion_draws.cpp` using `static int frame`
3. [ ] Declare draw function in `include/emotion_draws.h`
4. [ ] Register in `src/main.cpp` inside `registerEmotions()` with correct `frameCount`, `frameDelayMs`, `loopMode`, `isBlinkable`
5. [ ] Write at least one native unit test covering frame advance and draw calls
6. [ ] Run `pio test -e native` — all 64+ tests must pass
7. [ ] Run `pio run` — firmware must compile clean with zero warnings

---

## What Requires Human Review

Flag these to the user before proceeding:

- Any change to `include/config.h` timing constants (affects personality and gesture behavior in ways tests may not catch)
- Any change to the 7-frame blink transition logic in `emotion.cpp`
- Any new global singleton or extern declaration
- Adding a dependency to `platformio.ini`
- Changes to `build_src_filter` in `platformio.ini`