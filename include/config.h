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
#define EMOTION_CHANGE_INTERVAL_BASE 30000  // 30s base for autonomous cycling
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

// ===== DEBUG MODE =====
#define DEBUG_MODE_ENABLED false            // Set to true to enable debug mode (shows only DEBUG_MODE_EMOTION)
#define DEBUG_MODE_EMOTION EMOTION_HAPPY    // Which emotion to show in debug mode

#endif // CONFIG_H
