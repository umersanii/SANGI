# SANGI Changelog

All notable changes to the SANGI robot project.

## [Unreleased]

### Added
- **Offline Mode with SSID Validation** (2025-10-18)
  - SANGI now operates in two modes: Workspace Mode and Offline Mode
  - **Workspace Mode**: MQTT connected with valid SSID - emotions controlled by workspace monitor
  - **Offline Mode**: No MQTT or SSID mismatch - autonomous emotion cycling (20s intervals)
  - SSID validation prevents cross-network interference (e.g., neighbor's SANGI)
  - Workspace monitor now includes WiFi SSID in all MQTT messages
  - ESP32 validates incoming SSID matches current WiFi network
  - Automatic fallback to offline mode after 60s without valid MQTT messages
  - Offline mode cycles through all emotions as a primitive emotion cycler
  - Random emotion selection with 20-second display time for natural feel
  - Configuration: `MQTT_TIMEOUT_THRESHOLD` (60s), `OFFLINE_EMOTION_INTERVAL` (20s)

### Changed
- **Discord Notification Simplification** (2025-10-18)
  - Discord notifications now use simplified format for clarity
  - Title: Username only (extracted from notification summary)
  - Message: Always displays "new message"
  - Removes message content for privacy and OLED space optimization
  - Automatic username extraction from D-Bus notification data
  - Applied to both test notifications and live workspace monitor

- **Notification System Integration** (2025-10-18)
  - Integrated NotificationDetector into main workspace_monitor.py
  - Added automatic MQTT publishing callback for notifications
  - Enhanced notification_detector.py with Discord-specific formatting
  - Updated test_notifications.py to focus on relevant notification types

- **Notification Transition System** (2025-10-18)
  - Removed peeking eyes from static notification display
  - Added dedicated notification transition animation (surprise → run away → notification appears)
  - Fixed transition FROM notification to use standard blink (no peeking eyes)
  - Improved text content padding - now stays within double border outline
  - Notification box repositioned for better screen centering

- **Music Animation Overhaul** (2025-10-18)
  - Removed headphones design - replaced with closed eyes
  - Added smooth left-to-right swaying/humming motion
  - Music notes now float to top-left and top-right corners
  - Reduced note density for cleaner visual
  - Smooth 30ms frame timing (20 FPS) for fluid motion
  - Full animation cycle: 51 frames (~1.5 seconds)

### Planned
- WhatsApp notifications support (D-Bus desktop notifications)
- GitHub notifications support (desktop app or API polling)
- Motor control for movement
- Line following sensors
- Sound/buzzer feedback
- 3D printed enclosure
- Web dashboard for MQTT control
- Multi-device support

## [2.0.0] - 2025-10-18

### Major Changes
- **Comprehensive code review and bug fixes**
- Improved code quality from 75/100 to 88/100
- Added overflow protection for millis() timing
- Enhanced input validation across all modules

### Fixed
- Millis() overflow vulnerability (49-day wrap bug)
- Missing emotion state validation in MQTT handler
- Buffer overflow in notification text display
- Notification queue underflow protection
- ADC out-of-range reading safety
- Network timing overflow in reconnection logic

### Added
- Overflow detection pattern for timing comparisons
- Bounds checking for emotion state changes
- String truncation for OLED display safety
- ADC value clamping (0-4095 range)
- Input validation for all external data sources

### Improved
- Random distribution for touch interactions
- Error handling in Python workspace monitor
- Certificate validation in PC monitor
- Code documentation and inline comments

## [1.5.0] - 2025-10-17

### Added
- **PC Workspace Monitor Integration**
- D-Bus notification detection on Linux
- Real-time activity tracking (file changes, builds, errors)
- Audio playback detection for MUSIC emotion
- Emotion mapping based on coding activity

### Features
- Automatic emotion updates from PC activity
- Build process monitoring
- Error tracking integration
- Clipboard and keyboard activity detection
- Systemd service for auto-start

## [1.4.0] - 2025-10-14

### Added
- **Notification System**
- 66-frame notification animation sequence
- Notification queue (max 5 notifications)
- Multi-source support (Discord, Slack, email, GitHub, calendar)
- Animated display: surprise → run off → board slides in → show content → board slides out → return angry
- `EMOTION_NOTIFICATION` (13) state

### Technical
- NotificationDetector module for D-Bus monitoring
- Notification queue management in NetworkManager
- Text truncation for OLED display (title: 31 chars, message: 63 chars)

## [1.3.0] - 2025-10-14

### Added
- **MQTT Integration via AWS IoT Core**
- Remote emotion control
- Real-time telemetry (battery, emotion state, uptime)
- Certificate-based TLS 1.2 authentication
- Automatic fallback to autonomous mode

### Topics
- Subscribe: `sangi/emotion/set`, `sangi/notification/push`
- Publish: `sangi/status`, `sangi/battery`, `sangi/emotion/current`

### Configuration
- `secrets.h` for WiFi and AWS credentials
- `ENABLE_MQTT` feature flag in `config.h`
- NTP time synchronization
- 5-second reconnection interval

## [1.2.0] - 2025-10-12

### Added
- **EMOTION_MUSIC** (11) - Music listening animation
- 51-frame music animation with smooth swaying motion
- Closed eyes with relaxed expression
- Music notes floating to top corners (dispersed pattern)
- Slow left-to-right humming movement across full cycle
- 30ms frame timing for smooth animation

### Features
- Audio playback detection in PC monitor
- PulseAudio stream monitoring
- Media player process detection with CPU usage validation

## [1.1.0] - 2025-10-10

### Added
- **Complex Emotion Animations**
- 51-frame sequences for 7 emotions
- 30ms frame delay (~20 FPS)
- Animated versions of: HAPPY, LOVE, ANGRY, SAD, SURPRISED, CONFUSED, DEAD

### Animations
- HAPPY: Squinting joy with sparkles and blush
- LOVE: Pulsing heart eyes with floating hearts
- ANGRY: Violent shaking with furrowed brows
- SAD: Falling tears with droopy eyes
- SURPRISED: Wide eyes with quick blinks
- CONFUSED: Asymmetric eye wobble with question marks
- DEAD: X eyes formation with tongue and ghosts

## [1.0.0] - 2025-10-05

### Initial Release
- **Modular Singleton Architecture**
- EmotionManager with state machine
- DisplayManager for OLED rendering
- AnimationManager for complex sequences
- BatteryManager for voltage monitoring
- InputManager for touch sensor

### Emotions
- 13 emotion states implemented
- 7 static faces: IDLE, SAD, ANGRY, SURPRISED, CONFUSED, DEAD, BLINK
- 3 animated faces: SLEEPY, THINKING, EXCITED

### Features
- 128x64 SSD1306 OLED display
- ESP32-C3 microcontroller
- Batman boot logo
- 7-frame smooth transitions
- Autonomous emotion cycling
- Touch sensor interaction
- Battery monitoring

### Technical
- PlatformIO build system
- Arduino framework
- I2C communication (GPIO 6/7)
- 12-bit ADC for battery reading
- Configuration-first approach (`config.h`)

## [0.9.0] - 2025-09-25

### Pre-release
- Initial prototype with basic animations
- Simple emotion cycling
- Direct Adafruit library usage
- Monolithic codebase (~600 lines in main.cpp)

---

## Version History Summary

| Version | Date | Key Feature |
|---------|------|-------------|
| 2.0.0 | 2025-10-18 | Comprehensive bug fixes & code review |
| 1.5.0 | 2025-10-17 | PC workspace monitor integration |
| 1.4.0 | 2025-10-14 | Notification system |
| 1.3.0 | 2025-10-14 | MQTT/AWS IoT Core integration |
| 1.2.0 | 2025-10-12 | MUSIC emotion with audio detection |
| 1.1.0 | 2025-10-10 | Complex 51-frame animations |
| 1.0.0 | 2025-10-05 | Initial modular release |
| 0.9.0 | 2025-09-25 | Pre-release prototype |

## Notes

- All dates are in YYYY-MM-DD format
- Version numbering follows Semantic Versioning (SemVer)
- Breaking changes increment major version
- New features increment minor version
- Bug fixes increment patch version
