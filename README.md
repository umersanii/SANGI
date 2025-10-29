# SANGI - Square Animated Networked Gizmo Interface

A custom square-based Mochi robot with animated OLED expressions, built with ESP32-C3.

> 🔗 **GitHub**: [@umersanii](https://github.com/umersanii)

## 🤖 Project Overview

This project creates a Dasai M### Features

- 💬 **Discord Messages** - Personal account DM and mention monitoring every 5 minutes (selfbot ⚠️)
- 🐙 **GitHub** - API polling for PRs, issues, mentions
- 📊 **GitHub Stats** - Real-time profile statistics (contributions, repos, stars, followers) every 5 minutes
- 🔄 **Auto-start** - Runs as systemd service on boot
- 📊 **Rate limiting** - Prevents notification spam
- 📝 **Logging** - Comprehensive logs to file and journal
- 🐍 **Virtual Environment** - Isolated Python dependenciesrobot with cute animated facial expressions displayed on an SSD1306 OLED screen. The robot features 11 different emotions with smooth blinking and eye movement animations.

**Architecture**: The codebase uses a clean modular design with separate manager classes for display, emotions, animations, battery, and input handling. See the [Modular Architecture](#️-modular-architecture) section below for details.

## 🔧 Hardware

- **Microcontroller**: ESP32-C3
- **Display**: 0.96" SSD1306 OLED (128x64, I2C)
- **I2C Pins**: 
  - SDA: GPIO 6
  - SCL: GPIO 7
- **I2C Address**: 0x3C

## ✨ Features

### Animated Expressions
The robot displays 16 different emotions with smooth multi-frame animations:

1. 😊 **Normal** - Calm resting face
2. 😄 **Happy** - Slightly squinted eyes with smile
3. 😲 **Surprised** - Wide eyes with open mouth
4. 😢 **Sad** - Normal eyes with frown
5. 😠 **Angry** - Narrowed eyes with angry eyebrows
6. 😍 **Love** - Heart-shaped eyes with big smile
7. 😴 **Sleepy** - Half-closed eyes with "Zzz" animation (51 frames)
8. 🤩 **Excited** - Wide eyes with sparkles and big smile (51 frames)
9. 🤔 **Thinking** - Eyes looking up with thought bubbles (51 frames)
10. 😕 **Confused** - Asymmetric eyes with wavy mouth
11. 💀 **Dead** - X eyes forming with tongue sticking out (51 frames)
12. 🎵 **Music** - Closed eyes with smooth swaying motion and floating music notes (51 frames)
13. 🔔 **Notification** - Alert state with notification indicator (86 frames)
14. 🎯 **Focus** - Concentrated expression with enhanced attention
15. 💻 **Coding** - Matrix-style with thick glasses, squinted eyes, and falling binary digits (25 frames)
16. 📊 **GitHub Stats** - Dynamic stats display showing real-time GitHub activity (130 frames)

### Animations
- **Smooth Blinking** - Eyes gradually close and open (3-frame transition)
- **Look Around** - Eyes shift left and right
- **Complex Emotions** - Multi-frame sequences for sleepy, excited, thinking, music, coding, and dead emotions
- **Music Swaying** - Slow left-to-right humming motion with closed eyes (full cycle: 51 frames)
- **Floating Music Notes** - Dispersed notes moving to top-left and top-right corners
- **Matrix Binary Effect** - Falling 0s and 1s in 6 columns with synchronized speed (25 frames)
- **Fast Focus Blinks** - Quick double blinks for concentrated expression
- **X Eyes Formation** - Dead emotion features thick X eyes that form gradually (frames 9-15)
- **Tongue Animation** - Tongue extends and retracts smoothly (frames 16-50)
- **Ghost Effects** - Subtle floating particles during dead emotion hold phase
- **Natural Timing** - Consistent 30ms frame delay (~20 FPS) across all animations
- **Dynamic GitHub Stats** - Cycles through 8 stat screens (repos, followers, contributions, commits, PRs, issues, stars, summary)
- **Real-time API Integration** - Fetches live GitHub data via Raspberry Pi service every 5 minutes

### Debug Mode
- **Isolated Testing** - Set `DEBUG_MODE_ENABLED true` in `config.h` to test specific emotions
- **Quick Iteration** - Bypass state machine for rapid animation development
- **Configurable Target** - Use `DEBUG_MODE_EMOTION` to select which emotion to display
- **Serial Logging** - Detailed frame and state information via serial monitor

### IoT & Remote Control ✅ OPERATIONAL
- **MQTT Communication** - Remote emotion control via AWS IoT Core
- **Cloud Telemetry** - Real-time battery, emotion, and status updates
- **Autonomous Fallback** - Continues operation when network unavailable
- **Secure Connection** - Certificate-based AWS authentication (TLS 1.2)
- **Status**: Successfully deployed and tested (October 2025)

**MQTT Topics**:
- 📥 Subscribe: `sangi/emotion/set` - Remote emotion commands
- � Subscribe: `sangi/notification/push` - Notification messages
- 📥 Subscribe: `sangi/github/commits` - GitHub contribution heatmap data
- 📥 Subscribe: `sangi/github/stats` - GitHub profile statistics (real-time)
- �📤 Publish: `sangi/status` - Connection status
- 📤 Publish: `sangi/battery` - Battery telemetry (every 30s)
- 📤 Publish: `sangi/emotion/current` - Current emotion state (every 30s)

> 📡 See [MQTT_SETUP.md](docs/MQTT_SETUP.md) for full setup guide or [MQTT_QUICKSTART.md](docs/MQTT_QUICKSTART.md) for quick start.

## 📦 Dependencies

```ini
lib_deps = 
    adafruit/Adafruit GFX Library@^1.11.3
    adafruit/Adafruit SSD1306@^2.5.7
    knolleary/PubSubClient@^2.8        # MQTT client
    bblanchon/ArduinoJson@^6.21.3      # JSON parsing
```

> **Note**: MQTT libraries are optional. Set `ENABLE_MQTT false` in `config.h` to use autonomous mode only.

## 🚀 Quick Start

### Prerequisites
- PlatformIO installed in VS Code
- ESP32-C3 board connected via USB-C

### Setup

1. **Clone the repository**:
```bash
git clone https://github.com/umersanii/SANGI.git
cd SANGI
```

2. **Configure WiFi & MQTT** (optional):
   - Copy `include/secrets.h.template` to `include/secrets.h`
   - Add your WiFi credentials and AWS IoT certificates
   - See [docs/MQTT_SETUP.md](docs/MQTT_SETUP.md) for detailed instructions
   - Or set `ENABLE_MQTT false` in `config.h` for autonomous mode only

   **Quick MQTT Test** (after setup):
   ```bash
   # In AWS IoT MQTT test client, publish to: sangi/emotion/set
   {"emotion": 1}  # Makes SANGI happy! 😊
   ```

3. **Grant USB permissions** (Linux only):
```bash
sudo chmod 666 /dev/ttyUSB0
```

4. **Build and Upload**:
```bash
platformio run --target upload
```

5. **Monitor Serial Output** (optional):
```bash
platformio device monitor
```

### Testing Specific Animations

To test individual emotions during development:

1. Open `include/config.h`
2. Set `#define DEBUG_MODE_ENABLED true`
3. Set `#define DEBUG_MODE_EMOTION EMOTION_DEAD` (or any emotion)
4. Build and upload - the robot will display only that emotion

Available emotions: `EMOTION_NORMAL`, `EMOTION_HAPPY`, `EMOTION_SURPRISED`, `EMOTION_SAD`, `EMOTION_ANGRY`, `EMOTION_LOVE`, `EMOTION_SLEEPY`, `EMOTION_EXCITED`, `EMOTION_THINKING`, `EMOTION_CONFUSED`, `EMOTION_DEAD`, `EMOTION_MUSIC`, `EMOTION_NOTIFICATION`, `EMOTION_FOCUS`, `EMOTION_CODING`

### Hardware Wiring
See [HARDWARE_WIRING.md](HARDWARE_WIRING.md) for detailed wiring instructions.

## 🏗️ Modular Architecture

The codebase uses a clean, modular design with separate components for each functionality:

### Core Modules

- **`config.h`** - Hardware pin definitions and timing constants
- **`emotion.h/cpp`** - Emotion state management with `EmotionManager` class
- **`display.h/cpp`** - OLED display operations with `DisplayManager` class
- **`animations.h/cpp`** - Complex animation engine with `AnimationManager` class
- **`battery.h/cpp`** - Battery monitoring with `BatteryManager` class
- **`input.h/cpp`** - Touch sensor handling with `InputManager` class
- **`network.h/cpp`** - WiFi & MQTT communication with `NetworkManager` class
- **`main.cpp`** - Clean orchestration layer (~150 lines)

### Benefits

✅ **Maintainable**: Easy to locate and modify specific features  
✅ **Scalable**: Simple to add new emotions or sensors  
✅ **Testable**: Each module can be tested independently  
✅ **Readable**: Clear separation of concerns  

> See [COPILOT.md](COPILOT.md) for detailed refactoring documentation and [REFACTORING_SUMMARY.md](REFACTORING_SUMMARY.md) for module descriptions.

## 🔔 Notification Service (Raspberry Pi)

**NEW**: Standalone 24/7 notification monitoring service for Raspberry Pi!

Monitor Discord messages and GitHub activity and forward them to SANGI automatically.

### Quick Setup

```bash
cd pi-setup
chmod +x setup.sh
./setup.sh
```

The setup script will:
1. ✅ Check Python installation (3.7+)
2. ✅ Install system dependencies
3. ✅ Create virtual environment
4. ✅ Install AWS IoT SDK, Discord.py, and dependencies
5. ✅ Create `config.json` from template
6. ✅ Setup `certs/` and `logs/` directories
7. ✅ Install and configure systemd service

### Post-Setup Configuration

After running `setup.sh`, you need to:

1. **Configure AWS IoT endpoint** - Edit `pi-setup/config.json`:
   ```json
   {
     "mqtt": {
       "endpoint": "xxxxx-ats.iot.us-east-1.amazonaws.com",
       "client_id": "sangi-notification-monitor"
     }
   }
   ```

2. **Copy AWS IoT certificates** to `pi-setup/certs/`:
   ```bash
   cp ~/path/to/AmazonRootCA1.pem pi-setup/certs/
   cp ~/path/to/certificate.pem pi-setup/certs/cert.pem
   cp ~/path/to/private.key pi-setup/certs/private.key
   ```

3. **(Optional) Configure GitHub** - Add your token to `config.json`:
   ```json
   {
     "notifications": {
       "github": {
         "enabled": true,
         "token": "ghp_your_token_here",
         "username": "your_github_username"
       }
     },
     "github_stats": {
       "enabled": true,
       "token": "ghp_your_token_here",
       "username": "your_github_username",
       "poll_interval": 300
     }
   }
   ```

4. **(Optional) Configure Discord Messages** - ⚠️ WARNING: Violates Discord ToS!
   ```json
   {
     "discord_messages": {
       "enabled": false,
       "user_token": "YOUR_DISCORD_USER_TOKEN",
       "poll_interval": 300,
       "monitor_dms": true,
       "monitor_mentions": true
     }
   }
   ```
   > See [pi-setup/README.md](pi-setup/README.md) for how to obtain your user token.

5. **Start the service**:
   ```bash
   sudo systemctl start sangi-notification-monitor@$(whoami).service
   sudo systemctl enable sangi-notification-monitor@$(whoami).service  # Auto-start on boot
   ```

### Features

- 🎮 **Discord** - Desktop app notifications via D-Bus
- � **Discord Messages** - Personal account DM and mention monitoring every 5 minutes (selfbot)
- �🐙 **GitHub** - API polling for PRs, issues, mentions
- 📊 **GitHub Stats** - Real-time profile statistics (contributions, repos, stars, followers) every 5 minutes
- 💬 **WhatsApp** - Desktop app notifications via D-Bus
- 🔄 **Auto-start** - Runs as systemd service on boot
- 📊 **Rate limiting** - Prevents notification spam
- 📝 **Logging** - Comprehensive logs to file and journal
- 🐍 **Virtual Environment** - Isolated Python dependencies with system PyGObject/D-Bus access

### Service Management

```bash
sudo systemctl start sangi-notification-monitor@$(whoami).service    # Start
sudo systemctl stop sangi-notification-monitor@$(whoami).service     # Stop
sudo systemctl status sangi-notification-monitor@$(whoami).service   # Status
journalctl -u sangi-notification-monitor@$(whoami).service -f        # Logs
```

### Troubleshooting

**Test the service manually**:
```bash
cd pi-setup
./venv/bin/python notification_service.py
```

**Check Python dependencies**:
```bash
cd pi-setup
./venv/bin/python -c "import gi; import dbus; import awsiotsdk; print('✓ All imports OK')"
```

**Verify certificates**:
```bash
ls -l pi-setup/certs/
# Should show: AmazonRootCA1.pem, cert.pem, private.key
```

> 📚 See [pi-setup/README.md](pi-setup/README.md) for detailed documentation

## �📐 Future Plans

- [x] ~~Add Bluetooth/WiFi remote control~~ **✅ COMPLETED: MQTT via AWS IoT Core (Oct 2025)**
- [x] ~~Implement cloud telemetry~~ **✅ COMPLETED: Real-time battery & emotion updates**
- [x] ~~Battery monitoring with visual indicators~~ **✅ COMPLETED: Published via MQTT**
- [x] ~~24/7 notification monitoring~~ **✅ COMPLETED: Standalone service for Pi (Oct 2025)**
- [ ] Sound/buzzer feedback
- [ ] Touch sensor integration for interaction
- [ ] Home Assistant / Node-RED integration
- [ ] Web dashboard for MQTT control

## 🎨 Design Philosophy

Simple geometric eyes (rounded rectangles) inspired by classic Dasai Mochi robots, prioritizing clarity and expressiveness on the small OLED display.

## 📁 Project Structure

```
SANGI/
├── src/
│   ├── main.cpp              # Main orchestration layer
│   ├── emotion.cpp           # Emotion state management
│   ├── display.cpp           # Display operations
│   ├── animations.cpp        # Animation engine
│   ├── battery.cpp           # Battery monitoring
│   ├── input.cpp             # Input handling
│   └── network.cpp           # WiFi & MQTT communication
├── include/
│   ├── config.h              # Hardware & timing configuration
│   ├── emotion.h             # Emotion manager interface
│   ├── display.h             # Display manager interface
│   ├── animations.h          # Animation manager interface
│   ├── battery.h             # Battery manager interface
│   ├── input.h               # Input manager interface
│   ├── network.h             # Network manager interface
│   └── secrets.h.template    # Template for AWS IoT credentials
├── lib/                      # Custom libraries
├── test/                     # Unit tests
├── platformio.ini            # PlatformIO configuration
├── HARDWARE_WIRING.md        # Detailed wiring guide
├── MQTT_SETUP.md             # Complete MQTT/AWS IoT setup guide
├── MQTT_QUICKSTART.md        # Quick MQTT setup reference
├── PHASE1_IMPLEMENTATION.md  # Development roadmap
├── REFACTORING_SUMMARY.md    # Module architecture details
├── COPILOT.md                # AI-assisted development log
└── README.md                 # This file
```

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests
- Improve documentation

## 📄 License

This project is open source and available for personal and educational use.

## 👤 Author

**Umer Sani**
- GitHub: [@umersanii](https://github.com/umersanii)

---

**Created**: October 2025  
**Platform**: PlatformIO + ESP32-C3  
**Framework**: Arduino


---

```lsof /dev/ttyUSB1 2>/dev/null | grep -v COMMAND```
to check where the port is active
``` kill <port>```

```~/.platformio/penv/bin/platformio device monitor --port /dev/ttyUSB0 --baud 115200```