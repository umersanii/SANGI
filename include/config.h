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
#define MQTT_TOPIC_BATTERY "sangi/battery"                // Publish: battery voltage
#define MQTT_TOPIC_UPTIME "sangi/uptime"                  // Publish: uptime in seconds

// Workspace activity topics
#define MQTT_TOPIC_WORKSPACE_PC "workspace/pc/activity"   // Subscribe: PC activity data
#define MQTT_TOPIC_WORKSPACE_PI "workspace/pi/activity"   // Subscribe: Pi activity data
#define MQTT_TOPIC_PC_STATUS "workspace/pc/status"        // Subscribe: PC online/offline
#define MQTT_TOPIC_PI_STATUS "workspace/pi/status"        // Subscribe: Pi online/offline

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
#define ENABLE_MQTT false                   // Set to true to enable MQTT mode (requires secrets.h)

// ===== DEBUG MODE =====
#define DEBUG_MODE_ENABLED true            // Set to true to enable debug mode (shows only DEBUG_MODE_EMOTION)
#define DEBUG_MODE_EMOTION EMOTION_MUSIC   // Which emotion to show in debug mode

#endif // CONFIG_H
