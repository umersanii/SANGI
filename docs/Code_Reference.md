# SANGI Code Reference

Complete API reference, conventions, and code patterns for the SANGI robot codebase.

## Build Commands

```bash
# Build and flash
pio run --target upload

# Serial monitor (115200 baud)
pio device monitor

# Clean build
pio run --target clean

# Linux USB access
sudo chmod 666 /dev/ttyUSB0
```

## Manager Classes

### EmotionManager

**Header**: `include/emotion.h`  
**Source**: `src/emotion.cpp`

**Public Methods**:
```cpp
void init();
void setTargetEmotion(EmotionState emotion);
EmotionState getCurrentEmotion();
EmotionState getTargetEmotion();
bool isTransitionActive();
int getTransitionFrame();
void advanceTransition();
void completeTransition();
```

**Usage**:
```cpp
// Change emotion
emotionManager.setTargetEmotion(EMOTION_HAPPY);

// Check if transitioning
if (emotionManager.isTransitionActive()) {
  emotionManager.advanceTransition();
}

// Get current state
EmotionState current = emotionManager.getCurrentEmotion();
```

### DisplayManager

**Header**: `include/display.h`  
**Source**: `src/display.cpp`

**Public Methods**:
```cpp
void init();
void clearDisplay();
void updateDisplay();
void scanI2C();

// Static faces
void drawFace_Normal();
void drawFace_Happy();
void drawFace_Sad();
void drawFace_Angry();
void drawFace_Surprised();
void drawFace_Confused();
void drawFace_Dead();
void drawFace_Blink();
void drawFace_Notification(const char* title, const char* message);

// Transitions
void performTransition();
void performSleepyTransition(int transitionFrame, EmotionState targetEmotion);
void performNotificationTransition(int transitionFrame, EmotionState currentEmotion);

// Special
void displayBatmanLogo();
```

**Usage**:
```cpp
// Render sequence
displayManager.clearDisplay();
displayManager.drawFace_Happy();
displayManager.updateDisplay();

// Notification face (content box only, no peeking eyes)
displayManager.drawFace_Notification("Title", "Message text");

// Direct drawing (Adafruit GFX)
displayManager.display.fillCircle(x, y, radius, WHITE);
displayManager.display.drawLine(x1, y1, x2, y2, WHITE);
```

**Notes**:
- `drawFace_Notification()` displays only the content box (no peeking eyes)
- Peeking eyes appear only during `animateNotification()` full animation sequence
- Text content is automatically truncated to fit within double border with padding

### AnimationManager

**Header**: `include/animations.h`  
**Source**: `src/animations.cpp`

**Public Methods**:
```cpp
void init();
void animateSleepy();
void animateThinking();
void animateExcited();
void animateHappy();
void animateLove();
void animateAngry();
void animateSad();
void animateSurprised();
void animateMusic();
void animateNotification();
void resetAnimationState();
```

**Usage**:
```cpp
// Call continuously in loop
animationManager.animateMusic();

// Reset when changing emotions
animationManager.resetAnimationState();
```

**Animation Pattern**:
```cpp
void AnimationManager::animateEmotion() {
  static unsigned long lastFrame = 0;
  static int frameIndex = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastFrame > 30) {  // 30ms frame delay
    displayManager.clearDisplay();
    
    // Draw frame based on frameIndex
    switch(frameIndex) {
      case 0-15:   // Intro
      case 16-35:  // Main
      case 36-50:  // Outro
    }
    
    displayManager.updateDisplay();
    frameIndex = (frameIndex + 1) % 51;
    lastFrame = currentTime;
  }
}

// Music Animation Pattern (51 frames):
// Frames 0-5:   Eyes closing
// Frames 6-17:  Slow sway center → left
// Frames 18-29: Slow sway left → right
// Frames 30-41: Slow sway right → left
// Frames 42-50: Slow sway left → center
// Music notes float to top corners with dispersed pattern
```

### NetworkManager

**Header**: `include/network.h`  
**Source**: `src/network.cpp`

**Public Methods**:
```cpp
void init();
void update();
bool isConnected();

// Notifications
bool addNotification(NotificationType type, const char* title, const char* message);
Notification* getCurrentNotification();
void clearCurrentNotification();
bool hasNotifications();
int getNotificationCount();

// Offline mode
bool isInWorkspaceMode();
unsigned long getLastMQTTMessageTime();
```

**Usage**:
```cpp
// Check connection
if (networkManager.isConnected()) {
  networkManager.update();
}

// Handle notifications
if (networkManager.hasNotifications()) {
  Notification* notif = networkManager.getCurrentNotification();
  // Display notification...
  networkManager.clearCurrentNotification();
}

// Check workspace mode status
if (networkManager.isInWorkspaceMode()) {
  // MQTT active with valid SSID
}
```

**Offline Mode Behavior**:

SANGI operates in two modes:
1. **Workspace Mode**: MQTT connected with valid SSID - emotions controlled by workspace monitor
2. **Offline Mode**: No MQTT or SSID mismatch - autonomous emotion cycling

Offline mode triggers when:
- MQTT not connected, OR
- No valid MQTT message received for > 60 seconds (configurable: `MQTT_TIMEOUT_THRESHOLD`)
- SSID in message doesn't match current WiFi network

**MQTT Message Formats**:

Emotion control (`sangi/emotion/set`):
```json
{
  "emotion": 1,
  "source": "pc",
  "ssid": "YourWiFiNetwork",
  "timestamp": 1729180800
}
```

Notification (`sangi/notification/push`):
```json
{
  "type": "discord",
  "title": "username",
  "message": "new message",
  "ssid": "YourWiFiNetwork",
  "timestamp": 1729180800
}
```

**SSID Validation**:
- All MQTT messages from workspace monitor include the WiFi SSID
- ESP32 validates incoming SSID matches its connected network
- Prevents cross-network interference (e.g., neighbor's SANGI responding)
- Failed validation triggers offline mode

**Notification Types**:
- `discord` - Discord messages (simplified format: username + "new message")
- `system` - System notifications
- `generic` - General alerts

**Discord Notification Format**:
Discord notifications use a simplified format to avoid clutter:
- **Title**: Username (extracted from notification summary)
- **Message**: Always "new message"

Example: When "friend_username" sends a Discord message, SANGI displays:
```
┌────────────────────┐
│ friend_username    │
│ new message        │
└────────────────────┘
```

### BatteryManager

**Header**: `include/battery.h`  
**Source**: `src/battery.cpp`

**Public Methods**:
```cpp
void init();
float readVoltage();
int getBatteryPercentage();
EmotionState getBatteryBasedEmotion();
```

**Usage**:
```cpp
float voltage = batteryManager.readVoltage();
int percent = batteryManager.getBatteryPercentage();

if (percent < 20) {
  emotionManager.setTargetEmotion(EMOTION_SLEEPY);
}
```

### InputManager

**Header**: `include/input.h`  
**Source**: `src/input.cpp`

**Public Methods**:
```cpp
void init();
void handleTouchInteraction();
```

**Usage**:
```cpp
// Call in loop
inputManager.handleTouchInteraction();
```

## Emotion States

**Enum** (`include/emotion.h`):
```cpp
enum EmotionState {
  EMOTION_IDLE = 0,
  EMOTION_HAPPY = 1,
  EMOTION_SAD = 2,
  EMOTION_ANGRY = 3,
  EMOTION_SURPRISED = 4,
  EMOTION_CONFUSED = 5,
  EMOTION_SLEEPY = 6,
  EMOTION_THINKING = 7,
  EMOTION_EXCITED = 8,
  EMOTION_LOVE = 9,
  EMOTION_DEAD = 10,
  EMOTION_MUSIC = 11,
  EMOTION_BLINK = 12,
  EMOTION_NOTIFICATION = 13
};
```

**Static vs Animated**:
- Static: IDLE, SAD, ANGRY, SURPRISED, CONFUSED, DEAD, BLINK
- Animated: SLEEPY, THINKING, EXCITED, HAPPY, LOVE, MUSIC, NOTIFICATION

## Configuration

**File**: `include/config.h`

**Hardware Pins**:
```cpp
#define I2C_SDA 6           // Fixed on ESP32-C3
#define I2C_SCL 7           // Fixed on ESP32-C3
#define BATTERY_PIN 2       // ADC1 channel
#define TOUCH_PIN 3         // Touch sensor
```

**Display**:
```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
```

**Timing**:
```cpp
#define EMOTION_CHANGE_INTERVAL 30000  // 30s
#define FRAME_DELAY 30                 // 30ms (~20 FPS)
#define STATUS_PUBLISH_INTERVAL 30000  // 30s
#define DEBOUNCE_DELAY 500             // 500ms
```

**Features**:
```cpp
#define ENABLE_MQTT true
#define DEBUG_MODE_ENABLED false
#define DEBUG_MODE_EMOTION EMOTION_IDLE
```

**MQTT**:
```cpp
#define MQTT_TOPIC_EMOTION_SET "sangi/emotion/set"
#define MQTT_TOPIC_STATUS "sangi/status"
#define MQTT_TOPIC_BATTERY "sangi/battery"
#define MQTT_TOPIC_NOTIF "sangi/notification/push"
```

**Offline Mode**:
```cpp
#define MQTT_TIMEOUT_THRESHOLD 60000      // 60s without MQTT triggers offline mode
#define OFFLINE_EMOTION_INTERVAL 20000    // 20s between emotions in offline mode
```

## Secrets Configuration

**File**: `include/secrets.h` (gitignored)  
**Template**: `include/secrets.h.template`

```cpp
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"
#define AWS_IOT_ENDPOINT "xxxxx-ats.iot.region.amazonaws.com"
#define THINGNAME "SANGI_Robot"

static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
[Amazon Root CA 1]
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
[Device Certificate]
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
[Private Key]
-----END RSA PRIVATE KEY-----
)EOF";
```

## Code Conventions

**Naming**:
- `camelCase` for variables and functions
- `PascalCase` for classes and enums
- `UPPER_CASE` for #defines
- `snake_case` for file names

**Comments**:
```cpp
// Single-line comment

/* Multi-line
   comment block */

// ===== SECTION HEADER =====
```

**File Structure**:
```cpp
// Header guards
#ifndef MODULE_H
#define MODULE_H

// Includes
#include <Arduino.h>

// Forward declarations
class OtherClass;

// Constants
#define CONSTANT_NAME value

// Enums
enum EnumName { };

// Class declaration
class ClassName {
private:
  // Private members
  
public:
  // Public methods
};

// Global instance
extern ClassName instanceName;

#endif
```

## Common Patterns

### Emotion Transitions

**Standard Transition** (most emotions):
```cpp
// 7-frame blink transition sequence
// Frame 0: Current emotion
// Frame 1-3: Eyes closing
// Frame 4-6: Eyes opening to target emotion
emotionManager.setTargetEmotion(EMOTION_HAPPY);
```

**Sleepy Transition** (special handling):
```cpp
// Uses round mouth throughout transition
// Eyes lower gradually during closing/opening
emotionManager.setTargetEmotion(EMOTION_SLEEPY);
```

**Notification Transition** (animated sequence):
```cpp
// 7-frame surprise reaction sequence:
// Frame 0: Current emotion (idle)
// Frame 1: Eyes start widening (alert!)
// Frame 2: Eyes WIDE (startled!)
// Frame 3: Eyes squinting (preparing to run)
// Frame 4-5: Move right (running away)
// Frame 6: Notification appears
emotionManager.setTargetEmotion(EMOTION_NOTIFICATION);

// Note: Transition FROM notification uses standard blink
// Static drawFace_Notification() shows content box only (no peeking eyes)
// Peeking eyes only appear in animateNotification() full animation
```

**Implementation**:
```cpp
// In display.cpp - performTransition()
if (targetEmotion == EMOTION_NOTIFICATION) {
  performNotificationTransition(transitionFrame, currentEmotion);
  return;
}

if (currentEmotion == EMOTION_SLEEPY || targetEmotion == EMOTION_SLEEPY) {
  performSleepyTransition(transitionFrame, targetEmotion);
  return;
}

// Standard blink transition for all others
```

### Overflow-Safe Timing

```cpp
unsigned long currentMillis = millis();
bool overflow = currentMillis < lastTime;

if (overflow || (currentMillis - lastTime > interval)) {
  // Do something
  lastTime = currentMillis;
}
```

### Animation Frame Loop

```cpp
static unsigned long lastFrame = 0;
static int frameIndex = 0;

if (millis() - lastFrame > FRAME_DELAY) {
  // Draw frame
  frameIndex = (frameIndex + 1) % TOTAL_FRAMES;
  lastFrame = millis();
}
```

### Safe String Copy

```cpp
char buffer[32];
strncpy(buffer, input, 31);
buffer[31] = '\0';  // Ensure null termination
```

### Input Validation

```cpp
void setEmotion(EmotionState emotion) {
  if (emotion < 0 || emotion > EMOTION_NOTIFICATION) {
    Serial.println("ERROR: Invalid emotion");
    return;
  }
  // Safe to proceed
}
```

## Debugging

**Serial Output**:
```cpp
Serial.println("Message");
Serial.printf("Value: %d\n", value);
Serial.printf("Float: %.2f\n", floatValue);
```

**Display Debugging**:
```cpp
displayManager.display.setTextSize(1);
displayManager.display.setCursor(0, 0);
displayManager.display.printf("Debug: %d", value);
displayManager.updateDisplay();
```

**MQTT Debugging**:
```cpp
// Enable in network.cpp
client.setCallback(mqttCallback);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.printf("Received [%s]: ", topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
```

## Testing

**Animation Test Mode** (`main.cpp`):
```cpp
static const EmotionState testEmotions[] = {
  EMOTION_IDLE, EMOTION_HAPPY, EMOTION_SLEEPY,
  EMOTION_EXCITED, EMOTION_CONFUSED, EMOTION_THINKING,
  EMOTION_LOVE, EMOTION_ANGRY, EMOTION_SAD,
  EMOTION_SURPRISED, EMOTION_MUSIC
};

// Cycles every 10 seconds
```

**Debug Mode** (`config.h`):
```cpp
#define DEBUG_MODE_ENABLED true
#define DEBUG_MODE_EMOTION EMOTION_MUSIC
```

## PC Workspace Monitor

**Installation**:
```bash
cd PC-setup
./setup.sh
```

**Configuration**: `PC-setup/config.json`
```json
{
  "endpoint": "xxxxx-ats.iot.region.amazonaws.com",
  "client_id": "sangi-pc-monitor",
  "certificate_path": "/path/to/cert.pem",
  "private_key_path": "/path/to/private.key",
  "root_ca_path": "/path/to/AmazonRootCA1.pem",
  "watch_paths": ["~/workspace"]
}
```

**Run**:
```bash
python3 workspace_monitor.py
```

**Activity Thresholds**:
- High activity (>3 edits/min) → EXCITED
- Build process → THINKING
- Build success → LOVE/HAPPY
- Build failure → CONFUSED/ANGRY
- Audio playing → MUSIC
- Idle >10min → SLEEPY

### Notification Detector

**Module**: `PC-setup/lib/notification_detector.py`

The notification detector uses D-Bus to capture desktop notifications and forward them to SANGI via MQTT.

**Supported Sources**:
- Discord (simplified format)
- System notifications
- Generic alerts

**Discord Format**:
Discord notifications are automatically simplified:
- Extracts username from notification summary
- Sets message to "new message"
- Removes message content for privacy/simplicity

**Usage in workspace_monitor.py**:
```python
from lib import NotificationDetector

# Initialize with callback
notification_detector = NotificationDetector(
    logger=self.logger,
    callback=self._on_notification
)

# Callback handles MQTT publishing
def _on_notification(self, notif_type, title, message):
    payload = json.dumps({
        'type': notif_type,
        'title': title,
        'message': message,
        'timestamp': int(time.time())
    })
    self.mqtt_client.publish('sangi/notification/push', payload, 1)
```

**Testing Notifications**:
```bash
cd PC-setup
python3 test_notifications.py
```

Test script sends sample notifications:
- Discord (2 different usernames)
- System notification
- Generic alert

## Common Issues

**I2C Not Working**:
- ESP32-C3 uses fixed GPIO 6/7
- Check wiring: SDA→6, SCL→7
- Verify address: `displayManager.scanI2C()`

**MQTT Connection Failed**:
- Check WiFi credentials in `secrets.h`
- Verify AWS IoT endpoint format
- Ensure certificates are properly formatted
- Check NTP time sync

**Animation Frozen**:
- Verify `FRAME_DELAY` in `config.h`
- Check `static` variables are resetting
- Use modulo for frame index wrapping

**millis() Overflow**:
- Always use overflow detection pattern
- Never assume time always increases
- Test with long uptimes (>49 days)
