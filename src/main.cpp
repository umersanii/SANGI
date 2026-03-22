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
  switch (gesture) {
    case GESTURE_TAP:
      emotionManager.setTargetEmotion(EMOTION_HAPPY);
      Serial.println("Gesture: TAP → HAPPY");
      break;
    case GESTURE_LONG_PRESS:
      emotionManager.setTargetEmotion(EMOTION_LOVE);
      Serial.println("Gesture: LONG_PRESS → LOVE");
      break;
    case GESTURE_DOUBLE_TAP:
      emotionManager.setTargetEmotion(EMOTION_EXCITED);
      Serial.println("Gesture: DOUBLE_TAP → EXCITED");
      break;
    default: break;
  }
}

// ===== EMOTION REGISTRY SETUP =====
void registerEmotions() {
  emotionRegistry.add({EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, drawIdle});
  emotionRegistry.add({EMOTION_BLINK, "BLINK", 1, 0, LOOP_RESTART, false, drawBlink});
  emotionRegistry.add({EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, drawHappy});
  emotionRegistry.add({EMOTION_SLEEPY, "SLEEPY", 51, 30, LOOP_RESTART, false, drawSleepy});
  emotionRegistry.add({EMOTION_EXCITED, "EXCITED", 51, 30, LOOP_RESTART, true, drawExcited});
  emotionRegistry.add({EMOTION_SAD, "SAD", 51, 30, LOOP_RESTART, true, drawSad});
  emotionRegistry.add({EMOTION_ANGRY, "ANGRY", 51, 30, LOOP_RESTART, true, drawAngry});
  emotionRegistry.add({EMOTION_CONFUSED, "CONFUSED", 51, 30, LOOP_RESTART, true, drawConfused});
  emotionRegistry.add({EMOTION_THINKING, "THINKING", 51, 30, LOOP_RESTART, true, drawThinking});
  emotionRegistry.add({EMOTION_LOVE, "LOVE", 51, 30, LOOP_RESTART, true, drawLove});
  emotionRegistry.add({EMOTION_SURPRISED, "SURPRISED", 51, 30, LOOP_RESTART, true, drawSurprised});
  emotionRegistry.add({EMOTION_DEAD, "DEAD", 51, 30, LOOP_RESTART, false, drawDead});
  emotionRegistry.add({EMOTION_BORED, "BORED", 51, 60, LOOP_RESTART, true, drawBored});
  emotionRegistry.add({EMOTION_SHY, "SHY", 30, 40, LOOP_ONCE, false, drawShy});
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

#if !DEBUG_MODE_ENABLED
  displayManager.showBootScreen();
#else
  Serial.println("Skipping boot screen in DEBUG MODE");
#endif

#if DEBUG_MODE_ENABLED
  Serial.printf("=== DEBUG MODE: showing %s ===\n",
                emotionRegistry.getName(DEBUG_MODE_EMOTION));
  emotionManager.setTargetEmotion(DEBUG_MODE_EMOTION);
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

#if !DEBUG_MODE_ENABLED
  // Autonomous random cycling
  static unsigned long lastSwitch = 0;
  static EmotionState cyclable[EmotionRegistry::MAX_EMOTIONS];
  static int numCyclable = 0;
  static bool loaded = false;
  if (!loaded) {
    numCyclable = emotionRegistry.getCyclable(cyclable, EmotionRegistry::MAX_EMOTIONS);
    loaded = true;
  }
  if (currentTime - lastSwitch > EMOTION_CHANGE_INTERVAL_BASE) {
    emotionManager.setTargetEmotion(cyclable[random(0, numCyclable)]);
    lastSwitch = currentTime;
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
