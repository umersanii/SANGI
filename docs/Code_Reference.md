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
void animateNotification(const char* title = "", const char* message = "");
void animateCoding();
void animateGitHubStats();
void resetAnimationState();
```

**Notification Animation Details**:
- **Total duration**: 4.3 seconds (86 frames @ 50ms/frame)
- **Longest phase**: Content display (52 frames = 2.6 seconds)
- **Title font**: Size 2 (large, prominent)
- **Message font**: Size 1 (smaller, readable)
- **Behavior**: Plays once, holds on final idle frame (no loop)

**Animation Sequence**:
1. Surprise reaction (6 frames)
2. Run off screen (7 frames)
3. Board slides in (6 frames)
4. Display notification content (52 frames) ⭐ longest
5. Board slides out (6 frames)
6. Return to calm idle state (9 frames)

**Coding Animation Details** (Matrix-inspired):
- **Theme**: Focused coding/hacker aesthetic with Matrix falling binary
- **Update rate**: 80ms per frame (slower for Matrix effect)
- **Visual elements**: 
  - Rectangular glasses on focused face
  - 6 streams of falling binary digits (0s and 1s)
  - Concentrated expression with furrowed eyebrows
  - Neutral/focused mouth
  - Occasional blink every 40 frames for realism
- **Binary streams**: 
  - 4 full-length streams (left/right edges)
  - 2 shorter streams (near face)
  - Speed: 2 pixels per frame
  - Auto-reset when off-screen
- **Behavior**: Continuous loop, streams perpetually fall

**GitHub Stats Animation Details**:
- **Theme**: Clean GitHub contribution graph (last 14 days)
- **Update rate**: 200ms per frame
- **Display layout**:
  - **Grid**: 2 weeks (columns) x 7 days (rows)
  - **Cell size**: 8x8 pixels (large, clearly visible)
  - **Spacing**: 2px horizontal, 1px vertical gaps
  - **Position**: Centered on 128x64 display
- **Visual contribution levels** (GitHub-style heat map):
  - **Level 0**: Empty outline (no contributions)
  - **Level 1**: Outline + center dot (1-3 contributions)
  - **Level 2**: Outline + half filled (4-6 contributions)
  - **Level 3**: Filled + small hole (7-9 contributions)
  - **Level 4**: Completely filled (10+ contributions)
- **Day labels**: S M T W T F S on left side
- **Data source**: Reads from `networkManager.getGitHubData()`
- **Fallback**: Shows "No GitHub data" if empty
- **Behavior**: Static display (no scrolling for clarity)

**Usage**:
```cpp
// Call continuously in loop
animationManager.animateMusic();
animationManager.animateCoding();
animationManager.animateGitHubStats();

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

// GitHub Contribution Data
void setGitHubContributions(const uint8_t contributions[52][7], int total, int streak, int longest, const char* user);
GitHubContributionData* getGitHubData();
bool hasGitHubData();
void clearGitHubData();

// GitHub Stats Data
void setGitHubStats(const char* user, int repos, int followers, int following,
                    int contributions, int commits, int prs, int issues, int stars);
GitHubStatsData* getGitHubStats();
bool hasGitHubStats();

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

// Handle GitHub contribution data
if (networkManager.hasGitHubData()) {
  GitHubContributionData* data = networkManager.getGitHubData();
  // Display contribution graph...
  // Access: data->contributions[week][day], data->totalContributions, etc.
}

// Handle GitHub stats
if (networkManager.hasGitHubStats()) {
  GitHubStatsData* stats = networkManager.getGitHubStats();
  // Display stats: stats->repos, stats->followers, stats->contributions, etc.
}

// Check workspace mode status
if (networkManager.isInWorkspaceMode()) {
  // MQTT active with valid SSID
}
```

**Offline Mode Behavior**:

SANGI operates in two modes:
1. **Workspace Mode**: MQTT connected with valid SSID - emotions controlled by workspace monitor
2. **Offline Mode**: No MQTT or SSID mismatch - autonomous emotion cycling with all 13 emotions

Offline mode triggers when:
- MQTT not connected, OR
- No valid MQTT message received for > 60 seconds (configurable: `MQTT_TIMEOUT_THRESHOLD`)
- SSID in message doesn't match current WiFi network

**Offline Emotion Cycling**:
- Random selection from all emotions every 20 seconds (`OFFLINE_EMOTION_INTERVAL`)
- Includes: IDLE, HAPPY, SLEEPY, EXCITED, SAD, ANGRY, CONFUSED, THINKING, LOVE, SURPRISED, DEAD, MUSIC, NOTIFICATION, GITHUB_STATS
- Excludes: BLINK (handled separately by blink interval system)
- When EMOTION_NOTIFICATION is randomly selected, displays random system info:
  - Battery status: "Battery Status / 3.85V (75%)"
  - System uptime: "System Uptime / 2h 15m 43s"
  - 50/50 random selection between battery and uptime

**GitHub Contribution Data Structure**:

`GitHubContributionData` struct (defined in `include/network.h`):
```cpp
struct GitHubContributionData {
  uint8_t contributions[52][7];  // 52 weeks x 7 days grid (0-4 scale)
  int totalContributions;        // Total count for the year
  int currentStreak;             // Current consecutive days
  int longestStreak;             // Longest streak this year
  char username[32];             // GitHub username
  bool dataLoaded;               // Data is valid
};
```

**GitHub Stats Data Structure**:

`GitHubStatsData` struct (defined in `include/network.h`):
```cpp
struct GitHubStatsData {
  char username[32];             // GitHub username
  int repos;                     // Public repositories
  int followers;                 // Follower count
  int following;                 // Following count
  int contributions;             // Total contributions (last year)
  int commits;                   // Total commits (last year)
  int prs;                       // Total pull requests
  int issues;                    // Total issues
  int stars;                     // Total stars received
  unsigned long timestamp;       // Last update time
  bool dataLoaded;               // Data is valid
};
```

**Contribution Level Scale** (matches GitHub):
- **0**: No contributions
- **1**: 1-3 contributions (light)
- **2**: 4-6 contributions (medium)
- **3**: 7-9 contributions (high)
- **4**: 10+ contributions (very high)

**Hardcoded Test Data**:
- Simulates 397 total contributions across the year
- Loaded at boot via `loadHardcodedGitHubData()`
- Real data will replace hardcoded values when Pi publishes to `sangi/github/commits`

**Offline Notification Generator**:
```cpp
void generateOfflineNotification();  // Called in main.cpp when offline mode picks NOTIFICATION
// Populates: offlineNotifTitle[32] and offlineNotifMessage[64]
```

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

GitHub Contribution Graph (`sangi/github/commits`):
```json
{
  "username": "umersanii",
  "total": 397,
  "current_streak": 15,
  "longest_streak": 45,
  "contributions": [
    [0,1,2,3,2,1,0],
    [1,2,3,4,3,2,1],
    ...
  ]
}
```
*Note: `contributions` array contains 52 weeks, each with 7 days (Sunday-Saturday).  
Values: 0=none, 1=1-3, 2=4-6, 3=7-9, 4=10+ contributions per day.*

GitHub Stats (`sangi/github/stats`):
```json
{
  "type": "github_stats",
  "username": "umersanii",
  "repos": 25,
  "followers": 42,
  "following": 15,
  "contributions": 1247,
  "commits": 856,
  "prs": 45,
  "issues": 23,
  "stars": 150,
  "timestamp": 1729180800
}
```
*Note: Stats are fetched from GitHub API every 5 minutes and automatically trigger `EMOTION_GITHUB_STATS` display.*

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

### Speaker Functions

**Header**: `include/speaker.h`  
**Source**: `src/main.cpp`

**Public Functions**:
```cpp
void playEmotionBeep(EmotionState emotion);  // Play emotion-specific beep

// Individual emotion beep patterns
void beepIdle();         // Calm, neutral single tone
void beepHappy();        // Cheerful ascending chirp
void beepSleepy();       // Slow descending yawn
void beepExcited();      // Rapid energetic bursts
void beepSad();          // Descending melancholic tones
void beepAngry();        // Aggressive harsh buzzing
void beepConfused();     // Wandering uncertain tones
void beepThinking();     // Thoughtful rhythmic pulses
void beepLove();         // Sweet romantic melody
void beepSurprised();    // Quick ascending gasp
void beepDead();         // Dramatic "game over" sound
void beepMusic();        // Musical notes pattern
void beepNotification(); // Attention-grabbing triple beep
```

**Usage**:
```cpp
// Automatically called when emotion changes (if ENABLE_EMOTION_BEEP is true)
// Manual call:
playEmotionBeep(EMOTION_HAPPY);

// Or call specific patterns:
beepHappy();
```

**Beep Patterns**:
Each emotion has a unique sound signature:
- **IDLE**: Single 800Hz tone (150ms) - neutral presence
- **HAPPY**: Ascending 600→900→1200Hz - cheerful chirp
- **SLEEPY**: Descending 700→500→300Hz - yawn-like
- **EXCITED**: 4x rapid 1400Hz bursts + 1600Hz peak - energetic
- **SAD**: Descending 600→450→350Hz - melancholic
- **ANGRY**: 5x alternating 1800/1600Hz - aggressive buzzing
- **CONFUSED**: Wandering 700→900→600→800Hz - uncertain
- **THINKING**: 3x rhythmic 1000Hz pulses - contemplative
- **LOVE**: Sweet melody 880→1047→1319→1047Hz (A-C-E-C)
- **SURPRISED**: Ascending 400→800→1400→1800Hz - gasp
- **DEAD**: Dramatic descent 800→600→400→200Hz - game over
- **MUSIC**: Musical notes 523→659→784→1047Hz (C-E-G-C)
- **NOTIFICATION**: Triple beep 1200→1200→1500Hz - alert
- **CODING**: Keyboard taps 1000→1100→950→1050→1000Hz + compile success 1300→1500Hz

**Behavior**:
- **Configurable volume** via `SPEAKER_VOLUME` (0-255, default 64 for stability)
- **Non-blocking**: Uses BeepManager for asynchronous playback
- Triggered automatically in `EmotionManager::setTargetEmotion()`
- Can be disabled by setting `ENABLE_EMOTION_BEEP false` in `config.h`
- No beep for EMOTION_BLINK (silent)

**Hardware**:
- Speaker connected to GPIO 9 (SPEAKER_PIN)
- Uses PWM channel 0 (SPEAKER_CHANNEL)
- Supports any passive speaker (8Ω or higher recommended)
- Maximum volume achieved via 100% duty cycle

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
  EMOTION_NOTIFICATION = 13,
  EMOTION_CODING = 14,
  EMOTION_GITHUB_STATS = 15
};
```

**Static vs Animated**:
- Static: IDLE, SAD, ANGRY, SURPRISED, CONFUSED, DEAD, BLINK
- Animated: SLEEPY, THINKING, EXCITED, HAPPY, LOVE, MUSIC, NOTIFICATION, CODING, GITHUB_STATS

## Configuration

**File**: `include/config.h`

**Hardware Pins**:
```cpp
#define I2C_SDA 6           // Fixed on ESP32-C3
#define I2C_SCL 7           // Fixed on ESP32-C3
#define BATTERY_PIN 2       // ADC1 channel
#define TOUCH_PIN 3         // Touch sensor
#define SPEAKER_PIN 10      // PWM audio output (GPIO10 - SAFE, GPIO9 causes USB/display conflicts!)
```

**CRITICAL - ESP32-C3 Safe GPIO Pins**:
- ✅ **Safe for general use**: GPIO4, GPIO5, GPIO8, GPIO10
- ❌ **AVOID GPIO9**: Conflicts with USB/boot, causes display shutdown
- ❌ **AVOID GPIO18/19**: USB data lines
- ⚠️ **GPIO6/7**: Reserved for I2C only (hardware-fixed)

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

**Speaker Volume**:
```cpp
#define SPEAKER_VOLUME 128  // Volume level (0-255, default 128 prevents power issues)
```

**Features**:
```cpp
#define ENABLE_MQTT true
#define DEBUG_MODE_ENABLED false
#define DEBUG_MODE_EMOTION EMOTION_IDLE
#define ENABLE_EMOTION_BEEP true  // Beep sound on emotion change
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
// Full animateNotification() plays 86-frame cinematic sequence:
//   - Surprise → run away → board slides in → content (2.6s) → board out → calm return
//   - Title: large font (size 2), Message: normal font (size 1)
//   - Plays once, holds on final idle frame
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

### Raspberry Pi Notification Service (Standalone 24/7)

**Location**: `pi-setup/`

Standalone systemd service for Raspberry Pi that monitors Discord, GitHub, and WhatsApp notifications 24/7 and forwards them to SANGI via MQTT.

**Architecture**:
- **notification_service.py** - Main orchestrator
- **venv/** - Python virtual environment with `--system-site-packages`
- **lib/notification_monitor.py** - D-Bus notification capture (Discord, WhatsApp)
- **lib/github_monitor.py** - GitHub API polling
- **lib/mqtt_publisher.py** - AWS IoT MQTT publisher

**Installation**:
```bash
cd pi-setup
chmod +x setup.sh
./setup.sh
```

The setup script:
1. Checks Python 3.7+ installation
2. Installs system dependencies (python3-gi, python3-dbus, build-essential)
3. Creates virtual environment with `--system-site-packages`
4. Installs AWS IoT SDK (awscrt, awsiotsdk) and requests
5. Creates config.json from template
6. Sets up certs/ and logs/ directories
7. Installs systemd service

**Virtual Environment Strategy**:
- Uses `--system-site-packages` to access system PyGObject and D-Bus
- Avoids PEP 668 restrictions (no `--break-system-packages` needed)
- Prevents complex PyGObject builds (requires girepository-2.0)
- System packages: python3-gi, python3-dbus
- Venv packages: awscrt>=0.16.0, awsiotsdk>=1.11.0, requests>=2.28.0

**Configuration** (`pi-setup/config.json`):
```json
{
  "mqtt": {
    "endpoint": "xxxxx-ats.iot.us-east-1.amazonaws.com",
    "client_id": "sangi-notification-monitor",
    "certificate_path": "./certs/cert.pem",
    "private_key_path": "./certs/private.key",
    "root_ca_path": "./certs/AmazonRootCA1.pem",
    "topic": "sangi/notification/push"
  },
  "notifications": {
    "discord": {
      "enabled": true,
      "monitor_method": "dbus"
    },
    "github": {
      "enabled": true,
      "token": "ghp_your_token_here",
      "username": "your_github_username",
      "check_interval": 60,
      "monitor_types": ["pull_requests", "issues", "mentions"]
    },
    "whatsapp": {
      "enabled": false,
      "monitor_method": "dbus"
    }
  },
  "filters": {
    "min_notification_interval": 5,
    "max_notifications_per_minute": 10
  }
}
```

**Service Management**:
```bash
# Start service
sudo systemctl start sangi-notification-monitor@$(whoami).service

# Stop service
sudo systemctl stop sangi-notification-monitor@$(whoami).service

# Check status
sudo systemctl status sangi-notification-monitor@$(whoami).service

# View logs (real-time)
journalctl -u sangi-notification-monitor@$(whoami).service -f

# Enable auto-start on boot
sudo systemctl enable sangi-notification-monitor@$(whoami).service
```

**Manual Testing**:
```bash
cd pi-setup
./venv/bin/python notification_service.py
```

**Verify Dependencies**:
```bash
cd pi-setup
./venv/bin/python -c "import gi; import dbus; import awsiotsdk; print('✓ All imports OK')"
```

**Post-Setup**:
1. Edit `pi-setup/config.json` with AWS IoT endpoint
2. Copy certificates to `pi-setup/certs/`:
   - AmazonRootCA1.pem
   - cert.pem
   - private.key
3. (Optional) Add GitHub token
4. Start the service

**Notification Types**:
- `discord` - Discord messages (simplified: username + "new message")
- `github` - Pull requests, issues, mentions
- `system` - System notifications
- `generic` - Fallback for unknown sources

> See `pi-setup/README.md` for detailed setup and troubleshooting

**Supported Notifications**:
- **Discord**: D-Bus capture from desktop app (username + "new message")
- **GitHub**: API polling for PRs, issues, mentions (requires token)
- **WhatsApp**: D-Bus capture from desktop app (contact + message preview)

**Testing**:
```bash
cd PC-setup/notification-service
python3 test_service.py
```

**Features**:
- Auto-start on boot via systemd
- Rate limiting (5s min interval, 10/min max)
- Comprehensive logging to file and journal
- Automatic reconnection on network issues
- Runs as non-root user

**GitHub Setup**:
1. Generate token at https://github.com/settings/tokens
2. Required scopes: `notifications`, `repo`
3. Add to `config.json`

**See Also**: `PC-setup/notification-service/README.md` for detailed documentation

def _on_notification(self, notif_type, title, message):
### Notification Capture (Standalone Pi service)

Desktop D-Bus notification capture has been split out to a standalone Raspberry Pi
service: `pi-notification-service` (path: `PC-setup/notification-service/`). This
service runs 24/7 on a Pi, captures Discord/WhatsApp desktop notifications via
D-Bus, polls GitHub, and forwards notifications to SANGI over MQTT.

Use the standalone service and its `README.md` for setup instructions and tests.

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

**Display Shuts Off When Speaker Connected**:
- **ROOT CAUSE**: GPIO9 on ESP32-C3 conflicts with USB/boot circuitry!
- **CRITICAL**: GPIO9 is NOT safe for general use on ESP32-C3
- **SOLUTION**: Move speaker to GPIO10 (now default in config.h)
- **Safe GPIO pins for ESP32-C3**: GPIO4, GPIO5, GPIO8, GPIO10
- **Avoid these GPIOs**: GPIO9 (USB), GPIO18/19 (USB), GPIO6/7 (I2C)

**Speaker Still Causing Display Issues**:
- **Power Issue**: Speaker draws too much current, causing voltage drop
- **Solution 1**: Reduce `SPEAKER_VOLUME` in `config.h` (now default 64, range 0-255)
- **Solution 2**: Add external power for speaker (separate 3.3V regulator)
- **Solution 3**: Add decoupling capacitor (100µF) near ESP32 VCC pin
- **Solution 4**: Use NPN transistor to drive speaker from separate power rail
- Start with `SPEAKER_VOLUME 32` if display still browns out with 4V battery
- Higher values = louder but more current draw

**How to Wire Speaker Properly**:
```
ESP32-C3 GPIO10 -----> Speaker (+) Red Wire
ESP32-C3 GND --------> Speaker (-) Black Wire

OR with transistor (recommended for stability):
ESP32-C3 GPIO10 -----> 1kΩ resistor -----> NPN Base (2N2222)
NPN Emitter --------> GND
NPN Collector ------> Speaker (-) Black
VCC (3.3V or 5V) ---> Speaker (+) Red
```

**Speaker Not Working After Display Fix**:
- Non-blocking BeepManager runs in background via `update()` in main loop
- Check `beepManager.update()` is being called every loop iteration
- Verify `beepManager.init()` was called in `setup()`
- No blocking `delay()` calls in beep patterns
- Display and speaker now operate independently
- Verify speaker is connected to GPIO10, not GPIO9
