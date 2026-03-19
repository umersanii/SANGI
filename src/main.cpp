// main.cpp — orchestration layer for SANGI robot.
// CHANGED in Phase 1:
//   - Emotion registry populated in registerEmotions()
//   - Callbacks wire EmotionManager to AnimationManager/BeepManager/NetworkManager
//     (no more cross-module #includes in those modules)
//   - Transition driven by return value from displayManager, not globals
//   - Debug emotion name uses registry instead of 15-arm ternary
//   - Fixed duplicate beepManager.init()
//   - Emotion cycling uses registry.getCyclable() instead of hardcoded array

#include <Arduino.h>
#include "config.h"
#include "emotion.h"
#include "emotion_registry.h"
#include "display.h"
#include "animations.h"
#include "battery.h"
#include "input.h"
#include "network.h"
#include "speaker.h"

// ===== GLOBAL STATE =====
unsigned long bootTime = 0;

// Forward declaration (defined in network.cpp)
void loadHardcodedGitHubData();

// ===== OFFLINE NOTIFICATION GENERATOR =====
static char offlineNotifTitle[32] = "";
static char offlineNotifMessage[64] = "";

void generateOfflineNotification() {
  if (random(0, 2) == 0) {
    float voltage = batteryManager.readVoltage();
    int percentage = (int)((voltage - BATTERY_MIN_VOLTAGE) /
                           (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE) * 100);
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    snprintf(offlineNotifTitle, sizeof(offlineNotifTitle), "Battery");
    snprintf(offlineNotifMessage, sizeof(offlineNotifMessage), "%.2fV (%d%%)",
             voltage, percentage);
  } else {
    unsigned long uptimeSeconds = (millis() - bootTime) / 1000;
    unsigned long hours = uptimeSeconds / 3600;
    unsigned long minutes = (uptimeSeconds % 3600) / 60;
    unsigned long seconds = uptimeSeconds % 60;
    snprintf(offlineNotifTitle, sizeof(offlineNotifTitle), "Uptime");
    snprintf(offlineNotifMessage, sizeof(offlineNotifMessage), "%luh %lum %lus",
             hours, minutes, seconds);
  }
}

// ===== CALLBACKS (wiring between decoupled modules) =====

void onTransitionComplete(EmotionState newEmotion) {
  animationManager.resetAnimation(newEmotion);
}

void onEmotionChange(EmotionState from, EmotionState to) {
#if ENABLE_MQTT
  char logMsg[64];
  snprintf(logMsg, sizeof(logMsg), "Emotion: %s -> %s",
           emotionRegistry.getName(from), emotionRegistry.getName(to));
  networkManager.logInfo(logMsg);
#endif
#if ENABLE_EMOTION_BEEP
  beepManager.queueEmotionBeep(to);
#endif
}

// ===== EMOTION REGISTRY SETUP =====
void registerEmotions() {
  // Static emotions
  emotionRegistry.add(
      {EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_BLINK, "BLINK", 1, 0, LOOP_RESTART, false, nullptr});

  // Standard animated emotions (51 frames @ 30ms)
  emotionRegistry.add(
      {EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_SLEEPY, "SLEEPY", 51, 30, LOOP_RESTART, false, nullptr});
  emotionRegistry.add(
      {EMOTION_EXCITED, "EXCITED", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_SAD, "SAD", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_ANGRY, "ANGRY", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_CONFUSED, "CONFUSED", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_THINKING, "THINKING", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_LOVE, "LOVE", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_SURPRISED, "SURPRISED", 51, 30, LOOP_RESTART, true, nullptr});
  emotionRegistry.add(
      {EMOTION_DEAD, "DEAD", 51, 30, LOOP_RESTART, false, nullptr});
  emotionRegistry.add(
      {EMOTION_MUSIC, "MUSIC", 51, 30, LOOP_RESTART, false, nullptr});

  // Special animations
  emotionRegistry.add(
      {EMOTION_NOTIFICATION, "NOTIFICATION", 86, 50, LOOP_ONCE, false,
       nullptr});
  emotionRegistry.add(
      {EMOTION_CODING, "CODING", 25, 30, LOOP_RESTART, false, nullptr});
  emotionRegistry.add({EMOTION_GITHUB_STATS, "GITHUB_STATS", 130, 30,
                       LOOP_RESTART, false, nullptr});
}

// ===== POWER MANAGEMENT =====
void checkSleepConditions() {
  // Disabled until touch sensor is connected
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n>>> ESP32 BOOT SUCCESSFUL <<<");
  Serial.flush();
  delay(100);
  Serial.println("=== SANGI Robot Initializing ===");
  Serial.flush();

  bootTime = millis();
  randomSeed(analogRead(0) + millis());

  // Register all emotions first (needed for name lookups during init)
  registerEmotions();

  // Initialize display
  if (!displayManager.init()) {
    Serial.println("Display initialization failed!");
    for (;;)
      ;
  }

  // Initialize emotion manager with callbacks
  emotionManager.init(bootTime);
  emotionManager.setOnTransitionComplete(onTransitionComplete);
  emotionManager.setOnEmotionChange(onEmotionChange);

  // Initialize other modules
  inputManager.init();
  inputManager.updateLastInteraction(bootTime);
  batteryManager.init();
  beepManager.init();

#if !DEBUG_MODE_ENABLED
  displayManager.showBootScreen();
#else
  Serial.println("Skipping boot screen in DEBUG MODE");
#endif

  // Initialize network (if enabled)
#if ENABLE_MQTT
  networkManager.init();

  Serial.println("\n>>> Running Network Diagnostics <<<");
  delay(500);
  networkManager.testConnectivity();

  networkManager.logInfo("SANGI Robot initialized successfully");
  networkManager.logInfo("Wireless serial logging active");
#else
  Serial.println("MQTT disabled - running in autonomous mode");
#endif

  // Load hardcoded commit history for testing
  loadHardcodedGitHubData();

#if DEBUG_MODE_ENABLED
  Serial.println("=== DEBUG MODE ENABLED ===");
  Serial.printf("Showing only: %s\n\n",
                emotionRegistry.getName(DEBUG_MODE_EMOTION));
  emotionManager.setTargetEmotion(DEBUG_MODE_EMOTION);
#else
  displayManager.drawEmotionFace(emotionManager.getCurrentEmotion());
  delay(1500);
#endif

  Serial.printf("=== SANGI Ready! (%d emotions registered) ===\n",
                emotionRegistry.count());
  Serial.println("Cycling through all animations...\n");
}

// ===== MAIN LOOP =====
void loop() {
  static unsigned long lastEmotionSwitch = 0;
  static int emotionIndex = 0;
  unsigned long currentTime = millis();

  // Update non-blocking beep manager
  beepManager.update();

  // Update network manager
#if ENABLE_MQTT
  networkManager.update();
#endif

  // Update emotion manager
  emotionManager.update(currentTime);

#if !DEBUG_MODE_ENABLED
  // Get cyclable emotions from registry (once)
  static EmotionState cyclableEmotions[EmotionRegistry::MAX_EMOTIONS];
  static int numCyclable = 0;
  static bool emotionsLoaded = false;

  if (!emotionsLoaded) {
    numCyclable = emotionRegistry.getCyclable(cyclableEmotions,
                                              EmotionRegistry::MAX_EMOTIONS);
    emotionsLoaded = true;
  }

#if !ENABLE_MQTT
  // Autonomous cycling when MQTT disabled
  if (lastEmotionSwitch == 0 ||
      currentTime - lastEmotionSwitch > EMOTION_CHANGE_INTERVAL) {
    EmotionState newEmotion = cyclableEmotions[emotionIndex % numCyclable];
    emotionManager.setTargetEmotion(newEmotion);
    Serial.printf("Testing: %s (%d)\n",
                  emotionRegistry.getName(newEmotion), newEmotion);
    emotionIndex = (emotionIndex + 1) % numCyclable;
    lastEmotionSwitch = currentTime;
  }
#else
  // Workspace mode / offline mode
  bool inWorkspaceMode = networkManager.isInWorkspaceMode();
  unsigned long lastMQTTTime = networkManager.getLastMQTTMessageTime();
  unsigned long timeSinceLastMQTT = 0;

  if (lastMQTTTime > 0) {
    if (currentTime >= lastMQTTTime) {
      timeSinceLastMQTT = currentTime - lastMQTTTime;
    } else {
      timeSinceLastMQTT = 0;
    }
  }

  bool offlineMode = (lastMQTTTime == 0) ||
                     !networkManager.isMQTTConnected() ||
                     (lastMQTTTime > 0 &&
                      timeSinceLastMQTT > MQTT_TIMEOUT_THRESHOLD);

  if (offlineMode) {
    // Offline autonomous mode - randomly pick emotions
    if (lastEmotionSwitch == 0 ||
        currentTime - lastEmotionSwitch > OFFLINE_EMOTION_INTERVAL) {
      int randomIndex = random(0, numCyclable);
      EmotionState newEmotion = cyclableEmotions[randomIndex];

      // Clear previous offline notification when changing emotions
      if (emotionManager.getCurrentEmotion() == EMOTION_NOTIFICATION &&
          newEmotion != EMOTION_NOTIFICATION) {
        offlineNotifTitle[0] = '\0';
        offlineNotifMessage[0] = '\0';
      }

      if (newEmotion == EMOTION_NOTIFICATION) {
        generateOfflineNotification();
        Serial.printf("[Offline] Generated notification: %s - %s\n",
                      offlineNotifTitle, offlineNotifMessage);
      }

      emotionManager.setTargetEmotion(newEmotion);

      static bool offlineMsgShown = false;
      if (!offlineMsgShown) {
        Serial.println("Offline mode - autonomous emotion cycling");
        offlineMsgShown = true;
      }

      Serial.printf("[Offline] Emotion: %s\n",
                    emotionRegistry.getName(newEmotion));
      lastEmotionSwitch = currentTime;
    }
  } else {
    // Workspace mode active - MQTT controls emotions
    static bool workspaceMsgShown = false;
    if (inWorkspaceMode && !workspaceMsgShown) {
      Serial.println("Workspace mode active");
      workspaceMsgShown = true;
      lastEmotionSwitch = currentTime;
    }
  }
#endif
#endif // !DEBUG_MODE_ENABLED

  // Handle touch input
  inputManager.handleTouchInteraction();

  // Handle transitions (decoupled: display returns action, main.cpp drives state)
  if (emotionManager.isTransitionActive()) {
    TransitionResult r = displayManager.performTransitionFrame(
        emotionManager.getTransitionFrame(),
        emotionManager.getCurrentEmotion(),
        emotionManager.getTargetEmotion());
    if (r == TR_COMPLETE) {
      emotionManager.completeTransition();
    } else {
      emotionManager.advanceTransition();
    }
  } else {
    // Render current emotion
    // (Phase 2 will replace this switch with registry-based dispatch)
    switch (emotionManager.getCurrentEmotion()) {
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
      case EMOTION_NOTIFICATION: {
        Notification* notif = networkManager.getCurrentNotification();
        if (notif != nullptr) {
          animationManager.animateNotification(notif->title, notif->message);

          static unsigned long notifStartTime = 0;
          unsigned long currentMillis = millis();
          bool overflow = currentMillis < notifStartTime;

          if (notifStartTime == 0) {
            notifStartTime = currentMillis;
          } else if (overflow ||
                     (currentMillis - notifStartTime > 4300)) {
            networkManager.clearCurrentNotification();
            notifStartTime = 0;
          }
        } else if (offlineNotifTitle[0] != '\0') {
          animationManager.animateNotification(offlineNotifTitle,
                                               offlineNotifMessage);
        } else {
          generateOfflineNotification();
          animationManager.animateNotification(offlineNotifTitle,
                                               offlineNotifMessage);
        }
      } break;
      case EMOTION_CODING:
        animationManager.animateCoding();
        break;
      case EMOTION_GITHUB_STATS:
        animationManager.animateGitHubStats();
        break;
      default:
        displayManager.drawEmotionFace(emotionManager.getCurrentEmotion());
        break;
    }
  }

  checkSleepConditions();

  // Debug output every 10 seconds
  static unsigned long lastDebug = 0;
  if (currentTime - lastDebug > 10000) {
    float voltage = batteryManager.readVoltage();
    unsigned long uptimeSeconds = (currentTime - bootTime) / 1000;
    if (currentTime < bootTime) uptimeSeconds = currentTime / 1000;

    Serial.printf(
        "Battery: %.2fV | Emotion: %s | Uptime: %lus", voltage,
        emotionRegistry.getName(emotionManager.getCurrentEmotion()),
        uptimeSeconds);
#if ENABLE_MQTT
    Serial.printf(" | MQTT: %s\n",
                  networkManager.isMQTTConnected() ? "Connected"
                                                   : "Disconnected");
#else
    Serial.println();
#endif
    lastDebug = currentTime;
  }

  delay(50);
}
