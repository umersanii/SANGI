#ifndef CONFIG_H
#define CONFIG_H

// ===== HARDWARE CONFIGURATION =====
// OLED display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// I2C pins for ESP32-C3 (FIXED - cannot be changed on this chip)
#define I2C_SDA 6
#define I2C_SCL 7

// Battery monitoring (ADC)
#define BATTERY_PIN 2  // GPIO2 (ADC1_CH2) - adjust if needed
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_LOW_THRESHOLD 3.3

// Touch sensor configuration
#define TOUCH_PIN 3  // GPIO3 - adjust if needed
#define TOUCH_THRESHOLD 40

// Speaker configuration
// IMPORTANT: GPIO9 causes display issues on ESP32-C3 (conflicts with USB/boot)
// Safe GPIO options: GPIO4, GPIO5, GPIO8, GPIO10
#define SPEAKER_PIN 10         // GPIO10 for PWM audio output (SAFE PIN - no boot conflicts)
#define SPEAKER_CHANNEL 0      // PWM channel (0-7 available)
#define SPEAKER_RESOLUTION 8   // 8-bit resolution (0-255)
#define SPEAKER_BASE_FREQ 2000 // Base frequency in Hz
#define SPEAKER_VOLUME 255      // Volume level (0-255) - REDUCED to prevent power issues

// Emotion change beep configuration
#define ENABLE_EMOTION_BEEP true  // Set to false to disable beep on emotion change

// ===== TIMING CONFIGURATION =====
#define EMOTION_CHANGE_INTERVAL 30000  // 30 seconds
#define BLINK_INTERVAL 3000  // 3 seconds
#define SLEEP_TIMEOUT 300000  // 5 minutes
#define HOUR_IN_MILLIS 3600000

// ===== ANIMATION NEUTRAL STATE =====
// Standard neutral pose for smooth transitions between emotions
#define NEUTRAL_EYE_LEFT_X 40
#define NEUTRAL_EYE_LEFT_Y 28
#define NEUTRAL_EYE_RIGHT_X 88
#define NEUTRAL_EYE_RIGHT_Y 28
#define NEUTRAL_EYE_HEIGHT 18
#define NEUTRAL_MOUTH_X 58
#define NEUTRAL_MOUTH_Y 50
#define NEUTRAL_MOUTH_WIDTH 12
#define NEUTRAL_MOUTH_HEIGHT 5

// ===== NETWORK CONFIGURATION =====
// MQTT Topics for AWS IoT Core
#define MQTT_TOPIC_EMOTION_SET "sangi/emotion/set"        // Subscribe: receive emotion commands
#define MQTT_TOPIC_STATUS "sangi/status"                  // Publish: device status
#define MQTT_TOPIC_BATTERY "sangi/status/battery"         // Publish: battery status (level, voltage, charging)
#define MQTT_TOPIC_SYSTEM "sangi/status/system"           // Publish: system status (uptime, heap, rssi)
#define MQTT_TOPIC_EMOTION "sangi/status/emotion"         // Publish: current emotion state
#define MQTT_TOPIC_SERIAL_LOGS "sangi/logs/serial"        // Publish: buffered serial logs (every 5s)

// Workspace activity topics
#define MQTT_TOPIC_WORKSPACE_PC "workspace/pc/activity"   // Subscribe: PC activity data
#define MQTT_TOPIC_WORKSPACE_PI "workspace/pi/activity"   // Subscribe: Pi activity data
#define MQTT_TOPIC_PC_STATUS "workspace/pc/status"        // Subscribe: PC online/offline
#define MQTT_TOPIC_PI_STATUS "workspace/pi/status"        // Subscribe: Pi online/offline
#define MQTT_TOPIC_GITHUB_COMMITS "sangi/github/commits"  // Subscribe: GitHub commit history updates

// Network timing
#define MQTT_RECONNECT_INTERVAL 5000      // 5 seconds between reconnection attempts
#define STATUS_PUBLISH_INTERVAL 30000     // 30 seconds between status updates
#define NETWORK_TIMEOUT 10000              // 10 seconds WiFi connection timeout

// Workspace mode settings
#define WORKSPACE_MODE true                               // Enable workspace activity monitoring
#define WORKSPACE_ACTIVITY_TIMEOUT 30000                  // 30s timeout before considering device offline

// Time synchronization (NTP)
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 0                   // Adjust for your timezone (e.g., -28800 for PST)
#define DAYLIGHT_OFFSET_SEC 0              // Adjust for daylight saving

// Mode selection
#define ENABLE_MQTT true                   // Set to true to enable MQTT mode (requires secrets.h)

// ===== OFFLINE MODE CONFIGURATION =====
#define MQTT_TIMEOUT_THRESHOLD 60000       // ms without valid MQTT message before switching to offline mode (60s)
#define OFFLINE_EMOTION_INTERVAL 20000     // ms between emotion changes in offline autonomous mode (20s)

// ===== DEBUG MODE =====
#define DEBUG_MODE_ENABLED false            // Set to true to enable debug mode (shows only DEBUG_MODE_EMOTION)
#define DEBUG_MODE_EMOTION EMOTION_GITHUB_STATS  // Which emotion to show in debug mode

#endif // CONFIG_H
