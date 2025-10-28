#include <Arduino.h>
#include "config.h"
#include "emotion.h"
#include "display.h"
#include "animations.h"
#include "battery.h"
#include "input.h"
#include "network.h"
#include "speaker.h"

// ===== GLOBAL STATE =====
unsigned long bootTime = 0;

// Forward declaration for hardcoded GitHub data loading
void loadHardcodedGitHubData();

// ===== OFFLINE NOTIFICATION GENERATOR =====
// Stores generated notification data for offline mode
static char offlineNotifTitle[32] = "";
static char offlineNotifMessage[64] = "";

void generateOfflineNotification() {
  // Randomly pick between battery status and uptime
  if (random(0, 2) == 0) {
    // Battery status notification
    float voltage = batteryManager.readVoltage();
    int percentage = (int)((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100);
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    
    snprintf(offlineNotifTitle, sizeof(offlineNotifTitle), "Battery");
    snprintf(offlineNotifMessage, sizeof(offlineNotifMessage), "%.2fV (%d%%)", voltage, percentage);
  } else {
    // Uptime notification
    unsigned long uptimeSeconds = (millis() - bootTime) / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    
    snprintf(offlineNotifTitle, sizeof(offlineNotifTitle), "Uptime");
    snprintf(offlineNotifMessage, sizeof(offlineNotifMessage), "%luh %lum %lus", hours, minutes, seconds);
  }
}

// ===== SPEAKER TEST FUNCTIONS (DEPRECATED - Now using BeepManager) =====
// All beep functionality moved to speaker.cpp with non-blocking implementation

// ===== POWER MANAGEMENT =====
void checkSleepConditions() {
  // Disabled until touch sensor is connected - uncomment when ready
  /*
  unsigned long currentTime = millis();

  if (currentTime - inputManager.getLastInteraction() > SLEEP_TIMEOUT) {
    displayManager.drawFace_Sleepy();
    delay(1000);

    displayManager.clearDisplay();
    displayManager.getDisplay().setTextSize(1);
    displayManager.getDisplay().setTextColor(SSD1306_WHITE);
    displayManager.getDisplay().setCursor(25, 28);
    displayManager.getDisplay().println("Sleeping...");
    displayManager.updateDisplay();
    delay(1000);

    // ESP32-C3 uses GPIO wakeup instead of ext0
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)TOUCH_PIN, GPIO_INTR_LOW_LEVEL);
    esp_deep_sleep_start();
  }
  */
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(2000);  // Longer delay for USB-Serial to stabilize
  
  Serial.println();
  Serial.println();
  Serial.println(">>> ESP32 BOOT SUCCESSFUL <<<");
  Serial.flush();
  delay(100);
  Serial.println(">>> Serial communication working! <<<");
  Serial.flush();
  delay(100);
  Serial.println("=== SANGI Robot Initializing ===");
  Serial.flush();

  bootTime = millis();
  
  
  // Initialize random seed for offline mode
  randomSeed(analogRead(0) + millis());

  // Initialize all modules
  if (!displayManager.init()) {
    Serial.println("Display initialization failed!");
    for(;;);
  }

  emotionManager.init(bootTime);
  inputManager.init();
  inputManager.updateLastInteraction(bootTime);
  batteryManager.init();
  #if !DEBUG_MODE_ENABLED
    displayManager.showBootScreen();
  
  // Initialize non-blocking beep manager
  beepManager.init();
  
  // Initialize network (if enabled in config.h)
#if ENABLE_MQTT
  networkManager.init();
  
  // Run connectivity test
  Serial.println("\n>>> Running Network Diagnostics <<<");
  delay(500);
  networkManager.testConnectivity();
#else
  Serial.println("MQTT disabled - running in autonomous mode");
#endif

  // Load hardcoded commit history for testing (will be replaced by MQTT data in production)
  loadHardcodedGitHubData();

  // Show boot screen
//  displayManager.showBootScreen();

#if DEBUG_MODE_ENABLED
  Serial.println("=== DEBUG MODE ENABLED ===");
  Serial.printf("Showing only: %s\n\n", 
                DEBUG_MODE_EMOTION == EMOTION_DEAD ? "DEAD" :
                DEBUG_MODE_EMOTION == EMOTION_HAPPY ? "HAPPY" :
                DEBUG_MODE_EMOTION == EMOTION_SLEEPY ? "SLEEPY" :
                DEBUG_MODE_EMOTION == EMOTION_EXCITED ? "EXCITED" :
                DEBUG_MODE_EMOTION == EMOTION_CONFUSED ? "CONFUSED" :
                DEBUG_MODE_EMOTION == EMOTION_THINKING ? "THINKING" :
                DEBUG_MODE_EMOTION == EMOTION_LOVE ? "LOVE" :
                DEBUG_MODE_EMOTION == EMOTION_ANGRY ? "ANGRY" :
                DEBUG_MODE_EMOTION == EMOTION_SAD ? "SAD" :
                DEBUG_MODE_EMOTION == EMOTION_SURPRISED ? "SURPRISED" :
                DEBUG_MODE_EMOTION == EMOTION_MUSIC ? "MUSIC" :
                DEBUG_MODE_EMOTION == EMOTION_NOTIFICATION ? "NOTIFICATION" :
                DEBUG_MODE_EMOTION == EMOTION_CODING ? "CODING" :
                DEBUG_MODE_EMOTION == EMOTION_GITHUB_STATS ? "GITHUB_STATS" : "IDLE");
  emotionManager.setTargetEmotion(DEBUG_MODE_EMOTION);
#else
  // Ensure final face is rendered (only when not in debug mode)
  displayManager.drawEmotionFace(emotionManager.getCurrentEmotion());
  delay(1500);
#endif

  Serial.println("=== SANGI Ready! (ANIMATION TEST MODE) ===");
  Serial.println("Cycling through all animations...\n");
  
  // Run speaker test on boot
  delay(1000);
  //testSpeaker();
}

// ===== MAIN LOOP =====
void loop() {
  static unsigned long lastEmotionSwitch = 0;
  static int emotionIndex = 0;
  unsigned long currentTime = millis();

  // Update non-blocking beep manager (MUST be called every loop)
  beepManager.update();

  // Update network manager (handles MQTT if enabled)
#if ENABLE_MQTT
  networkManager.update();
#endif

  // Update emotion manager to process transitions
  emotionManager.update(currentTime);

#if !DEBUG_MODE_ENABLED
  // Array of all emotions to test (excluding BLINK only)
  static const EmotionState testEmotions[] = {
    EMOTION_IDLE,
    EMOTION_HAPPY,
    EMOTION_SLEEPY,
    EMOTION_EXCITED,
    EMOTION_SAD,
    EMOTION_ANGRY,
    EMOTION_CONFUSED,
    EMOTION_THINKING,
    EMOTION_LOVE,
    EMOTION_SURPRISED,
    EMOTION_DEAD,
    EMOTION_MUSIC,
    EMOTION_NOTIFICATION,
    EMOTION_GITHUB_STATS
  };
  static const int numEmotions = sizeof(testEmotions) / sizeof(testEmotions[0]);

  // Autonomous emotion cycling (only when MQTT is disabled or disconnected)
#if !ENABLE_MQTT
  // Cycle through test emotions at configured interval
  if (lastEmotionSwitch == 0 || currentTime - lastEmotionSwitch > EMOTION_CHANGE_INTERVAL) {
    EmotionState newEmotion = testEmotions[emotionIndex % numEmotions];
    emotionManager.setTargetEmotion(newEmotion);
    Serial.printf("Now testing emotion: %d (index %d)\n", newEmotion, emotionIndex);
    emotionIndex = (emotionIndex + 1) % numEmotions;
    lastEmotionSwitch = currentTime;
  }
#else
  // Check if in workspace mode based on recent MQTT messages
  bool inWorkspaceMode = networkManager.isInWorkspaceMode();
  unsigned long lastMQTTTime = networkManager.getLastMQTTMessageTime();
  unsigned long timeSinceLastMQTT = 0;
  
  // Calculate time since last MQTT message
  if (lastMQTTTime > 0) {
    // Handle millis() overflow
    if (currentTime >= lastMQTTTime) {
      timeSinceLastMQTT = currentTime - lastMQTTTime;
    } else {
      timeSinceLastMQTT = 0;  // Reset on overflow
    }
  }
  
  // Enter offline mode if:
  // 1. MQTT never connected (lastMQTTTime == 0), OR
  // 2. MQTT not currently connected, OR
  // 3. Timeout exceeded since last valid message
  bool offlineMode = (lastMQTTTime == 0) || 
                     !networkManager.isMQTTConnected() || 
                     (lastMQTTTime > 0 && timeSinceLastMQTT > MQTT_TIMEOUT_THRESHOLD);
  
  if (offlineMode) {
    // Offline autonomous mode - randomly pick emotions
    if (lastEmotionSwitch == 0 || currentTime - lastEmotionSwitch > OFFLINE_EMOTION_INTERVAL) {
      // Pick random emotion
      int randomIndex = random(0, numEmotions);
      EmotionState newEmotion = testEmotions[randomIndex];
      
      // Clear previous offline notification when changing emotions
      if (emotionManager.getCurrentEmotion() == EMOTION_NOTIFICATION && newEmotion != EMOTION_NOTIFICATION) {
        offlineNotifTitle[0] = '\0';
        offlineNotifMessage[0] = '\0';
      }
      
      // If notification emotion selected, generate offline notification content
      if (newEmotion == EMOTION_NOTIFICATION) {
        generateOfflineNotification();
        Serial.printf("[Offline] Generated notification: %s - %s\n", offlineNotifTitle, offlineNotifMessage);
      }
      
      emotionManager.setTargetEmotion(newEmotion);
      
      static bool offlineMsgShown = false;
      if (!offlineMsgShown) {
        Serial.println("📴 Offline mode - autonomous emotion cycling");
        offlineMsgShown = true;
      }
      
      Serial.printf("[Offline] Emotion: %d (random)\n", newEmotion);
      lastEmotionSwitch = currentTime;
    }
  } else {
    // Workspace mode active - MQTT controls emotions
    static bool workspaceMsgShown = false;
    static bool offlineMsgReset = false;
    
    if (inWorkspaceMode && !workspaceMsgShown) {
      Serial.println("💼 Workspace mode active");
      workspaceMsgShown = true;
      offlineMsgReset = false;  // Allow offline message to show again
      
      // Reset autonomous timer ONCE when entering workspace mode to prevent interference
      lastEmotionSwitch = currentTime;
    }
  }
#endif
#endif // !DEBUG_MODE_ENABLED

  // // For focused testing, set a specific emotion once
  // static bool emotionSet = false;
  // if (!emotionSet) {
  //   EmotionState newEmotion = testEmotions[11]; // Test MUSIC
  //   emotionManager.setTargetEmotion(newEmotion);
  //   Serial.printf("Now testing: %s\n", "MUSIC");
  //   emotionSet = true;
  // }

  // Handle touch input
  inputManager.handleTouchInteraction();

  // Handle transitions
  if (emotionManager.isTransitionActive()) {
    displayManager.performTransition();
  } else {
    // Use animated states for specific emotions
    switch(emotionManager.getCurrentEmotion()) {
      case EMOTION_SLEEPY:
        animationManager.animateSleepy();
        break;
      case EMOTION_THINKING:
        animationManager.animateThinking();
        break;
      case EMOTION_EXCITED:
        animationManager.animateExcited();
        break;
      case EMOTION_CONFUSED:
        animationManager.animateConfused();
        break;
      case EMOTION_HAPPY:
        animationManager.animateHappy();
        break;
      case EMOTION_LOVE:
        animationManager.animateLove();
        break;
      case EMOTION_ANGRY:
        animationManager.animateAngry();
        break;
      case EMOTION_SAD:
        animationManager.animateSad();
        break;
      case EMOTION_SURPRISED:
        animationManager.animateSurprised();
        break;
      case EMOTION_MUSIC:
        animationManager.animateMusic();
        break;
      case EMOTION_DEAD:
        animationManager.animateDead();
        break;
      case EMOTION_NOTIFICATION:
        {
          // Check for queued notifications from MQTT
          Notification* notif = networkManager.getCurrentNotification();
          if (notif != nullptr) {
            // Show MQTT notification with actual data
            animationManager.animateNotification(notif->title, notif->message);
            
            // After animation completes one cycle, clear it and check for next
            static unsigned long notifStartTime = 0;
            unsigned long currentMillis = millis();
            
            // Handle millis() overflow (occurs every ~49 days)
            bool overflow = currentMillis < notifStartTime;
            
            if (notifStartTime == 0) {
              notifStartTime = currentMillis;
            } else if (overflow || (currentMillis - notifStartTime > 4300)) {  // 86 frames * 50ms = 4.3s
              networkManager.clearCurrentNotification();
              notifStartTime = 0;
            }
          } else if (offlineNotifTitle[0] != '\0') {
            // Offline mode: show generated battery/uptime notification
            animationManager.animateNotification(offlineNotifTitle, offlineNotifMessage);
          } else {
            // Fallback: Generate offline notification if none exists
            generateOfflineNotification();
            animationManager.animateNotification(offlineNotifTitle, offlineNotifMessage);
          }
        }
        break;
      case EMOTION_CODING:
        animationManager.animateCoding();
        break;
      case EMOTION_GITHUB_STATS:
        animationManager.animateGitHubStats();
        break;
      default:
        // Static display for other emotions
        displayManager.drawEmotionFace(emotionManager.getCurrentEmotion());
        break;
    }
  }

  checkSleepConditions();

  // Debug output
  static unsigned long lastDebug = 0;
  unsigned long currentMillis = millis();
  
  // Handle millis() overflow
  bool debugOverflow = currentMillis < lastDebug;
  
  if (debugOverflow || (currentMillis - lastDebug > 10000)) {
    float voltage = batteryManager.readVoltage();
    unsigned long uptimeSeconds = (currentMillis - bootTime) / 1000;
    
    // Handle uptime overflow
    if (currentMillis < bootTime) {
      uptimeSeconds = currentMillis / 1000;  // Reset after overflow
    }
    
    Serial.printf("Battery: %.2fV | Emotion: %d | Uptime: %lus", 
                  voltage, emotionManager.getCurrentEmotion(), uptimeSeconds);
#if ENABLE_MQTT
    Serial.printf(" | MQTT: %s\n", networkManager.isMQTTConnected() ? "Connected" : "Disconnected");
#else
    Serial.println();
#endif
    lastDebug = currentMillis;
  }

  delay(50);
}
