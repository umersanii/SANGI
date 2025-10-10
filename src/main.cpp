#include <Arduino.h>
#include "config.h"
#include "emotion.h"
#include "display.h"
#include "animations.h"
#include "battery.h"
#include "input.h"

// ===== GLOBAL STATE =====
unsigned long bootTime = 0;

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
  delay(1000);
  Serial.println("\n=== SANGI Robot Initializing ===");
  
  bootTime = millis();
  
  // Initialize all modules
  if (!displayManager.init()) {
    Serial.println("Display initialization failed!");
    for(;;);
  }
  
  emotionManager.init(bootTime);
  inputManager.init();
  inputManager.updateLastInteraction(bootTime);
  batteryManager.init();
  
  // Show boot screen
  displayManager.showBootScreen();
  delay(1500);
  
  Serial.println("=== SANGI Ready! (ANIMATION TEST MODE) ===");
  Serial.println("Cycling through all animations...\n");
}

// ===== MAIN LOOP =====
void loop() {
  static unsigned long lastEmotionSwitch = 0;
  static int emotionIndex = 0;
  unsigned long currentTime = millis();
  
  // Array of all emotions to test (excluding BLINK and DEAD)
  static const EmotionState testEmotions[] = {
    EMOTION_IDLE,
    EMOTION_HAPPY,
    EMOTION_SLEEPY,
    EMOTION_EXCITED,
    EMOTION_CONFUSED,
    EMOTION_THINKING,
    EMOTION_LOVE,
    EMOTION_ANGRY,
    EMOTION_SAD,
    EMOTION_SURPRISED
  };
  static const int numEmotions = 10;
  
  // Switch emotion every 10 seconds
  if (currentTime - lastEmotionSwitch > 10000) {
    emotionIndex = (emotionIndex + 1) % numEmotions;
    EmotionState newEmotion = testEmotions[emotionIndex];
    emotionManager.setTargetEmotion(newEmotion);
    lastEmotionSwitch = currentTime;
    
    // Print which emotion is being displayed
    const char* emotionNames[] = {
      "IDLE", "HAPPY", "SLEEPY", "EXCITED", "CONFUSED",
      "THINKING", "LOVE", "ANGRY", "SAD", "SURPRISED"
    };
    Serial.printf("Now testing: %s\n", emotionNames[emotionIndex]);
  }
  
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
      default:
        // Static display for other emotions
        displayManager.drawEmotionFace(emotionManager.getCurrentEmotion());
        break;
    }
  }
  
  checkSleepConditions();
  
  // Debug output
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 10000) {
    float voltage = batteryManager.readVoltage();
    Serial.printf("Battery: %.2fV | Emotion: %d | Uptime: %lus\n", 
                  voltage, emotionManager.getCurrentEmotion(), (millis() - bootTime) / 1000);
    lastDebug = millis();
  }
  
  delay(50);
}
