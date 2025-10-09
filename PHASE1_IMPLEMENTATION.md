# SANGI Phase 1 Implementation Complete! 🤖

## What's Been Implemented

I've created a **complete Phase 1 standalone companion robot** implementation in `src/main_refactored.cpp` with all the features from your technical specification.

---

## ✅ Features Completed

### 1. **Emotion State Machine**
- ✅ Enum-based state system with 11 emotions
- ✅ Clean state transitions with proper tracking
- ✅ Previous/current/target emotion management

### 2. **Smooth Transition Animations**
- ✅ 3-frame transition system (current → blink → target)
- ✅ Prevents jarring emotion switches
- ✅ 100ms delays for organic feel

### 3. **Time-Based Emotion Cycles**
- ✅ Morning (6AM-12PM): Happy/energetic
- ✅ Afternoon (12PM-6PM): Thinking/focused
- ✅ Evening (6PM-10PM): Idle/calm
- ✅ Night (10PM-6AM): Sleepy
- ✅ Uses `millis()` for relative time tracking

### 4. **Battery Voltage Monitoring**
- ✅ ADC reading on GPIO2 (configurable)
- ✅ Voltage-to-emotion mapping:
  - `<3.0V` → Dead (critical)
  - `<3.3V` → Sleepy (low battery)
  - `>3.3V` → Normal operation
- ✅ Battery status overrides other emotions

### 5. **Touch Sensor Input**
- ✅ GPIO3 configured for digital touch input
- ✅ Touch triggers Excited or Surprised reactions
- ✅ Debounce delay to prevent false triggers
- ✅ Updates last interaction timestamp

### 6. **Deep Sleep Power Management**
- ✅ Enters sleep after 5 minutes of no interaction
- ✅ Displays "Sleeping..." message before sleep
- ✅ Wake-on-touch configured (GPIO3)
- ✅ Sleepy face animation before deep sleep

### 7. **Autonomous Behavior**
- ✅ Emotion changes every 30 seconds
- ✅ Automatic blinking every 3 seconds
- ✅ Random emotion variations (33% chance)
- ✅ Battery-aware emotional responses

### 8. **Your Original Animations** (All Kept!)
- ✅ Normal/Idle
- ✅ Happy
- ✅ Sad
- ✅ Angry
- ✅ Love (heart eyes)
- ✅ Sleepy (with Zzz)
- ✅ Excited
- ✅ Confused
- ✅ Thinking (with thought bubbles)
- ✅ Dead/exhausted
- ✅ Surprised

---

## 📋 Hardware Configuration

### Current Pin Assignments:
```cpp
I2C_SDA: GPIO6
I2C_SCL: GPIO7
BATTERY_PIN: GPIO2 (ADC)
TOUCH_PIN: GPIO3 (Digital Input)
```

### Display:
- SSD1306 OLED (128x64)
- I2C Address: 0x3C
- Adafruit GFX library

---

## 🔧 Configuration Parameters (Easy to Adjust)

All timing and behavior settings are defined at the top:

```cpp
EMOTION_CHANGE_INTERVAL: 30000ms (30 sec)
BLINK_INTERVAL: 3000ms (3 sec)
SLEEP_TIMEOUT: 300000ms (5 min)
BATTERY_LOW_THRESHOLD: 3.3V
TOUCH_THRESHOLD: 40 (adjust for sensitivity)
```

---

## 🚀 How to Use

### Option 1: Test the New Code
1. Rename `main.cpp` to `main_old.cpp` (backup)
2. Rename `main_refactored.cpp` to `main.cpp`
3. Upload to your ESP32-C3
4. Open Serial Monitor (115200 baud)

### Option 2: Keep Both (Recommended for Testing)
The refactored code is in `main_refactored.cpp`. You can:
- Review it first
- Test specific features
- Merge changes gradually

---

## 🔍 What to Test

### 1. **Basic Operation**
- Does it boot with the "SANGI Booting..." screen?
- Does it show a happy face after boot?
- Does the I2C scanner find your display?

### 2. **Emotion State Machine**
- Wait 30 seconds - does emotion change?
- Are transitions smooth (blink between states)?
- Does it blink automatically every ~3 seconds?

### 3. **Touch Interaction**
- Connect GPIO3 to GND briefly (simulates touch)
- Does it react with Excited or Surprised face?
- Does the serial monitor show interaction detected?

### 4. **Battery Monitoring**
- Check Serial Monitor for battery voltage readings
- Are they reasonable? (should be ~3.3V if connected to USB)
- *Note: You may need a voltage divider if reading >3.3V LiPo*

### 5. **Deep Sleep** (Optional - causes reset)
- Wait 5 minutes without touching
- Does it show sleepy face then "Sleeping..." message?
- Touch GPIO3 to wake - does it restart?

### 6. **Serial Debug Output**
Check for these messages every 10 seconds:
```
Battery: 3.30V | Emotion: 1 | Uptime: 45s
```

---

## ⚠️ Important Notes & Adjustments Needed

### 1. **Battery Monitoring Requires Calibration**
Your current setup uses direct 3.3V. For proper LiPo monitoring:
- If using 3.7V-4.2V LiPo, add a voltage divider (2:1 ratio)
- Example: 10kΩ + 10kΩ resistors from VBAT to GND
- Measure midpoint with GPIO2
- Adjust voltage calculation in code

**Current code assumes direct 3.3V input. For LiPo battery:**
```cpp
float voltage = (rawValue / 4095.0) * 3.3 * 2.0;  // *2 for voltage divider
```

### 2. **Touch Sensor Configuration**
Current implementation uses simple digital input (GPIO3).

**For actual capacitive touch sensor:**
- TTP223 module → already outputs digital signal (current code works)
- ESP32-C3 built-in touch → requires different API (not yet implemented)

If you want ESP32-C3 native touch, let me know and I'll add that code.

### 3. **GPIO Pin Availability**
ESP32-C3 has limited GPIOs. Current usage:
- GPIO6, 7: I2C (OLED)
- GPIO2: Battery ADC
- GPIO3: Touch sensor

**Available for future use:**
- GPIO0, 1, 4, 5, 8, 9, 10 (check your board's constraints)

### 4. **Deep Sleep Wake-Up**
The code uses `esp_sleep_enable_ext0_wakeup()` for GPIO3.
- Requires GPIO3 to go LOW to wake
- May need external pull-up resistor
- Test wake-up behavior with your actual sensor

---

## 🎯 Next Steps (Your Choice)

### Immediate Testing:
1. **Upload and test basic functionality**
2. **Verify display and emotions work**
3. **Calibrate battery monitoring** (if using LiPo)
4. **Connect touch sensor and test interaction**
5. **Adjust timing constants** to your preference

### Hardware Integration:
1. **Add voltage divider** for battery monitoring (if needed)
2. **Connect capacitive touch sensor** to GPIO3
3. **Test deep sleep wake-up** functionality
4. **Fine-tune touch threshold** for sensitivity

### Code Refinement:
1. **Adjust emotion change frequency** (currently 30s)
2. **Modify time-based emotion mapping** to your schedule
3. **Add more random behaviors** for variety
4. **Tune transition timing** (currently 100ms/frame)

### Move to Phase 2:
Once Phase 1 is solid, we can add:
1. **Wi-Fi connectivity**
2. **HTTP REST server** for remote control
3. **Pi integration scripts**
4. **Network fallback handling**

---

## 🐛 Troubleshooting Guide

### Display doesn't work:
- Check I2C wiring (SDA=GPIO6, SCL=GPIO7)
- Try I2C address 0x3D instead of 0x3C
- Verify 3.3V and GND connections
- Check Serial Monitor for I2C scan results

### Emotions don't change:
- Check Serial Monitor - is it running the main loop?
- Verify `millis()` is incrementing
- Check if battery voltage is too low (forces DEAD emotion)

### Touch doesn't work:
- Confirm GPIO3 connection
- Try touching GPIO3 directly to GND
- Adjust `TOUCH_THRESHOLD` if using capacitive sensor
- Check Serial Monitor for interaction logs

### Battery voltage reads 0V:
- Verify GPIO2 is not used by something else
- Check ADC configuration (`analogReadResolution(12)`)
- May need voltage source connected to GPIO2

### Deep sleep doesn't trigger:
- Wait full 5 minutes without touching
- Check `SLEEP_TIMEOUT` value
- Disable sleep for testing (comment out `checkSleepConditions()`)

---

## 📊 Behavior Summary

**Priority order for emotion selection:**
1. **Battery critical (<3.0V)** → DEAD
2. **Battery low (<3.3V)** → SLEEPY  
3. **Touch detected** → EXCITED/SURPRISED
4. **Time-based cycle** → varies by hour
5. **Random variation** → 33% chance every 30s
6. **Automatic blink** → every 3 seconds

**State machine flow:**
```
[Current Emotion] → [User/System Event] → [Target Emotion]
                          ↓
                   [Transition Animation]
                   (current → blink → target)
                          ↓
                   [New Current Emotion]
```

---

## 💡 Key Improvements Over Original

### Original Code (Demo Mode):
- ❌ Loops through emotions sequentially
- ❌ No state machine
- ❌ No external input
- ❌ No battery awareness
- ❌ No power management
- ❌ Demo timing only

### Refactored Code (Autonomous Robot):
- ✅ State machine with intelligent selection
- ✅ Time-based emotion cycles
- ✅ Touch interaction
- ✅ Battery voltage monitoring
- ✅ Deep sleep power saving
- ✅ Smooth transitions
- ✅ Random behavioral variation
- ✅ Hierarchical emotion priority

---

## 🔄 File Structure

```
SANGI/
├── src/
│   ├── main.cpp              # Your original demo code
│   └── main_refactored.cpp   # New Phase 1 implementation
├── platformio.ini            # Build configuration (unchanged)
├── .github/
│   └── copilot.md           # Technical specification
└── PHASE1_IMPLEMENTATION.md  # This file!
```

---

## Questions to Answer Before Testing

1. **Do you have a LiPo battery connected?**
   - If yes → We need to add voltage divider circuit
   - If no → Battery monitoring will read USB voltage (~3.3V)

2. **What type of touch sensor?**
   - TTP223 capacitive module → Current code works!
   - Bare capacitive touch pad → Need different API
   - Physical button → Current code works!

3. **Which GPIO for battery/touch?**
   - Current: GPIO2 (battery), GPIO3 (touch)
   - Conflicts with your board? → Easy to change

4. **Preferred test approach?**
   - A) Upload immediately and debug issues
   - B) Review code first, test incrementally
   - C) Modify some settings before uploading

---

## Ready to Test? 🚀

Let me know:
1. If you want to review the code first
2. If you need help with hardware wiring
3. If you want to adjust any parameters before uploading
4. If you need help switching from `main.cpp` to `main_refactored.cpp`

**The code is complete and ready to upload!** Just need your hardware configuration confirmed.

---

*Phase 1 Complete: Standalone "Pocket Pet" Mode* ✨
