# SANGI - Custom Mochi Robot

A custom square-based Mochi robot with animated OLED expressions, built with ESP32-C3.

> 🔗 **GitHub**: [@umersanii](https://github.com/umersanii)

## 🤖 Project Overview

This project creates a Dasai Mochi-style robot with cute animated facial expressions displayed on an SSD1306 OLED screen. The robot features 11 different emotions with smooth blinking and eye movement animations.

## 🔧 Hardware

- **Microcontroller**: ESP32-C3
- **Display**: 0.96" SSD1306 OLED (128x64, I2C)
- **I2C Pins**: 
  - SDA: GPIO 6
  - SCL: GPIO 7
- **I2C Address**: 0x3C

## ✨ Features

### Animated Expressions
The robot displays 11 different emotions:

1. 😊 **Normal** - Calm resting face
2. 😄 **Happy** - Slightly squinted eyes with smile
3. 😲 **Surprised** - Wide eyes with open mouth
4. 😢 **Sad** - Normal eyes with frown
5. 😠 **Angry** - Narrowed eyes with angry eyebrows
6. 😍 **Love** - Heart-shaped eyes with big smile
7. 😴 **Sleepy** - Half-closed eyes with "Zzz"
8. 🤩 **Excited** - Wide eyes with sparkles and big smile
9. 🤔 **Thinking** - Eyes looking up with thought bubbles
10. 😕 **Confused** - Asymmetric eyes with wavy mouth
11. 💀 **Dead/Exhausted** - X eyes with tongue out

### Animations
- **Smooth Blinking** - Eyes gradually close and open
- **Look Around** - Eyes shift left and right
- **Natural Timing** - Realistic delays between expressions

## 📦 Dependencies

```ini
lib_deps = 
    adafruit/Adafruit GFX Library@^1.11.3
    adafruit/Adafruit SSD1306@^2.5.7
```

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

2. **Grant USB permissions** (Linux only):
```bash
sudo chmod 666 /dev/ttyUSB0
```

3. **Build and Upload**:
```bash
platformio run --target upload
```

4. **Monitor Serial Output** (optional):
```bash
platformio device monitor
```

### Hardware Wiring
See [HARDWARE_WIRING.md](HARDWARE_WIRING.md) for detailed wiring instructions.

## 📐 Future Plans

- [ ] Add motor control for movement
- [ ] Implement line following sensors
- [ ] Add Bluetooth/WiFi remote control
- [ ] Battery monitoring with visual indicators
- [ ] Sound/buzzer feedback
- [ ] Touch sensor integration for interaction
- [ ] 3D printed enclosure design

## 🎨 Design Philosophy

Simple geometric eyes (rounded rectangles) inspired by classic Dasai Mochi robots, prioritizing clarity and expressiveness on the small OLED display.

## 📁 Project Structure

```
SANGI/
├── src/
│   └── main.cpp              # Main robot code with state machine
├── include/                  # Header files
├── lib/                      # Custom libraries
├── test/                     # Unit tests
├── platformio.ini            # PlatformIO configuration
├── HARDWARE_WIRING.md        # Detailed wiring guide
├── PHASE1_IMPLEMENTATION.md  # Development roadmap
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