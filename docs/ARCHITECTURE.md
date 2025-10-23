# SANGI Architecture

ESP32-C3 companion robot with modular singleton architecture for emotion states, OLED animations, and IoT integration.

## System Design

**Pattern**: Singleton manager pattern with clean separation of concerns  
**Platform**: ESP32-C3 (160MHz, 280KB SRAM, 4MB Flash)  
**Framework**: Arduino/PlatformIO

## Core Modules

### EmotionManager (`emotion.h/cpp`)
**Responsibility**: State machine for emotion transitions

- 13 emotion states (IDLE, HAPPY, SAD, ANGRY, SURPRISED, CONFUSED, SLEEPY, THINKING, EXCITED, LOVE, DEAD, MUSIC, NOTIFICATION)
- 7-frame transition system (current → blink → target)
- Autonomous emotion selection based on time/battery
- MQTT-driven emotion control

**API**:
```cpp
void setTargetEmotion(EmotionState emotion);
EmotionState getCurrentEmotion();
bool isTransitionActive();
void advanceTransition();
```

### DisplayManager (`display.h/cpp`)
**Responsibility**: OLED rendering primitives

- SSD1306 128x64 I2C display (address 0x3C)
- Static face drawing (7 emotions)
- Batman boot logo
- I2C scanner utility
- Display buffer management

**API**:
```cpp
void init();
void clearDisplay();
void updateDisplay();
void drawFace_X();  // X = emotion name
```

**I2C Pins** (ESP32-C3 fixed):
- SDA: GPIO 6
- SCL: GPIO 7

### AnimationManager (`animations.h/cpp`)
**Responsibility**: Complex multi-frame animations

- 51-frame sequences for 8 emotions
- 30ms frame delay (~20 FPS)
- 86-frame notification animation (50ms delay, 4.3s total)
- Static frame state with millis() timing

**Animated Emotions**:
- SLEEPY: Eyes droop, zzz letters float
- THINKING: Eyes look up, thought bubbles
- EXCITED: Wide eyes, sparkles, bouncing
- HAPPY: Squinting joy, blush, smile
- LOVE: Heart eyes, floating hearts
- ANGRY: Shaking, furrowed brows
- SAD: Tears falling, droopy expression
- MUSIC: Headphones, head bobbing
- NOTIFICATION: 86-frame cinematic sequence (see below)

**Notification Animation Sequence** (86 frames @ 50ms = 4.3s):
- **Phase 1**: Surprise reaction (frames 0-5)
- **Phase 2**: SANGI runs off screen (frames 6-12)
- **Phase 3**: Notification board slides in (frames 13-18)
- **Phase 4**: Display content (frames 19-70, **52 frames = 2.6s**) - longest phase
- **Phase 5**: Board slides out (frames 71-76)
- **Phase 6**: SANGI returns calm to idle (frames 77-85)

**Notification Display Formatting**:
- Title: `setTextSize(2)` - large, prominent font
- Message: `setTextSize(1)` - smaller, readable font
- Animation plays once, holds on final idle frame (no loop)

### NetworkManager (`network.h/cpp`)
**Responsibility**: WiFi and AWS IoT MQTT with offline mode

- Certificate-based TLS 1.2 authentication
- NTP time synchronization
- Notification queue (max 5)
- Automatic reconnection (5s interval)
- **SSID validation** for cross-network isolation
- **Offline mode**: Autonomous emotion cycling when MQTT unavailable with notification generation

**Operating Modes**:
1. **Workspace Mode**: MQTT connected + valid SSID → workspace-driven emotions
2. **Offline Mode**: No MQTT or SSID mismatch → autonomous emotion cycling (20s intervals)

**SSID Validation**:
- Workspace monitor includes WiFi SSID in all MQTT messages
- ESP32 validates SSID matches its connected network
- Prevents neighbor's SANGI from responding to your messages
- Failed validation triggers offline mode

**Offline Detection**:
- MQTT disconnected, OR
- >60 seconds without valid MQTT message
- Automatically switches to autonomous emotion cycling

**Offline Emotion Cycling**:
- Random emotion selection every 20 seconds from full emotion set
- 13 emotions cycle: IDLE, HAPPY, SLEEPY, EXCITED, SAD, ANGRY, CONFUSED, THINKING, LOVE, SURPRISED, DEAD, MUSIC, NOTIFICATION
- BLINK excluded (handled by separate interval system)
- When NOTIFICATION selected → generates random system info notification:
  - Battery status: voltage + percentage
  - System uptime: hours/minutes/seconds
  - Content displayed via `animateNotification()` with offline data

**Topics**:
- Subscribe: `sangi/emotion/set`, `sangi/notification/push`
- Publish: `sangi/status`, `sangi/battery`, `sangi/emotion/current`

**API**:
```cpp
void init();
void update();
bool isConnected();
bool isInWorkspaceMode();
unsigned long getLastMQTTMessageTime();
bool addNotification(type, title, message);
bool hasNotifications();
```

### BatteryManager (`battery.h/cpp`)
**Responsibility**: ADC voltage monitoring

- 12-bit ADC on GPIO 2
- Voltage divider calibration
- Battery percentage calculation
- Low battery detection

**API**:
```cpp
void init();
float readVoltage();
int getBatteryPercentage();
EmotionState getBatteryBasedEmotion();
```

### InputManager (`input.h/cpp`)
**Responsibility**: Touch sensor handling

- Capacitive touch on GPIO 3
- Debouncing (500ms)
- Random emotion triggers

**API**:
```cpp
void init();
void handleTouchInteraction();
```

## Data Flow

```
┌─────────────────┐
│   main.cpp      │  Orchestration layer
│   (loop)        │
└────────┬────────┘
         │
    ┌────┴─────┬─────────────┬──────────────┐
    │          │             │              │
┌───▼────┐ ┌──▼────┐   ┌────▼─────┐   ┌───▼─────┐
│Battery │ │ Input │   │ Network  │   │ Emotion │
│Manager │ │Manager│   │ Manager  │   │ Manager │
└───┬────┘ └──┬────┘   └────┬─────┘   └───┬─────┘
    │         │              │             │
    └─────────┴──────────────┴─────────────┘
                      │
           ┌──────────▼────────────┐
           │   Emotion Manager     │
           │   (State Machine)     │
           └──────────┬────────────┘
                      │
         ┌────────────▼────────────┐
         │   isTransitionActive()? │
         └────────────┬────────────┘
                      │
           ┌──────────▼──────────┐
           │   Transition?       │
           │   Yes → Blink       │
           │   No → Static/Anim  │
           └──────────┬──────────┘
                      │
      ┌───────────────┴───────────────┐
      │                               │
┌─────▼────────┐           ┌──────────▼────────┐
│ DisplayManager│           │ AnimationManager  │
│ (Static Face) │           │ (51-frame loops)  │
└──────────────┘           └───────────────────┘
```

## Configuration

**All constants in `include/config.h`**:

```cpp
// Hardware
#define I2C_SDA 6
#define I2C_SCL 7
#define BATTERY_PIN 2
#define TOUCH_PIN 3
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Timing
#define EMOTION_CHANGE_INTERVAL 30000
#define FRAME_DELAY 30
#define STATUS_PUBLISH_INTERVAL 30000

// Features
#define ENABLE_MQTT true
#define DEBUG_MODE_ENABLED false
```

## Memory Layout

**Global Managers** (declared `extern` in headers):
```cpp
extern EmotionManager emotionManager;
extern DisplayManager displayManager;
extern AnimationManager animationManager;
extern BatteryManager batteryManager;
extern InputManager inputManager;
extern NetworkManager networkManager;
```

**RAM Usage** (approximate):
- Display buffer: 1KB (128×64÷8)
- MQTT stack: 15KB
- Notification queue: 1KB (5 notifications)
- Manager objects: 2KB
- **Total**: ~20KB / 280KB available

## Power Management

**Deep Sleep** (currently disabled):
- GPIO wakeup on touch sensor
- 5-minute idle timeout
- Displays SLEEPY animation before sleep

## Initialization Order

```cpp
void setup() {
  Serial.begin(115200);
  displayManager.init();
  emotionManager.init();
  animationManager.init();
  batteryManager.init();
  inputManager.init();
  #if ENABLE_MQTT
    networkManager.init();
  #endif
}
```

## State Transitions

**3-Phase Transition**:
1. **Current** → Eyes open, current emotion
2. **Blink** → Eyes closing (3 frames)
3. **Target** → Eyes open, new emotion

**Timing**: 150ms (3 frames × 50ms)

## PC Integration

**Workspace Monitor** (`PC-setup/`):
- Linux D-Bus notification detection
- File watcher for code changes
- Build process monitoring
- Audio playback detection
- MQTT publisher to AWS IoT

**Activity → Emotion Mapping**:
- High file changes → EXCITED
- Build process → THINKING
- Build success → LOVE/HAPPY
- Build failure → CONFUSED/ANGRY
- Audio playing → MUSIC
- Idle >10min → SLEEPY

### Notification System

**Module Architecture** (`PC-setup/lib/`):
```
workspace_monitor.py (orchestrator)
    ├── activity_tracker.py (event counting)
    ├── audio_detector.py (audio playback)
    ├── emotion_mapper.py (emotion logic)
    └── notification_detector.py (D-Bus notifications) ← NEW
```

**Notification Detector** (`notification_detector.py`):
- Uses D-Bus FreeDesktop notification spec
- Captures desktop notifications automatically
- Categorizes by source (Discord, System, etc.)
- Formats and publishes to MQTT

**Discord Notification Simplification**:
Discord messages are automatically simplified for clarity:
- **Input**: "friend_username - Direct Message" + "Hey, how are you?"
- **Output**: title="friend_username", message="new message"
- **Reason**: Privacy + OLED space constraints

**Notification Flow**:
```
Discord Desktop App
    ↓ (sends notification)
Linux D-Bus System
    ↓ (captures)
NotificationDetector (moved)

Note: Desktop D-Bus notification capture has been moved to the standalone
`pi-notification-service` (see `PC-setup/notification-service/README.md`). The
workspace monitor no longer includes live notification capture to keep PC and
Pi responsibilities separated.
    ↓ (formats: username + "new message")
workspace_monitor._on_notification()
    ↓ (publishes)
MQTT → AWS IoT Core
    ↓ (subscribes)
NetworkManager.addNotification()
    ↓ (triggers)
EmotionManager.setTargetEmotion(EMOTION_NOTIFICATION)
    ↓ (animates)
SANGI displays notification with peeking animation
```

**Supported Notification Types**:
- `discord` - Simplified format (username only)
- `system` - System notifications
- `generic` - Fallback for unknown sources

### Offline Notification Generation

When SANGI is in **offline mode** (no MQTT connection), it still displays notifications with auto-generated content:

**Offline Notification Generator** (`main.cpp`):
```cpp
void generateOfflineNotification() {
  // 50/50 random between battery status or uptime
  // Populates: offlineNotifTitle[32] and offlineNotifMessage[64]
}
```

**Generated Content**:
1. **Battery Status** (50% chance):
   - Title: "Battery Status"
   - Message: "3.85V (75%)" - voltage + percentage calculated from ADC
   
2. **System Uptime** (50% chance):
   - Title: "System Uptime"
   - Message: "2h 15m 43s" - formatted uptime since boot

**Trigger Logic**:
- Offline mode cycles through all 13 emotions randomly every 20 seconds
- When `EMOTION_NOTIFICATION` is randomly selected:
  1. `generateOfflineNotification()` creates random battery or uptime message
  2. Notification data stored in static globals (`offlineNotifTitle`, `offlineNotifMessage`)
  3. `animateNotification()` displays the generated content
  4. Next cycle picks different random emotion

**Implementation Flow**:
```
Offline Mode Active
    ↓ (20s interval)
Random emotion picker → EMOTION_NOTIFICATION selected
    ↓
generateOfflineNotification() → Random(battery status OR uptime)
    ↓
offlineNotifTitle = "Battery Status" or "System Uptime"
offlineNotifMessage = "3.85V (75%)" or "2h 15m 43s"
    ↓
animateNotification(offlineNotifTitle, offlineNotifMessage)
    ↓
SANGI displays system info as notification
```

## Critical Rules

1. **Never instantiate managers locally** - Use global instances only
2. **Never call `display.begin()` twice** - Causes I2C hang
3. **All pins/constants in `config.h`** - No hardcoded values
4. **I2C pins are fixed** - ESP32-C3 hardware limitation
5. **Static frame variables** - For animation state persistence

## Extension Points

**Adding New Emotions**:
1. Add to `EmotionState` enum in `emotion.h`
2. Implement `drawFace_X()` in `display.cpp` (static)
3. OR implement `animateX()` in `animations.cpp` (animated)
4. Update `main.cpp` loop switch statement

**Adding New Sensors**:
1. Create `SensorManager` class
2. Add `extern` declaration in header
3. Initialize in `main.cpp::setup()`
4. Call `update()` in `main.cpp::loop()`

## Testing

**Animation Test Mode** (`main.cpp`):
- Cycles all emotions every 10s
- Bypasses state machine
- Fast visual iteration

**Debug Mode** (`config.h`):
```cpp
#define DEBUG_MODE_ENABLED true
#define DEBUG_MODE_EMOTION EMOTION_MUSIC
```
