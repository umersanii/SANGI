# CLAUDE.md
## v1 Simplification in progress
Architecture is being migrated from 3-device AWS IoT system to single ESP32-C3.
See SANGI_V1_PLAN.md for full module breakdown and implementation order.
Current state: [MODULE NAME] in progress / not started.
Do not assume the architecture described below is current — it reflects the pre-v1 state.

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Test Commands

```bash
# ESP32 firmware
/home/sani/.platformio/penv/bin/pio run                                    # Build
/home/sani/.platformio/penv/bin/pio run --target upload                    # Upload to device
/home/sani/.platformio/penv/bin/pio device monitor --port /dev/ttyUSB1 --baud 115200  # Serial monitor
/home/sani/.platformio/penv/bin/pio test -e native                         # Run native (desktop) unit tests

# Pi notification service
cd pi-setup && pip install -e . && python -m sangi_notify

# Frontend
cd Frontend/sagni && npm install --legacy-peer-deps && npm run dev
```
## Architecture

Three-layer system communicating over AWS IoT Core MQTT:

1. **ESP32-C3 firmware** (`src/`, `include/`) — SSD1306 OLED robot face with 16 animated emotions, touch input, battery monitoring, speaker
2. **Raspberry Pi service** (`pi-setup/sangi_notify/`) — 24/7 notification monitoring (GitHub, Discord, D-Bus), forwards to ESP32 via MQTT
3. **Web dashboard** (`Frontend/sagni/`) — Next.js + MQTT.js real-time control panel

**PC workspace monitor** (`PC-setup/`) — detects coding activity, maps to emotions

## Key Patterns

**ICanvas** (`include/canvas.h`): Abstract display interface. `DisplayManager` implements it for hardware; `MockCanvas` (`test/mock_canvas.h`) records draw calls for testing. All emotion rendering goes through ICanvas — never access OLED directly.

**EmotionRegistry** (`include/emotion_registry.h`): Data-driven emotion definitions. Each emotion is registered via `emotionRegistry.add({enum, name, frameCount, frameDelay, loopMode, blinkable, drawFn})` in `main.cpp`. Eliminates switch statements. Adding a new emotion: enum in `emotion.h`, draw function in `emotion_draws.cpp`, one `registry.add()` call. See `NEW_EMOTION_HOWTO.md`.

**EmotionManager state machine** (`include/emotion.h`): 7-frame blink transition between emotions (3 close + 3 open + target). Callbacks (`onTransitionComplete`, `onEmotionChange`) injected by `main.cpp` to avoid circular deps.

**MQTT subscription registry** (`include/mqtt_manager.h`): Topic handlers registered via `mqttManager.subscribe(topic, handler)` — no if/else chains. Handlers are free functions in `main.cpp`.

**Monitor base class** (`pi-setup/sangi_notify/monitors/base.py`): ABC with `poll()` + `interval` property. Threading loop in `start()`. Discord and D-Bus monitors override `start()` for their own event loops.

## MQTT Topics

```
sangi/emotion/set          # Pi/Web → ESP32: {"emotion": 1}
sangi/notification/push    # Pi → ESP32: {"title": "...", "message": "..."}
sangi/github/stats         # Pi → ESP32: profile stats
sangi/github/commits       # Pi → ESP32: contribution heatmap
sangi/discord/stats        # Pi → ESP32: DM/mention counts
sangi/status               # ESP32 → Cloud: online status
sangi/battery              # ESP32 → Cloud: voltage/percentage
sangi/status/emotion       # ESP32 → Cloud: current emotion
```

SSID validation prevents cross-device command injection when multiple SANGIs share a broker.

## Native Testing

Tests in `test/test_sangi.cpp` (Unity framework). Arduino API stubbed in `test/arduino_stub/Arduino.h` — provides `millis()` (shared across TUs via inline+static-local pattern), `Serial`, GPIO no-ops, `String`. The `[env:native]` PlatformIO environment compiles only the source files listed in `build_src_filter`.

## Hardware Constants

All in `include/config.h`. I2C pins are **fixed** on ESP32-C3 (GPIO 6 SDA, GPIO 7 SCL). Speaker on GPIO 10 (GPIO 9 conflicts with USB/boot). Battery ADC on GPIO 2. Touch on GPIO 3.

## Critical Rules

- Global manager instances are `extern` singletons — never instantiate locally
- Never call `display.begin()` twice (I2C hang)
- All pin assignments and timing constants live in `config.h`
- Animation draw functions use `static` local variables for state persistence across ticks
- Compile flags `ENABLE_MQTT`, `DEBUG_MODE_ENABLED`, `ENABLE_EMOTION_BEEP` gate optional features; native tests disable all three
