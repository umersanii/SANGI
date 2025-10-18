# SANGI - GitHub Copilot Instructions

ESP32-C3 companion robot with OLED animations, emotion state machine, and AWS IoT MQTT integration.

## Essential Documentation

**CRITICAL**: This project maintains ONLY these 5 documentation files in the root directory. Never create additional .md files elsewhere.

- **README.md** - Quick start, setup, and usage guide
- **ARCHITECTURE.md** - System design, patterns, and module responsibilities
- **Code_Reference.md** - API reference, conventions, and code examples
- **CHANGELOG.md** - Version history and feature updates
- **LICENSE** - Project license terms

**Documentation Rules**:
1. All new documentation goes into one of these 5 files
2. Never create new .md files in `docs/`, `PC-setup/`, or subdirectories
3. Keep content concise, complete, and actionable
4. Update CHANGELOG.md for all significant changes

## Core Architecture

**Singleton Manager Pattern**: All subsystems use global manager instances declared in headers:
```cpp
extern EmotionManager emotionManager;
extern DisplayManager displayManager;
extern AnimationManager animationManager;
extern BatteryManager batteryManager;
extern InputManager inputManager;
extern NetworkManager networkManager;
```

**Configuration-First**: All pins/constants in `include/config.h` - never hardcode values.

**State Flow**: `EmotionManager` → `DisplayManager` checks `isTransitionActive()` → Render static face or call `AnimationManager` for complex animations.

## Quick Commands

```bash
pio run --target upload    # Build and flash
pio device monitor         # Serial output (115200 baud)
sudo chmod 666 /dev/ttyUSB0  # Linux USB access
```

## Critical Rules

1. **Never** instantiate managers locally - use global instances only
2. **Never** call `display.begin()` outside `DisplayManager::init()` (causes I2C hang)
3. **Always** add new constants to `include/config.h`, not inline
4. **Always** build with `pio run` before committing changes
5. ESP32-C3 I2C **only on GPIO 6/7** (hardware fixed, non-configurable)

## Adding New Emotions

1. Add enum to `include/emotion.h`
2. Implement `drawFace_X()` in `src/display.cpp` OR `animateX()` in `src/animations.cpp`
3. Update `src/main.cpp` loop switch statement
4. Test both autonomous and test modes
5. Document in Code_Reference.md and update CHANGELOG.md

## Common Pitfalls

- **millis() overflow**: Use overflow detection for timing comparisons (49-day wrap)
- **Battery calibration**: Adjust voltage divider formula in `include/config.h`
- **MQTT certificates**: Must exist in `certs/` before network initialization
- **Frame timing**: Use static variables in animation loops with modulo wrapping
- **Documentation sprawl**: Never create additional .md files - update existing ones only

## Making Changes

When implementing new features:
1. Update relevant code files
2. Add constants to `include/config.h` if needed
3. Document API changes in Code_Reference.md
4. Add architecture notes to ARCHITECTURE.md if pattern changes
5. Update CHANGELOG.md with version entry
6. Test with `pio run --target upload`

For detailed architecture, API reference, and change history, consult the 5 essential documentation files listed above.

---

## 📝 End-of-Task Documentation Checklist

**ALWAYS review before completing any task:**

- [ ] Did I add/modify any public APIs? → Update **Code_Reference.md**
- [ ] Did I change system design or module responsibilities? → Update **ARCHITECTURE.md**
- [ ] Did I add features, fix bugs, or make breaking changes? → Update **CHANGELOG.md**
- [ ] Did I change hardware pins or constants? → Verify documented in **Code_Reference.md**
- [ ] Did user request new documentation? → Add to one of the 5 essential files, **never create new .md files**

**Remember**: The 5-file documentation standard is non-negotiable. All documentation must go into README.md, ARCHITECTURE.md, Code_Reference.md, CHANGELOG.md, or LICENSE.
