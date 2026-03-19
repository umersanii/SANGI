// main.cpp — orchestration layer for SANGI robot.
// CHANGED in Phase 2:
//   - DrawFrameFn pointers wired into registerEmotions()
//   - loop() switch replaced with animationManager.tick()
//   - All animation rendering goes through emotion_draws.cpp via registry
// CHANGED in Phase 3:
//   - MQTT topic handlers registered via mqttManager.subscribe()
//   - NetworkManager decomposed into MqttManager + NotificationQueue + GitHubDataStore
//   - InputManager decoupled via touch callback
//   - No more if/else topic chain in network.cpp

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "emotion.h"
#include "emotion_registry.h"
#include "emotion_draws.h"
#include "display.h"
#include "animations.h"
#include "battery.h"
#include "input.h"
#include "network.h"
#include "speaker.h"

// ===== GLOBAL STATE =====
unsigned long bootTime = 0;

// Forward declaration (defined in network.cpp, debug only)
#if DEBUG_MODE_ENABLED
void loadHardcodedGitHubData();
#endif

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

// ===== TOUCH CALLBACK =====
void onTouch(unsigned long currentTime) {
  int r = random(0, 100);
  if (r < 50) {
    emotionManager.setTargetEmotion(EMOTION_EXCITED);
  } else {
    emotionManager.setTargetEmotion(EMOTION_SURPRISED);
  }
}

// ===== AUTONOMOUS EMOTION SELECTION =====
// Consolidates offline/workspace/autonomous mode logic out of loop().
// Returns the emotion to set, or (EmotionState)-1 if MQTT is in control.
static EmotionState selectAutonomousEmotion(unsigned long currentTime,
                                             unsigned long& lastEmotionSwitch,
                                             int& emotionIndex,
                                             const EmotionState* cyclable,
                                             int numCyclable) {
#if !ENABLE_MQTT
  // No MQTT — sequential cycling
  if (lastEmotionSwitch == 0 ||
      currentTime - lastEmotionSwitch > EMOTION_CHANGE_INTERVAL) {
    EmotionState e = cyclable[emotionIndex % numCyclable];
    emotionIndex = (emotionIndex + 1) % numCyclable;
    lastEmotionSwitch = currentTime;
    Serial.printf("Testing: %s (%d)\n", emotionRegistry.getName(e), e);
    return e;
  }
  return (EmotionState)-1;
#else
  bool inWorkspaceMode = networkManager.isInWorkspaceMode();
  unsigned long lastMQTTTime = networkManager.getLastMQTTMessageTime();
  unsigned long timeSinceLastMQTT = 0;

  if (lastMQTTTime > 0 && currentTime >= lastMQTTTime) {
    timeSinceLastMQTT = currentTime - lastMQTTTime;
  }

  bool offlineMode = (lastMQTTTime == 0) ||
                     !mqttManager.isConnected() ||
                     (lastMQTTTime > 0 && timeSinceLastMQTT > MQTT_TIMEOUT_THRESHOLD);

  if (offlineMode) {
    if (lastEmotionSwitch == 0 ||
        currentTime - lastEmotionSwitch > OFFLINE_EMOTION_INTERVAL) {
      int randomIndex = random(0, numCyclable);
      EmotionState e = cyclable[randomIndex];

      if (emotionManager.getCurrentEmotion() == EMOTION_NOTIFICATION &&
          e != EMOTION_NOTIFICATION) {
        offlineNotifTitle[0] = '\0';
        offlineNotifMessage[0] = '\0';
      }

      if (e == EMOTION_NOTIFICATION) {
        generateOfflineNotification();
        Serial.printf("[Offline] Generated notification: %s - %s\n",
                      offlineNotifTitle, offlineNotifMessage);
      }

      static bool offlineMsgShown = false;
      if (!offlineMsgShown) {
        Serial.println("Offline mode - autonomous emotion cycling");
        offlineMsgShown = true;
      }

      Serial.printf("[Offline] Emotion: %s\n", emotionRegistry.getName(e));
      lastEmotionSwitch = currentTime;
      return e;
    }
  } else {
    static bool workspaceMsgShown = false;
    if (inWorkspaceMode && !workspaceMsgShown) {
      Serial.println("Workspace mode active");
      workspaceMsgShown = true;
      lastEmotionSwitch = currentTime;
    }
  }

  return (EmotionState)-1;  // MQTT in control
#endif
}

// ===== MQTT TOPIC HANDLERS =====

void handleEmotionSet(const char* topic, const char* payload) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return;

  // Validate SSID if present
  if (doc.containsKey("ssid")) {
    const char* receivedSSID = doc["ssid"];
    if (!networkManager.validateSSID(receivedSSID)) {
      Serial.println("SSID validation failed - message ignored");
      networkManager.setWorkspaceMode(false);
      return;
    }
    networkManager.setWorkspaceMode(true);
    networkManager.updateLastMQTTMessageTime();
  }

  if (doc.containsKey("emotion")) {
    int emotionValue = doc["emotion"];

    Serial.printf(">>> MQTT EMOTION: %d <<<\n", emotionValue);

    if (emotionValue >= EMOTION_IDLE && emotionValue <= EMOTION_GITHUB_STATS) {
      emotionManager.setTargetEmotion((EmotionState)emotionValue);
      Serial.printf("Emotion set to: %d\n", emotionValue);
    } else {
      Serial.printf("Invalid emotion: %d (valid range: 0-15)\n", emotionValue);
    }
  } else {
    Serial.println("Missing 'emotion' field in JSON");
  }
}

void handleNotificationPush(const char* topic, const char* payload) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, payload)) return;

  // Validate SSID if present
  if (doc.containsKey("ssid")) {
    const char* receivedSSID = doc["ssid"];
    if (!networkManager.validateSSID(receivedSSID)) {
      Serial.println("SSID validation failed - message ignored");
      networkManager.setWorkspaceMode(false);
      return;
    }
    networkManager.setWorkspaceMode(true);
    networkManager.updateLastMQTTMessageTime();
  }

  NotificationType notifType = NOTIFY_GENERIC;
  const char* title = "";
  const char* message = "";

  if (doc.containsKey("type")) {
    const char* typeStr = doc["type"];
    if (strcmp(typeStr, "discord") == 0) notifType = NOTIFY_DISCORD;
    else if (strcmp(typeStr, "slack") == 0) notifType = NOTIFY_SLACK;
    else if (strcmp(typeStr, "email") == 0) notifType = NOTIFY_EMAIL;
    else if (strcmp(typeStr, "github") == 0) notifType = NOTIFY_GITHUB;
    else if (strcmp(typeStr, "calendar") == 0) notifType = NOTIFY_CALENDAR;
    else if (strcmp(typeStr, "system") == 0) notifType = NOTIFY_SYSTEM;
  }

  if (doc.containsKey("title")) {
    title = doc["title"];
  }

  if (doc.containsKey("message")) {
    message = doc["message"];
  }

  if (networkManager.notifications().add(notifType, title, message)) {
    Serial.printf("Notification queued: [%d] %s - %s\n", notifType, title, message);

    if (emotionManager.getCurrentEmotion() != EMOTION_NOTIFICATION) {
      emotionManager.setTargetEmotion(EMOTION_NOTIFICATION);
    }
  } else {
    Serial.println("Notification queue full - dropped");
  }
}

void handleGitHubCommits(const char* topic, const char* payload) {
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, payload)) return;

  // Validate SSID if present
  if (doc.containsKey("ssid")) {
    const char* receivedSSID = doc["ssid"];
    if (!networkManager.validateSSID(receivedSSID)) {
      networkManager.setWorkspaceMode(false);
      return;
    }
    networkManager.setWorkspaceMode(true);
    networkManager.updateLastMQTTMessageTime();
  }

  if (doc.containsKey("contributions") && doc["contributions"].is<JsonArray>()) {
    const char* username = doc["username"] | "user";
    int total = doc["total"] | 0;
    int currentStreak = doc["current_streak"] | 0;
    int longestStreak = doc["longest_streak"] | 0;

    JsonArray weeksArray = doc["contributions"].as<JsonArray>();

    uint8_t contributions[52][7];
    memset(contributions, 0, sizeof(contributions));

    int weekIndex = 0;
    for (JsonVariant weekVar : weeksArray) {
      if (weekIndex >= 52) break;

      if (weekVar.is<JsonArray>()) {
        JsonArray daysArray = weekVar.as<JsonArray>();
        int dayIndex = 0;

        for (JsonVariant dayVar : daysArray) {
          if (dayIndex >= 7) break;
          contributions[weekIndex][dayIndex] = dayVar.as<uint8_t>();
          dayIndex++;
        }
      }
      weekIndex++;
    }

    networkManager.github().setContributions(contributions, total, currentStreak, longestStreak, username);

    Serial.printf("Updated GitHub data: %d contributions, %d day streak\n", total, currentStreak);
  } else {
    Serial.println("Invalid GitHub contribution format");
  }
}

void handleGitHubStats(const char* topic, const char* payload) {
  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, payload)) return;

  // Validate SSID if present
  if (doc.containsKey("ssid")) {
    const char* receivedSSID = doc["ssid"];
    if (!networkManager.validateSSID(receivedSSID)) {
      networkManager.setWorkspaceMode(false);
      return;
    }
    networkManager.setWorkspaceMode(true);
    networkManager.updateLastMQTTMessageTime();
  }

  const char* username = doc["username"] | "user";
  int repos = doc["repos"] | 0;
  int followers = doc["followers"] | 0;
  int following = doc["following"] | 0;
  int contributions = doc["contributions"] | 0;
  int commits = doc["commits"] | 0;
  int prs = doc["prs"] | 0;
  int issues = doc["issues"] | 0;
  int stars = doc["stars"] | 0;

  networkManager.github().setStats(username, repos, followers, following, contributions, commits, prs, issues, stars);

  Serial.printf("Updated GitHub stats: %s - %d repos, %d followers, %d contributions\n",
                username, repos, followers, contributions);

  // Automatically trigger GitHub stats emotion
  if (emotionManager.getCurrentEmotion() != EMOTION_GITHUB_STATS) {
    emotionManager.setTargetEmotion(EMOTION_GITHUB_STATS);
  }
}

// ===== EMOTION REGISTRY SETUP =====
void registerEmotions() {
  // Static emotions
  emotionRegistry.add(
      {EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, drawIdle});
  emotionRegistry.add(
      {EMOTION_BLINK, "BLINK", 1, 0, LOOP_RESTART, false, drawBlink});

  // Standard animated emotions (51 frames @ 30ms)
  emotionRegistry.add(
      {EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, drawHappy});
  emotionRegistry.add(
      {EMOTION_SLEEPY, "SLEEPY", 51, 30, LOOP_RESTART, false, drawSleepy});
  emotionRegistry.add(
      {EMOTION_EXCITED, "EXCITED", 51, 30, LOOP_RESTART, true, drawExcited});
  emotionRegistry.add(
      {EMOTION_SAD, "SAD", 51, 30, LOOP_RESTART, true, drawSad});
  emotionRegistry.add(
      {EMOTION_ANGRY, "ANGRY", 51, 30, LOOP_RESTART, true, drawAngry});
  emotionRegistry.add(
      {EMOTION_CONFUSED, "CONFUSED", 51, 30, LOOP_RESTART, true, drawConfused});
  emotionRegistry.add(
      {EMOTION_THINKING, "THINKING", 51, 30, LOOP_RESTART, true, drawThinking});
  emotionRegistry.add(
      {EMOTION_LOVE, "LOVE", 51, 30, LOOP_RESTART, true, drawLove});
  emotionRegistry.add(
      {EMOTION_SURPRISED, "SURPRISED", 51, 30, LOOP_RESTART, true, drawSurprised});
  emotionRegistry.add(
      {EMOTION_DEAD, "DEAD", 51, 30, LOOP_RESTART, false, drawDead});
  emotionRegistry.add(
      {EMOTION_MUSIC, "MUSIC", 51, 30, LOOP_RESTART, false, drawMusic});

  // Special animations
  emotionRegistry.add(
      {EMOTION_NOTIFICATION, "NOTIFICATION", 86, 50, LOOP_ONCE, false,
       drawNotification});
  emotionRegistry.add(
      {EMOTION_CODING, "CODING", 25, 100, LOOP_RESTART, false, drawCoding});
  emotionRegistry.add({EMOTION_GITHUB_STATS, "GITHUB_STATS", 131, 80,
                       LOOP_RESTART, false, drawGitHubStats});
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
    Serial.println("FATAL: Display init failed");
    for (;;) { delay(1000); }
  }

  // Initialize emotion manager with callbacks
  emotionManager.init(bootTime);
  emotionManager.setOnTransitionComplete(onTransitionComplete);
  emotionManager.setOnEmotionChange(onEmotionChange);

  // Initialize other modules
  inputManager.init();
  inputManager.updateLastInteraction(bootTime);
  inputManager.setOnTouch(onTouch);
  batteryManager.init();
  beepManager.init();

#if !DEBUG_MODE_ENABLED
  displayManager.showBootScreen();
#else
  Serial.println("Skipping boot screen in DEBUG MODE");
#endif

  // Initialize network and register MQTT topic handlers
#if ENABLE_MQTT
  // Register topic handlers BEFORE init (so they subscribe on connect)
  mqttManager.subscribe(MQTT_TOPIC_EMOTION_SET, handleEmotionSet);
  mqttManager.subscribe("sangi/notification/push", handleNotificationPush);
  mqttManager.subscribe(MQTT_TOPIC_GITHUB_COMMITS, handleGitHubCommits);
  mqttManager.subscribe("sangi/github/stats", handleGitHubStats);

  networkManager.init();

  Serial.println("\n>>> Running Network Diagnostics <<<");
  delay(500);
  mqttManager.testConnectivity();

  networkManager.logInfo("SANGI Robot initialized successfully");
  networkManager.logInfo("Wireless serial logging active");
#else
  Serial.println("MQTT disabled - running in autonomous mode");
#endif

  // Load hardcoded commit history for testing (debug only)
#if DEBUG_MODE_ENABLED
  loadHardcodedGitHubData();
#endif

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

  EmotionState autoEmotion = selectAutonomousEmotion(
      currentTime, lastEmotionSwitch, emotionIndex,
      cyclableEmotions, numCyclable);
  if ((int)autoEmotion != -1) {
    emotionManager.setTargetEmotion(autoEmotion);
  }
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
    // Render current emotion via registry-based dispatch
    EmotionState current = emotionManager.getCurrentEmotion();

    if (current == EMOTION_NOTIFICATION) {
      // Notification needs context (title + message)
      Notification* notif = networkManager.notifications().current();
      NotificationContext nctx;
      if (notif != nullptr) {
        nctx = {notif->title, notif->message};

        static unsigned long notifStartTime = 0;
        unsigned long currentMillis = millis();
        bool overflow = currentMillis < notifStartTime;

        if (notifStartTime == 0) {
          notifStartTime = currentMillis;
        } else if (overflow ||
                   (currentMillis - notifStartTime > 4300)) {
          networkManager.notifications().clearCurrent();
          notifStartTime = 0;

          // If no more notifications, return to previous emotion
          if (!networkManager.notifications().hasItems() &&
              emotionManager.getCurrentEmotion() == EMOTION_NOTIFICATION) {
            emotionManager.setTargetEmotion(EMOTION_IDLE);
          }
        }
      } else {
        if (offlineNotifTitle[0] == '\0') {
          generateOfflineNotification();
        }
        nctx = {offlineNotifTitle, offlineNotifMessage};
      }
      animationManager.tick(current, displayManager, &nctx);
    } else if (current == EMOTION_GITHUB_STATS) {
      // GitHub stats needs context
      const GitHubStatsData* statsData = networkManager.github().getStats();
      GitHubStatsContext sctx = {};
      if (statsData != nullptr && networkManager.github().hasStats()) {
        sctx.hasData = true;
        sctx.username = statsData->username;
        sctx.repos = statsData->repos;
        sctx.followers = statsData->followers;
        sctx.following = statsData->following;
        sctx.contributions = statsData->contributions;
        sctx.commits = statsData->commits;
        sctx.prs = statsData->prs;
        sctx.issues = statsData->issues;
        sctx.stars = statsData->stars;
      }
      animationManager.tick(current, displayManager, &sctx);
    } else {
      animationManager.tick(current, displayManager);
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
                  mqttManager.isConnected() ? "Connected"
                                            : "Disconnected");
#else
    Serial.println();
#endif
    lastDebug = currentTime;
  }

  delay(50);
}
