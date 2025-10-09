# SANGI Hardware Wiring Guide

## Required Components

### Core Components:
- ✅ ESP32-C3 development board (AirM2M Core)
- ✅ SSD1306 OLED Display (128x64, I2C)
- ✅ LiPo battery (3.7V, 500-2000mAh recommended)
- ✅ USB-C cable for programming/charging

### Optional Components for Full Phase 1:
- ⚪ Capacitive touch sensor (TTP223 module) OR physical button
- ⚪ Voltage divider resistors (2x 10kΩ) for battery monitoring
- ⚪ Breadboard and jumper wires for prototyping

---

## Wiring Diagram

### OLED Display (I2C) - **REQUIRED**
```
OLED Display        ESP32-C3
────────────────────────────
VCC (3.3V)    →     3.3V
GND           →     GND
SDA           →     GPIO6
SCL           →     GPIO7
```

**I2C Address:** 0x3C (or 0x3D - auto-detected by code)

---

### Touch Sensor - **OPTIONAL for Phase 1**

#### Option A: TTP223 Capacitive Touch Module (Recommended)
```
TTP223 Module       ESP32-C3
────────────────────────────
VCC           →     3.3V
GND           →     GND
SIG/OUT       →     GPIO3
```

**Configuration:** Set module to active-LOW output mode

#### Option B: Simple Physical Button
```
Button (Normally Open)
────────────────────────────
One side      →     GPIO3
Other side    →     GND
```

**Internal pull-up enabled in code** - no external resistor needed

#### Option C: Bare Capacitive Touch Pad
```
Copper pad/foil → GPIO3 (direct connection)
```

**Note:** Requires different code (ESP32-C3 native touch API)

---

### Battery Monitoring - **OPTIONAL but Recommended**

#### If using 3.3V direct (USB-C only, no battery):
```
USB Power → Already monitored via GPIO2
```
Current code will read ~3.3V

#### If using 3.7-4.2V LiPo Battery:

**⚠️ IMPORTANT:** ESP32-C3 ADC max input is 3.3V!  
**Exceeding this will damage your board!**

**Voltage Divider Circuit Required:**
```
                    ┌──────────── LiPo+ (4.2V max)
                    │
                   ┌┴┐
                   │R│ 10kΩ
                   └┬┘
                    ├────────────→ GPIO2 (ADC input)
                   ┌┴┐
                   │R│ 10kΩ
                   └┬┘
                    │
                    └──────────── GND
```

**Calculation:**
- 4.2V battery ÷ 2 = 2.1V at GPIO2 ✅ Safe!
- 3.0V battery ÷ 2 = 1.5V at GPIO2 ✅ Safe!

**Code adjustment needed:**
In `main_refactored.cpp`, change line ~78 to:
```cpp
float voltage = (rawValue / 4095.0) * 3.3 * 2.0;  // Multiply by 2 for divider
```

#### Wiring with voltage divider:
```
LiPo Battery
────────────────────────────
VBAT+         →     10kΩ resistor → GPIO2
                              ↓
                           10kΩ resistor → GND
GND           →     ESP32-C3 GND
```

---

## Complete Minimal Setup (No Battery/Touch)

**Just want to test animations?** This is all you need:

```
Component         ESP32-C3 Pin
─────────────────────────────
OLED VCC     →    3.3V
OLED GND     →    GND
OLED SDA     →    GPIO6
OLED SCL     →    GPIO7
USB-C        →    For power & programming
```

**What works:**
- ✅ All emotion animations
- ✅ Automatic blinking
- ✅ Time-based emotion cycles
- ✅ State machine transitions
- ⚠️ Battery will read ~3.3V (USB voltage)
- ⚠️ Touch will not work (no sensor connected)
- ⚠️ Deep sleep will trigger but may not wake (no touch sensor)

---

## Full Phase 1 Setup (Everything)

```
Component         ESP32-C3 Pin       Notes
──────────────────────────────────────────────────────
OLED VCC     →    3.3V              Power from ESP32
OLED GND     →    GND               Common ground
OLED SDA     →    GPIO6             I2C data
OLED SCL     →    GPIO7             I2C clock

Touch VCC    →    3.3V              (if using TTP223)
Touch GND    →    GND               Common ground
Touch OUT    →    GPIO3             Active-LOW signal

Battery+     →    10kΩ → GPIO2      Voltage divider top
(divider)         10kΩ → GND        Voltage divider bottom
Battery-     →    GND               Common ground

USB-C        →    Programming/charging
```

---

## GPIO Pin Reference (ESP32-C3)

### Used by SANGI:
| GPIO | Function | Direction | Notes |
|------|----------|-----------|-------|
| 6    | I2C SDA  | Bidirectional | OLED communication |
| 7    | I2C SCL  | Output | OLED clock |
| 2    | Battery ADC | Input | Analog voltage reading |
| 3    | Touch Input | Input | Digital or capacitive |

### Available for Future Use:
| GPIO | Available? | Notes |
|------|------------|-------|
| 0    | ✅ Yes | Boot button on some boards |
| 1    | ✅ Yes | ADC capable |
| 4    | ✅ Yes | ADC capable |
| 5    | ✅ Yes | ADC capable |
| 8    | ✅ Yes | General purpose |
| 9    | ⚠️ Maybe | Used for flash on some boards |
| 10   | ⚠️ Maybe | Used for flash on some boards |

**Reserved by System:**
- GPIO18, 19: USB (D-, D+) - do not use!
- GPIO11-17: SPI Flash - do not use!

---

## Power Supply Options

### Option 1: USB-C Only (Tethered)
```
USB-C → ESP32-C3 → 3.3V regulator → Components
```
**Pros:** Simple, no battery concerns  
**Cons:** Not portable, always needs cable

### Option 2: LiPo Battery + Charging (Portable)
```
LiPo → Charge controller → ESP32-C3 → Components
         ↑
       USB-C (charging)
```
**Pros:** Portable, rechargeable  
**Cons:** Needs battery protection circuit

### Option 3: Both (Recommended)
```
USB-C ←→ Charge IC ←→ LiPo Battery
              ↓
          ESP32-C3
```
**Pros:** Can run on battery or USB  
**Best for:** Final robot design

---

## Safety Warnings ⚠️

### Battery Safety:
- ✅ **DO** use batteries with protection circuits
- ✅ **DO** use proper LiPo charger (TP4056 or similar)
- ✅ **DO** monitor battery voltage (don't discharge below 3.0V)
- ❌ **DON'T** exceed 3.3V on any GPIO pin
- ❌ **DON'T** short circuit battery terminals
- ❌ **DON'T** leave charging unattended initially

### ADC Protection:
- ✅ **ALWAYS** use voltage divider for >3.3V sources
- ✅ **TEST** voltage divider with multimeter before connecting
- ❌ **NEVER** connect 4.2V directly to GPIO2
- Expected voltage at GPIO2: **0V - 2.1V range** (with divider)

### General:
- ✅ Double-check wiring before powering on
- ✅ Test I2C address with scanner before assuming 0x3C
- ✅ Use common ground for all components
- ❌ Don't reverse power polarity

---

## Testing Checklist

### Before First Power-On:
- [ ] OLED VCC connected to 3.3V (NOT 5V)
- [ ] OLED GND connected to ESP32-C3 GND
- [ ] SDA/SCL on correct pins (GPIO6/7)
- [ ] No shorts between power and ground
- [ ] Battery voltage divider tested with multimeter
- [ ] Touch sensor (if used) connected properly

### After Power-On:
- [ ] ESP32-C3 LED indicator lights up
- [ ] Serial monitor shows boot messages
- [ ] I2C scan detects display at 0x3C or 0x3D
- [ ] OLED shows "SANGI Booting..." message
- [ ] Face animations appear on screen

### Functional Tests:
- [ ] Face changes every 30 seconds
- [ ] Automatic blinking works
- [ ] Touching GPIO3 triggers reaction (if sensor connected)
- [ ] Battery voltage appears in serial monitor
- [ ] Smooth transitions between emotions

---

## Troubleshooting

### "OLED allocation failed"
- Check VCC is 3.3V
- Verify I2C wiring (SDA=6, SCL=7)
- Try swapping SDA/SCL if board pinout differs
- Test I2C address 0x3D in code

### "No I2C devices found"
- Check physical connections
- Verify OLED is powered (backlight on?)
- Try different I2C address
- Check for damaged OLED module

### Battery reads 0V
- Verify voltage divider circuit
- Check GPIO2 connection
- Measure with multimeter first
- Confirm ADC resolution set to 12-bit

### Touch doesn't trigger
- Check GPIO3 is free (not used by bootloader)
- Test by connecting GPIO3 to GND directly
- Adjust TOUCH_THRESHOLD in code
- Verify sensor power supply

### Won't wake from sleep
- Confirm GPIO3 wake-up configured
- Check external pull-up resistor
- Test without deep sleep first
- Verify touch sensor stays LOW when triggered

---

## Recommended Development Setup

### Breadboard Prototyping:
1. Mount ESP32-C3 on breadboard
2. Connect OLED with jumper wires
3. Add touch sensor/button
4. Test without battery first (USB power)
5. Add voltage divider circuit
6. Test battery monitoring
7. Add LiPo battery last

### Permanent Assembly (After Testing):
1. Design custom PCB or use perfboard
2. Solder connections for reliability
3. Add enclosure with display cutout
4. Mount touch sensor on exterior
5. Secure battery with double-sided tape
6. Add access for USB-C charging

---

## Parts List & Suppliers

### Essential Components:
| Part | Specification | Quantity | Notes |
|------|---------------|----------|-------|
| ESP32-C3 Board | AirM2M Core or similar | 1 | Must have USB-C |
| OLED Display | SSD1306, 128x64, I2C | 1 | 0.96" or 1.3" size |
| Jumper Wires | Male-Female, 10cm | 6+ | For breadboard |

### Optional Components:
| Part | Specification | Quantity | Notes |
|------|---------------|----------|-------|
| LiPo Battery | 3.7V, 500-2000mAh | 1 | With JST connector |
| Touch Sensor | TTP223 module | 1 | Or use button |
| Button | 6x6mm tactile | 1 | Alternative to touch |
| Resistors | 10kΩ, 1/4W | 2 | For voltage divider |
| Charge Module | TP4056 | 1 | If battery used |
| Breadboard | 400 tie-points | 1 | For prototyping |

---

## Quick Start (Absolute Minimum)

**Want to test RIGHT NOW with minimal parts?**

### You Need:
1. ESP32-C3 board
2. SSD1306 OLED display
3. 4 jumper wires
4. USB-C cable

### Connections:
```
OLED → ESP32-C3
VCC  → 3.3V
GND  → GND
SDA  → GPIO6
SCL  → GPIO7
```

### Upload Code:
1. Open PlatformIO
2. Rename `main_refactored.cpp` → `main.cpp`
3. Click Upload
4. Watch the animations!

**Everything else (battery, touch) can be added later.**

---

## Next Hardware Steps

Once basic display works:

1. **Add Touch Input** (most fun!)
   - Connect TTP223 or button to GPIO3
   - Touch to see excited reactions

2. **Add Battery Monitoring** (most useful)
   - Build voltage divider circuit
   - Monitor battery level visually

3. **Add LiPo Battery** (portability!)
   - Install charge circuit
   - Test deep sleep behavior

4. **Design Enclosure** (polish!)
   - 3D print or laser cut case
   - Expose display and touch sensor
   - Add cutout for USB-C charging

---

## Hardware Questions?

Let me know:
- What components do you already have?
- Are you breadboarding or soldering?
- Do you have a LiPo battery ready?
- Need help with voltage divider circuit?
- Want help designing an enclosure?

**The code is ready - let's get your hardware connected!** 🔌
