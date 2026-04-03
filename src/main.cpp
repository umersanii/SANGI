// main.cpp — orchestration layer for SANGI robot.
// v1: Standalone device, no networking. Autonomous emotion cycling via registry.

#include <Arduino.h>
#include "config.h"
#include "emotion.h"
#include "emotion_registry.h"
#include "emotion_draws.h"
#include "display.h"
#include "animations.h"
#include "battery.h"
#include "input.h"
#include "speaker.h"
#include "ble_control.h"
#include "personality.h"
#include "runtime_config.h"
#include "web_server.h"
#include <WiFi.h>

// ===== GLOBAL STATE =====
unsigned long bootTime = 0;

// ===== CALLBACKS (wiring between decoupled modules) =====

// Called when a blink transition completes; resets the animation state for the new emotion.
void onTransitionComplete(EmotionState newEmotion) {
  animationManager.resetAnimation(newEmotion);
}

// Called when the active emotion changes; queues a beep if enableEmotionBeep is set.
void onEmotionChange(EmotionState from, EmotionState to) {
  if (runtimeConfig.enableEmotionBeep) {
    beepManager.queueEmotionBeep(to);
  }
}

// ===== BLE CALLBACK =====

// BLE write callback: sets the target emotion when a valid emotion ID is received over BLE.
void onBleEmotion(EmotionState e) {
  emotionManager.setTargetEmotion(e);
  Serial.printf("BLE: emotion set to %s\n", emotionRegistry.getName(e));
}

// ===== GESTURE CALLBACK =====

// Touch gesture callback: resets neglect arc on any touch, then maps gesture type to emotion.
// Multi-touch forgiveness: deep neglect (GRUMPY/ANGRY) requires multiple touches before recovery.
// During forgiveness, SANGI stays in its current sulk emotion — no SHY yet.
void onGesture(TouchGesture gesture, unsigned long currentTime) {
  bool wasNeglected = personality.onTouch(currentTime, emotionManager.getCurrentEmotion());

  // Still forgiving — SANGI hasn't warmed up yet, stay in sulk
  if (personality.isForgiving()) {
    return;
  }

  // Neglect recovery complete — enter SHY before warmth arc
  if (wasNeglected) {
    emotionManager.setTargetEmotion(EMOTION_SHY);
    return;
  }

  // Normal touch responses
  switch (gesture) {
    case GESTURE_TAP:
      emotionManager.setTargetEmotion(EMOTION_HAPPY);
      break;
    case GESTURE_LONG_PRESS:
      emotionManager.setTargetEmotion(EMOTION_LOVE);
      break;
    case GESTURE_DOUBLE_TAP:
      emotionManager.setTargetEmotion(EMOTION_EXCITED);
      break;
    default: break;
  }
}

// ===== EMOTION REGISTRY SETUP =====

// Registers all emotion definitions with per-emotion frame counts, delays, loop modes, and draw functions.
void registerEmotions() {
  // Frame counts and delays now vary per emotion — each has its own temporal character.
  // Loop durations (approximate):
  //   PINGPONG full cycle = (frameCount * 2 - 2) * frameDelay
  //   RESTART full cycle  = frameCount * frameDelay
  //
  // frameDelay tiers match emotional energy:
  //   High energy  (EXCITED, ANGRY, SURPRISED): 25-35ms — snappy, intense
  //   Neutral      (HAPPY, LOVE, CONFUSED, THINKING, IDLE): 50-60ms — expressive, measured
  //   Low energy   (SAD, SLEEPY, BORED): 75-85ms — heavy, lingering
  emotionRegistry.add({EMOTION_IDLE,      "IDLE",      60,  55, LOOP_PINGPONG, true,  drawIdle});      // ~6.4s  neutral breathing
  emotionRegistry.add({EMOTION_BLINK,     "BLINK",      1,   0, LOOP_RESTART,  false, drawBlink});
  emotionRegistry.add({EMOTION_HAPPY,     "HAPPY",     50,  55, LOOP_PINGPONG, true,  drawHappy});     // ~5.4s  warm, measured
  emotionRegistry.add({EMOTION_SLEEPY,    "SLEEPY",    59,  80, LOOP_RESTART,  false, drawSleepy});    // ~4.7s  doze + snap wake
  emotionRegistry.add({EMOTION_EXCITED,   "EXCITED",   40,  28, LOOP_PINGPONG, true,  drawExcited});   // ~2.2s  rapid bounce energy
  emotionRegistry.add({EMOTION_SAD,       "SAD",       56,  80, LOOP_RESTART,  true,  drawSad});       // ~4.5s  heavy, slow tears
  emotionRegistry.add({EMOTION_ANGRY,     "ANGRY",     56,  30, LOOP_PINGPONG, true,  drawAngry});     // ~3.3s  fast intense shake
  emotionRegistry.add({EMOTION_CONFUSED,  "CONFUSED",  44,  55, LOOP_PINGPONG, true,  drawConfused});  // ~4.7s  measured puzzlement
  emotionRegistry.add({EMOTION_THINKING,  "THINKING",  44,  55, LOOP_PINGPONG, true,  drawThinking});  // ~4.7s  contemplative pace
  emotionRegistry.add({EMOTION_LOVE,      "LOVE",      44,  55, LOOP_PINGPONG, true,  drawLove});      // ~4.7s  gentle pulse
  emotionRegistry.add({EMOTION_SURPRISED, "SURPRISED", 44,  30, LOOP_RESTART,  true,  drawSurprised}); // ~1.3s  quick shock snap
  emotionRegistry.add({EMOTION_DEAD,      "DEAD",      70,  65, LOOP_RESTART,  false, drawDead});      // ~4.6s  keep RESTART — no zombie bounce
  emotionRegistry.add({EMOTION_BORED,     "BORED",     60,  80, LOOP_PINGPONG, true,  drawBored});     // ~9.5s  painfully slow
  emotionRegistry.add({EMOTION_SHY,       "SHY",       50,  60, LOOP_RESTART,  true,  drawShy});       // ~3.0s  bashful recovery arc
  emotionRegistry.add({EMOTION_NEEDY,     "NEEDY",     54,  65, LOOP_PINGPONG, true,  drawNeedy});     // ~3.5s  pleading solicitation
  emotionRegistry.add({EMOTION_CONTENT,   "CONTENT",   60,  90, LOOP_PINGPONG, true,  drawContent});   // ~10.8s quiet purring satisfaction
  emotionRegistry.add({EMOTION_PLAYFUL,   "PLAYFUL",   48,  40, LOOP_RESTART,  true,  drawPlayful});   // ~1.9s  mischievous wink-face
  emotionRegistry.add({EMOTION_GRUMPY,    "GRUMPY",    56,  45, LOOP_PINGPONG, true,  drawGrumpy});    // ~5.0s  low flat disapproval
}

// ===== POWER MANAGEMENT =====

// Placeholder for power management logic; no-op until touch sensor wiring is finalized.
void checkSleepConditions() {
  // Disabled until touch sensor is connected
}

// ===== DEBUG CYCLE =====
#if DEBUG_MODE_ENABLED && DEBUG_MODE_CYCLE
static EmotionState debugCycleList[EmotionRegistry::MAX_EMOTIONS];
static int debugCycleCount = 0;
static int debugCycleIndex = 0;
static unsigned long debugCycleLastChange = 0;

// Builds the debug cycle list from all cyclable registered emotions.
void debugCycleInit() {
  debugCycleCount = emotionRegistry.getCyclable(debugCycleList, EmotionRegistry::MAX_EMOTIONS);
  debugCycleIndex = 0;
  debugCycleLastChange = millis();
  Serial.printf("[DEBUG] Cycle: %d emotions registered\n", debugCycleCount);
}

// Steps through all registered emotions at DEBUG_CYCLE_INTERVAL_MS intervals.
void debugCycleTick(unsigned long currentTime) {
  if (debugCycleCount == 0) return;
  if (emotionManager.isTransitionActive()) return;
  if (currentTime - debugCycleLastChange >= DEBUG_CYCLE_INTERVAL_MS) {
    debugCycleIndex = (debugCycleIndex + 1) % debugCycleCount;
    EmotionState next = debugCycleList[debugCycleIndex];
    Serial.printf("[DEBUG] → %s (%d/%d)\n",
                  emotionRegistry.getName(next),
                  debugCycleIndex + 1, debugCycleCount);
    emotionManager.setTargetEmotion(next);
    debugCycleLastChange = currentTime;
  }
}
#endif

// ===== SETUP =====

// Initializes all hardware and software modules in dependency order; runs once at boot.
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

  runtimeConfigLoad();
  registerEmotions();

  if (!displayManager.init()) {
    Serial.println("FATAL: Display init failed");
    for (;;) { delay(1000); }
  }

  emotionManager.init(bootTime);
  emotionManager.setOnTransitionComplete(onTransitionComplete);
  emotionManager.setOnEmotionChange(onEmotionChange);

  inputManager.init();
  inputManager.updateLastInteraction(bootTime);
  inputManager.setOnGesture(onGesture);
  batteryManager.init();
  beepManager.init();
  bleControl.init(onBleEmotion);

  webServerManager.setEmotionManager(&emotionManager);
  webServerManager.setBatteryManager(&batteryManager);
  webServerManager.setInputManager(&inputManager);
  webServerManager.setRuntimeConfig(&runtimeConfig);
  webServerManager.setPersonality(&personality);
  webServerManager.setOnEmotionSet([](EmotionState e) {
    emotionManager.setTargetEmotion(e);
    Serial.printf("[WEB] emotion → %s\n", emotionRegistry.getName(e));
  });
  webServerManager.setOnGesture([](TouchGesture g) {
    onGesture(g, millis());
  });
  webServerManager.init();

#ifndef NATIVE_BUILD
  // Wire real-time hour provider — falls back to millis() until NTP syncs.
  // Set unconditionally so it works even when NTP connects later via the web UI.
  personality.setTimeProvider([]() -> int {
    if (webServerManager.isNtpSynced()) {
      struct tm ti;
      if (getLocalTime(&ti, 100)) return ti.tm_hour;
    }
    return (int)((millis() / HOUR_IN_MILLIS) % 24);
  });
#endif

  personality.init(bootTime);

#if !DEBUG_MODE_ENABLED
  displayManager.showBootScreen();
#else
  Serial.println("Skipping boot screen in DEBUG MODE");
#endif

#if DEBUG_MODE_ENABLED
#if DEBUG_MODE_CYCLE
  Serial.println("=== DEBUG MODE: cycling all emotions ===");
  emotionManager.setTargetEmotion(EMOTION_IDLE);
  debugCycleInit();
#else
  Serial.printf("=== DEBUG MODE: showing %s ===\n",
                emotionRegistry.getName(DEBUG_MODE_EMOTION));
  emotionManager.setTargetEmotion(DEBUG_MODE_EMOTION);
#endif
#else
  displayManager.drawEmotionFace(emotionManager.getCurrentEmotion());
  delay(1500);
#endif

  Serial.printf("=== SANGI Ready! (%d emotions registered) ===\n",
                emotionRegistry.count());
}

// ===== MAIN LOOP =====

// Web server is polled on every loop() iteration (no delay) for fast HTTP responses.
// All other modules are throttled to a ~50ms tick via millis() comparison.
void loop() {
  // Poll web server at full speed — HTTP state machine requires rapid handleClient() calls.
  webServerManager.update();

  unsigned long currentTime = millis();
  static unsigned long lastTick = 0;
  if (currentTime - lastTick < 50) return;
  lastTick = currentTime;

  beepManager.update();
  emotionManager.update(currentTime);
  bleControl.updateCurrentEmotion((uint8_t)emotionManager.getCurrentEmotion());

#if DEBUG_MODE_ENABLED && DEBUG_MODE_CYCLE
  debugCycleTick(currentTime);
#elif !DEBUG_MODE_ENABLED
  Personality::Decision d = personality.update(currentTime, emotionManager.getCurrentEmotion());
  if (d.shouldChange) {
    emotionManager.setTargetEmotion(d.emotion);
  }
#endif

  inputManager.handleTouchInteraction();

  if (emotionManager.isTransitionActive()) {
    TransitionResult r = displayManager.performTransitionFrame(
        emotionManager.getTransitionFrame(),
        emotionManager.getCurrentEmotion(),
        emotionManager.getTargetEmotion());
    if (r == TR_COMPLETE) emotionManager.completeTransition();
    else emotionManager.advanceTransition();
  } else {
    animationManager.tick(emotionManager.getCurrentEmotion(), displayManager);
  }

  checkSleepConditions();

  static unsigned long lastDebug = 0;
  if (currentTime - lastDebug > 10000) {
    float voltage = batteryManager.readVoltage();
    Serial.printf("Battery: %.2fV | Emotion: %s | Uptime: %lus\n",
                  voltage,
                  emotionRegistry.getName(emotionManager.getCurrentEmotion()),
                  (currentTime - bootTime) / 1000);
    Serial.printf("[WEB] heap: %u | clients: %d\n",
                  ESP.getFreeHeap(),
                  WiFi.softAPgetStationNum());
    lastDebug = currentTime;
  }
}
