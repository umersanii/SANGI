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

// ===== GLOBAL STATE =====
unsigned long bootTime = 0;

// ===== CALLBACKS (wiring between decoupled modules) =====

void onTransitionComplete(EmotionState newEmotion) {
  animationManager.resetAnimation(newEmotion);
}

void onEmotionChange(EmotionState from, EmotionState to) {
#if ENABLE_EMOTION_BEEP
  beepManager.queueEmotionBeep(to);
#endif
}

// ===== BLE CALLBACK =====
void onBleEmotion(EmotionState e) {
  emotionManager.setTargetEmotion(e);
  Serial.printf("BLE: emotion set to %s\n", emotionRegistry.getName(e));
}

// ===== GESTURE CALLBACK =====
void onGesture(TouchGesture gesture, unsigned long currentTime) {
  bool wasNeglected = personality.onTouch(currentTime, emotionManager.getCurrentEmotion());
  if (wasNeglected) {
    emotionManager.setTargetEmotion(EMOTION_HAPPY);
    return;
  }
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
void registerEmotions() {
  // Frame counts and delays now vary per emotion — each has its own temporal character.
  emotionRegistry.add({EMOTION_IDLE,      "IDLE",      60, 55, LOOP_PINGPONG, true,  drawIdle});
  emotionRegistry.add({EMOTION_BLINK,     "BLINK",      1,  0, LOOP_RESTART,  false, drawBlink});
  emotionRegistry.add({EMOTION_HAPPY,     "HAPPY",     50, 35, LOOP_RESTART,  true,  drawHappy});
  emotionRegistry.add({EMOTION_SLEEPY,    "SLEEPY",    60, 50, LOOP_PINGPONG, false, drawSleepy});
  emotionRegistry.add({EMOTION_EXCITED,   "EXCITED",   40, 25, LOOP_RESTART,  true,  drawExcited});
  emotionRegistry.add({EMOTION_SAD,       "SAD",       56, 48, LOOP_RESTART,  true,  drawSad});
  emotionRegistry.add({EMOTION_ANGRY,     "ANGRY",     56, 32, LOOP_RESTART,  true,  drawAngry});
  emotionRegistry.add({EMOTION_CONFUSED,  "CONFUSED",  44, 45, LOOP_PINGPONG, true,  drawConfused});
  emotionRegistry.add({EMOTION_THINKING,  "THINKING",  44, 45, LOOP_PINGPONG, true,  drawThinking});
  emotionRegistry.add({EMOTION_LOVE,      "LOVE",      44, 48, LOOP_PINGPONG, true,  drawLove});
  emotionRegistry.add({EMOTION_SURPRISED, "SURPRISED", 44, 30, LOOP_RESTART,  true,  drawSurprised});
  emotionRegistry.add({EMOTION_DEAD,      "DEAD",      70, 55, LOOP_RESTART,  false, drawDead});
  emotionRegistry.add({EMOTION_BORED,     "BORED",     60, 65, LOOP_PINGPONG, true,  drawBored});
}

// ===== POWER MANAGEMENT =====
void checkSleepConditions() {
  // Disabled until touch sensor is connected
}

// ===== DEBUG CYCLE =====
#if DEBUG_MODE_ENABLED && DEBUG_MODE_CYCLE
static EmotionState debugCycleList[EmotionRegistry::MAX_EMOTIONS];
static int debugCycleCount = 0;
static int debugCycleIndex = 0;
static unsigned long debugCycleLastChange = 0;

void debugCycleInit() {
  debugCycleCount = emotionRegistry.getCyclable(debugCycleList, EmotionRegistry::MAX_EMOTIONS);
  debugCycleIndex = 0;
  debugCycleLastChange = millis();
  Serial.printf("[DEBUG] Cycle: %d emotions registered\n", debugCycleCount);
}

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
void loop() {
  unsigned long currentTime = millis();
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
    lastDebug = currentTime;
  }

  delay(50);
}
