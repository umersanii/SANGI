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

// ===== TOUCH CALLBACK =====
void onTouch(unsigned long currentTime) {
  int r = random(0, 100);
  if (r < 50) {
    emotionManager.setTargetEmotion(EMOTION_EXCITED);
  } else {
    emotionManager.setTargetEmotion(EMOTION_SURPRISED);
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
  inputManager.setOnTouch(onTouch);
  batteryManager.init();
  beepManager.init();

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
