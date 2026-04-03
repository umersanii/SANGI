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
#define SPEAKER_PIN 5         // GPIO10 for PWM audio output (SAFE PIN - no boot conflicts)
#define SPEAKER_CHANNEL 0      // PWM channel (0-7 available)
#define SPEAKER_RESOLUTION 8   // 8-bit resolution (0-255)
#define SPEAKER_BASE_FREQ 2000 // Base frequency in Hz
#define SPEAKER_VOLUME 255      // Volume level (0-255) - ~50% duty cycle, safer for 8ohm speaker with series resistor

// Emotion change beep configuration
#define ENABLE_EMOTION_BEEP true  // Set to false to disable beep on emotion change

// ===== TIMING CONFIGURATION =====
#define EMOTION_CHANGE_INTERVAL_BASE 30000  // 30s base for autonomous cycling
#define BLINK_INTERVAL 3000  // 3 seconds
#define SLEEP_TIMEOUT 300000  // 5 minutes
#define HOUR_IN_MILLIS 3600000
#define LONG_PRESS_MS 600
#define DOUBLE_TAP_WINDOW_MS 300

// ===== FACE GRAMMAR =====
// Canonical neutral pose — every emotion deviates from these values.
#define FACE_EYE_LX   38    // Left eye center X
#define FACE_EYE_LY   28    // Left eye center Y
#define FACE_EYE_RX   90    // Right eye center X
#define FACE_EYE_RY   28    // Right eye center Y
#define FACE_EYE_W    24    // Eye width (fillRoundRect)
#define FACE_EYE_H    22    // Eye height (default)
#define FACE_EYE_R     7    // Eye corner radius
#define FACE_MOUTH_CX 64    // Mouth center X
#define FACE_MOUTH_Y  52    // Mouth top Y
#define FACE_MOUTH_W  14    // Mouth width (default)
#define FACE_MOUTH_H   5    // Mouth height (default)

// Legacy aliases (used by display.cpp transition code until migrated)
#define NEUTRAL_EYE_LEFT_X  FACE_EYE_LX
#define NEUTRAL_EYE_LEFT_Y  FACE_EYE_LY
#define NEUTRAL_EYE_RIGHT_X FACE_EYE_RX
#define NEUTRAL_EYE_RIGHT_Y FACE_EYE_RY
#define NEUTRAL_EYE_HEIGHT  FACE_EYE_H
#define NEUTRAL_MOUTH_X     (FACE_MOUTH_CX - FACE_MOUTH_W / 2)
#define NEUTRAL_MOUTH_Y     FACE_MOUTH_Y
#define NEUTRAL_MOUTH_WIDTH FACE_MOUTH_W
#define NEUTRAL_MOUTH_HEIGHT FACE_MOUTH_H

// ===== PERSONALITY CONFIGURATION =====
#define ATTENTION_STAGE0_MS 150000    // 2.5 min → NEEDY (soft nudge before sulk)
#define ATTENTION_STAGE1_MS 300000    // 5 min → BORED (base, ±20% jitter)
#define ATTENTION_STAGE2_MS 600000    // 10 min → SAD
#define ATTENTION_STAGE3_MS 750000    // 12.5 min → GRUMPY
#define ATTENTION_STAGE4_MS 900000    // 15 min → ANGRY
#define MOOD_DRIFT_INTERVAL_MS 120000 // 2 min between mood drift checks (base)
#define MICRO_EXPRESSION_CHANCE 15    // % chance per drift check to do a micro-expression
#define JITTER_PERCENT 20             // ±20% applied to all personality timings

// Warmth arc — rewarding frequent interaction with sustained positive bias
#define WARMTH_TOUCH_THRESHOLD  5        // touches within WARMTH_WINDOW_MS to activate
#define WARMTH_WINDOW_MS        600000   // 10 min rolling touch-count window
#define WARMTH_DRIFT_CYCLES     4        // drift cycles the warmth bias lasts

// Post-interaction glow — immediate positive burst after any touch
#define GLOW_DRIFT_CYCLES       2        // positive drift cycles after a touch

// Habituation — prevents getting stuck looping the same emotion
#define HABITUATION_THRESHOLD   3        // consecutive same drifts before forcing variety

// Multi-touch forgiveness — deep neglect requires effort to recover
#define FORGIVENESS_TOUCHES     3        // touches needed to forgive GRUMPY/ANGRY

// ===== DEBUG MODE =====
#define DEBUG_MODE_ENABLED true            // Set to true to enable debug mode
#define DEBUG_MODE_CYCLE off              // true = cycle all emotions; false = show only DEBUG_MODE_EMOTION
#define DEBUG_MODE_EMOTION EMOTION_SLEEPY  // Shown when DEBUG_MODE_CYCLE is false
#define DEBUG_CYCLE_INTERVAL_MS 10000      // ms to show each emotion before advancing

// ===== WIFI / WEB SERVER =====
#define WIFI_AP_SSID        "SANGI"   // AP network name (open, no password)
#define WIFI_AP_CHANNEL     1          // 2.4GHz ch1 — minimal BLE advertising overlap
#define WIFI_SERVER_PORT    80
#define WEB_MIN_FREE_HEAP   51200      // 50KB — warning threshold after WiFi init

// Optional STA WiFi for NTP time sync — leave SSID empty "" to skip
#define WIFI_STA_SSID        ""        // Home WiFi SSID (empty = AP-only mode)
#define WIFI_STA_PASSWORD    ""        // Home WiFi password
#define WIFI_STA_TIMEOUT_MS  10000     // Max ms to wait for STA connection
#define NTP_SERVER           "pool.ntp.org"
#define NTP_UTC_OFFSET_S     0         // UTC offset in seconds (e.g. 18000 for UTC+5)
#define NTP_DST_OFFSET_S     0         // DST offset (0 or 3600)

#endif // CONFIG_H
