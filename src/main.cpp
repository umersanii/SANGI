#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <esp_sleep.h>  // Required for ESP32 sleep functions

// ===== HARDWARE CONFIGURATION =====
// OLED display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// I2C pins for ESP32-C3
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

// ===== TIMING CONFIGURATION =====
#define EMOTION_CHANGE_INTERVAL 30000  // 30 seconds
#define BLINK_INTERVAL 3000  // 3 seconds
#define SLEEP_TIMEOUT 300000  // 5 minutes
#define HOUR_IN_MILLIS 3600000

// ===== EMOTION STATE MACHINE =====
enum EmotionState {
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
  EMOTION_BLINK
};

// Forward declaration for transition function
void drawEmotionFace(EmotionState emotion);

// Global state variables
EmotionState currentEmotion = EMOTION_IDLE;
EmotionState previousEmotion = EMOTION_IDLE;
EmotionState targetEmotion = EMOTION_IDLE;
unsigned long lastEmotionChange = 0;
unsigned long lastBlink = 0;
unsigned long lastInteraction = 0;
unsigned long bootTime = 0;
bool isTransitioning = false;
int transitionFrame = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===== FORWARD DECLARATIONS =====
void drawEyes(int leftX, int leftY, int rightX, int rightY, int eyeHeight);
void drawFace_Normal();
void drawFace_Happy();
void drawFace_Blink();
void drawFace_Sad();
void drawFace_Angry();
void drawFace_Love();
void drawFace_Sleepy();
void drawFace_Excited();
void drawFace_Confused();
void drawFace_Thinking();
void drawFace_Dead();
void drawFace_Surprised();


// ===== BATTERY MONITORING =====
float readBatteryVoltage() {
  int rawValue = analogRead(BATTERY_PIN);
  float voltage = (rawValue / 4095.0) * 3.3;
  return voltage;
}

EmotionState getBatteryBasedEmotion() {
  // Disabled for USB power - uncomment when using LiPo battery
  /*
  float voltage = readBatteryVoltage();
  
  if (voltage < BATTERY_MIN_VOLTAGE) {
    return EMOTION_DEAD;
  } else if (voltage < BATTERY_LOW_THRESHOLD) {
    return EMOTION_SLEEPY;
  }
  */
  
  return EMOTION_IDLE;  // No override when on USB power
}

// ===== TIME-BASED EMOTION CYCLES =====
EmotionState getTimeBasedEmotion() {
  unsigned long currentTime = millis() - bootTime;
  unsigned long hourOfDay = (currentTime / HOUR_IN_MILLIS) % 24;
  
  if (hourOfDay >= 6 && hourOfDay < 12) {
    return EMOTION_HAPPY;  // Morning
  } else if (hourOfDay >= 12 && hourOfDay < 18) {
    return EMOTION_THINKING;  // Afternoon
  } else if (hourOfDay >= 18 && hourOfDay < 22) {
    return EMOTION_IDLE;  // Evening
  } else {
    return EMOTION_SLEEPY;  // Night
  }
}

// ===== TOUCH SENSOR INPUT =====
void setupTouchSensor() {
  pinMode(TOUCH_PIN, INPUT_PULLUP);
}

bool isTouched() {
  return digitalRead(TOUCH_PIN) == LOW;
}

void handleTouchInteraction() {
  if (isTouched()) {
    lastInteraction = millis();
    
    if (random(0, 2) == 0) {
      targetEmotion = EMOTION_EXCITED;
    } else {
      targetEmotion = EMOTION_SURPRISED;
    }
    
    if (currentEmotion != targetEmotion) {
      previousEmotion = currentEmotion;
      isTransitioning = true;
      transitionFrame = 0;
    }
    
    delay(50);  // Debounce
  }
}

// ===== EMOTION STATE MANAGEMENT =====
void setTargetEmotion(EmotionState newEmotion) {
  if (currentEmotion != newEmotion) {
    previousEmotion = currentEmotion;
    targetEmotion = newEmotion;
    isTransitioning = true;
    transitionFrame = 0;
  }
}

void updateEmotionState() {
  unsigned long currentTime = millis();
  
  // DISABLED FOR SLEEPY TESTING
  /*
  // Check battery status first
  EmotionState batteryEmotion = getBatteryBasedEmotion();
  if (batteryEmotion != EMOTION_IDLE) {
    setTargetEmotion(batteryEmotion);
    return;
  }
  
  // Regular emotion changes
  if (currentTime - lastEmotionChange > EMOTION_CHANGE_INTERVAL) {
    EmotionState timeEmotion = getTimeBasedEmotion();
    
    if (random(0, 3) == 0) {
      EmotionState randomEmotions[] = {
        EMOTION_HAPPY, EMOTION_THINKING, EMOTION_CONFUSED, 
        EMOTION_LOVE, EMOTION_IDLE
      };
      timeEmotion = randomEmotions[random(0, 5)];
    }
    
    setTargetEmotion(timeEmotion);
    lastEmotionChange = currentTime;
  }
  */
  
  // Periodic blinking - DISABLED FOR SLEEPY TEST
  /*
  if (currentTime - lastBlink > BLINK_INTERVAL && !isTransitioning) {
    int eyeHeights[] = {20, 16, 10, 6, 4, 6, 10, 16, 20};
    for(int i = 0; i < 9; i++) {
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, eyeHeights[i]);
      display.display();
      delay(30);
    }
    lastBlink = currentTime;
  }
  */
}

// ===== FACE DRAWING FUNCTIONS =====
void drawEyes(int leftX, int leftY, int rightX, int rightY, int eyeHeight) {
  display.fillRoundRect(leftX - 10, leftY - eyeHeight/2, 20, eyeHeight, 5, SSD1306_WHITE);
  display.fillRoundRect(rightX - 10, rightY - eyeHeight/2, 20, eyeHeight, 5, SSD1306_WHITE);
}

void drawFace_Normal() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 20);
  display.display();
}

void drawFace_Happy() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 16);
  display.fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
  display.display();
}

void drawFace_Blink() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 4);
  display.display();
}

void drawFace_Sad() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 20);
  display.fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
  display.display();
}

void drawFace_Angry() {
  display.clearDisplay();
  drawEyes(40, 30, 88, 30, 12);
  display.fillTriangle(25, 18, 30, 22, 50, 18, SSD1306_WHITE);
  display.fillTriangle(78, 18, 98, 22, 103, 18, SSD1306_WHITE);
  display.fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
  display.display();
}

void drawFace_Love() {
  display.clearDisplay();
  display.fillCircle(35, 26, 8, SSD1306_WHITE);
  display.fillCircle(45, 26, 8, SSD1306_WHITE);
  display.fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
  display.fillCircle(83, 26, 8, SSD1306_WHITE);
  display.fillCircle(93, 26, 8, SSD1306_WHITE);
  display.fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
  display.fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
  display.display();
}

void drawFace_Sleepy() {
  display.clearDisplay();
  drawEyes(40, 30, 88, 30, 8);
  display.drawCircle(64, 48, 6, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(105, 12);
  display.print("z");
  display.setCursor(108, 8);
  display.print("Z");
  display.display();
}

// Animated sleepy state - smooth drowsy animations with synchronized mouth
void animateSleepy() {
  static unsigned long lastSleepyAnim = 0;
  static int sleepyFrame = 0;
  unsigned long currentTime = millis();
  
  // Animate every 30ms to match blinking animation frame rate
  if (currentTime - lastSleepyAnim > 30) {
    display.clearDisplay();
    
    switch(sleepyFrame) {
      // === CLOSING SEQUENCE (frames 0-8) ===
      case 0:
        // Eyes fully open - normal idle mouth
        drawEyes(40, 28, 88, 28, 20);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes start drooping
        drawEyes(40, 29, 88, 29, 16);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 2:
        // Eyes more droopy
        drawEyes(40, 29, 88, 29, 12);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 3:
        // Eyes getting heavy
        drawEyes(40, 30, 88, 30, 10);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 4:
        // Eyes barely open - mouth starting to open
        drawEyes(40, 30, 88, 30, 8);
        display.drawCircle(64, 48, 6, SSD1306_WHITE);
        break;
        
      case 5:
        // Eyes almost closed - mouth opening more
        drawEyes(40, 31, 88, 31, 6);
        display.drawCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 6:
        // Eyes nearly shut - mouth transitioning to filled
        drawEyes(40, 31, 88, 31, 4);
        display.fillCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 7:
        // Eyes closing - mouth fully open round
        drawEyes(40, 31, 88, 31, 3);
        display.fillCircle(64, 50, 8, SSD1306_WHITE);
        break;
        
      case 8:
        // Eyes fully closed - mouth round, zzz appear
        drawEyes(40, 31, 88, 31, 2);
        display.fillCircle(64, 50, 8, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(105, 12);
        display.print("z");
        display.setCursor(110, 8);
        display.print("Z");
        break;
        
      // === STAY CLOSED (frames 9-42) - approximately 1 second at 30ms per frame ===
      case 9: case 10: case 11: case 12: case 13:
      case 14: case 15: case 16: case 17: case 18:
      case 19: case 20: case 21: case 22: case 23:
      case 24: case 25: case 26: case 27: case 28:
      case 29: case 30: case 31: case 32: case 33:
        // Eyes stay closed, mouth round, zzz active
        drawEyes(40, 31, 88, 31, 2);
        display.fillCircle(64, 50, 8, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(105, 12);
        display.print("z");
        display.setCursor(110, 8);
        display.print("Z");
        display.setCursor(115, 4);
        display.print("Z");
        break;
        
      case 34: case 35: case 36: case 37: case 38:
      case 39: case 40: case 41: case 42:
        // Eyes stay closed, mouth round, zzz active (variation)
        drawEyes(40, 31, 88, 31, 2);
        display.fillCircle(64, 50, 8, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(106, 10);
        display.print("z");
        display.setCursor(111, 6);
        display.print("Z");
        display.setCursor(116, 2);
        display.print("Z");
        break;
        
      // === OPENING SEQUENCE (frames 43-50) ===
      case 43:
        // Eyes start opening - zzz fading
        drawEyes(40, 31, 88, 31, 3);
        display.fillCircle(64, 50, 8, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(110, 8);
        display.print("Z");
        break;
        
      case 44:
        // Eyes opening more - mouth transitioning
        drawEyes(40, 31, 88, 31, 4);
        display.fillCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 45:
        // Eyes opening - mouth becoming circle outline
        drawEyes(40, 31, 88, 31, 6);
        display.drawCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 46:
        // Eyes half open
        drawEyes(40, 30, 88, 30, 8);
        display.drawCircle(64, 48, 6, SSD1306_WHITE);
        break;
        
      case 47:
        // Eyes more open
        drawEyes(40, 30, 88, 30, 10);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 48:
        // Eyes almost fully open
        drawEyes(40, 29, 88, 29, 14);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 49:
        // Eyes fully open - back to normal idle mouth
        drawEyes(40, 28, 88, 28, 18);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 50:
        // Eyes fully open - normal idle mouth
        drawEyes(40, 28, 88, 28, 20);
        display.drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
    }
    
    display.display();
    sleepyFrame = (sleepyFrame + 1) % 51;
    lastSleepyAnim = currentTime;
  }
}

// Animated thinking state - shifting thought bubbles
void animateThinking() {
  static unsigned long lastThinkAnim = 0;
  static int thinkFrame = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastThinkAnim > 30) {
    display.clearDisplay();
    drawEyes(35, 22, 83, 22, 20);  // Eyes looking up
    display.fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
    
    // Animated thought bubbles (20 frames for smoother animation)
    int bubblePhase = thinkFrame / 5;  // 0-3 phases, 5 frames each
    switch(bubblePhase) {
      case 0:
        display.fillCircle(102, 18, 2, SSD1306_WHITE);
        display.fillCircle(108, 12, 3, SSD1306_WHITE);
        break;
      case 1:
        display.fillCircle(105, 16, 2, SSD1306_WHITE);
        display.fillCircle(110, 10, 3, SSD1306_WHITE);
        display.fillCircle(116, 5, 4, SSD1306_WHITE);
        break;
      case 2:
        display.fillCircle(108, 14, 2, SSD1306_WHITE);
        display.fillCircle(113, 8, 3, SSD1306_WHITE);
        display.fillCircle(118, 3, 4, SSD1306_WHITE);
        break;
      case 3:
        display.fillCircle(110, 10, 3, SSD1306_WHITE);
        display.fillCircle(116, 5, 4, SSD1306_WHITE);
        break;
    }
    
    display.display();
    thinkFrame = (thinkFrame + 1) % 20;
    lastThinkAnim = currentTime;
  }
}

// Animated excited state - bouncing with sparkles
void animateExcited() {
  static unsigned long lastExciteAnim = 0;
  static int exciteFrame = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastExciteAnim > 30) {
    display.clearDisplay();
    
    // Bouncing eyes (smoother with more frames)
    int yOffset = (exciteFrame < 7) ? 26 : 30;
    drawEyes(40, yOffset, 88, yOffset, 24);
    display.fillCircle(40, yOffset, 3, SSD1306_BLACK);
    display.fillCircle(88, yOffset, 3, SSD1306_BLACK);
    
    // Big smile
    display.fillRoundRect(45, 50, 38, 10, 5, SSD1306_WHITE);
    
    // Sparkles around face
    if (exciteFrame < 7 || (exciteFrame >= 14 && exciteFrame < 21)) {
      display.fillCircle(20, 15, 2, SSD1306_WHITE);
      display.fillCircle(108, 15, 2, SSD1306_WHITE);
      display.fillCircle(15, 35, 2, SSD1306_WHITE);
      display.fillCircle(113, 35, 2, SSD1306_WHITE);
    }
    
    display.display();
    exciteFrame = (exciteFrame + 1) % 14;
    lastExciteAnim = currentTime;
  }
}

// Animated confused state - head tilting
void animateConfused() {
  static unsigned long lastConfuseAnim = 0;
  static int confuseFrame = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastConfuseAnim > 30) {
    display.clearDisplay();
    
    // Alternating eye sizes (one squint, one open) - smoother transition
    int phase = confuseFrame / 8;  // 0-2 phases, 8 frames each
    if (phase % 2 == 0) {
      display.fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
      display.fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
    } else {
      display.fillRoundRect(30, 26, 20, 12, 5, SSD1306_WHITE);
      display.fillRoundRect(78, 24, 20, 20, 5, SSD1306_WHITE);
    }
    
    // Wavy mouth
    display.drawLine(50, 48, 58, 52, SSD1306_WHITE);
    display.drawLine(58, 52, 70, 48, SSD1306_WHITE);
    display.drawLine(70, 48, 78, 52, SSD1306_WHITE);
    
    // Question mark appears
    if (phase == 1) {
      display.setTextSize(2);
      display.setCursor(105, 10);
      display.print("?");
    }
    
    display.display();
    confuseFrame = (confuseFrame + 1) % 24;
    lastConfuseAnim = currentTime;
  }
}

// Animated happy state - gentle smile pulsing
void animateHappy() {
  static unsigned long lastHappyAnim = 0;
  static int happyFrame = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastHappyAnim > 30) {
    display.clearDisplay();
    drawEyes(40, 28, 88, 28, 16);
    
    // Pulsing smile width (smoother with more frames)
    int phase = happyFrame / 8;  // 0-2 phases
    int smileWidth = (phase % 2 == 0) ? 28 : 32;
    int smileX = (phase % 2 == 0) ? 50 : 48;
    display.fillRoundRect(smileX, 48, smileWidth, 8, 4, SSD1306_WHITE);
    
    display.display();
    happyFrame = (happyFrame + 1) % 16;
    lastHappyAnim = currentTime;
  }
}

// Animated love state - heart eyes pulsing
void animateLove() {
  static unsigned long lastLoveAnim = 0;
  static int loveFrame = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastLoveAnim > 30) {
    display.clearDisplay();
    
    // Pulsing heart eyes (smoother transition)
    int phase = loveFrame / 8;  // 0-1 phases
    int heartSize = (phase % 2 == 0) ? 8 : 7;
    display.fillCircle(35, 26, heartSize, SSD1306_WHITE);
    display.fillCircle(45, 26, heartSize, SSD1306_WHITE);
    display.fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
    
    display.fillCircle(83, 26, heartSize, SSD1306_WHITE);
    display.fillCircle(93, 26, heartSize, SSD1306_WHITE);
    display.fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
    
    // Big smile
    display.fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
    
    // Floating hearts
    if (phase == 0) {
      display.drawCircle(110, 15, 3, SSD1306_WHITE);
      display.drawCircle(115, 10, 2, SSD1306_WHITE);
    }
    
    display.display();
    loveFrame = (loveFrame + 1) % 16;
    lastLoveAnim = currentTime;
  }
}

// Animated angry state - shaking/vibrating
void animateAngry() {
  static unsigned long lastAngryAnim = 0;
  static int angryFrame = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastAngryAnim > 30) {
    display.clearDisplay();
    
    // Shaking offset (smoother with more frames)
    int xOffset = (angryFrame % 4 < 2) ? 0 : 2;
    
    drawEyes(40 + xOffset, 30, 88 + xOffset, 30, 12);
    
    // Angry eyebrows
    display.fillTriangle(25 + xOffset, 18, 30 + xOffset, 22, 50 + xOffset, 18, SSD1306_WHITE);
    display.fillTriangle(78 + xOffset, 18, 98 + xOffset, 22, 103 + xOffset, 18, SSD1306_WHITE);
    
    // Frown
    display.fillRoundRect(52 + xOffset, 50, 24, 6, 3, SSD1306_WHITE);
    
    // Anger lines
    if (angryFrame % 4 < 2) {
      display.drawLine(15, 25, 20, 30, SSD1306_WHITE);
      display.drawLine(108, 25, 113, 30, SSD1306_WHITE);
    }
    
    display.display();
    angryFrame = (angryFrame + 1) % 12;
    lastAngryAnim = currentTime;
  }
}

void drawFace_Excited() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 24);
  display.fillCircle(40, 28, 3, SSD1306_BLACK);
  display.fillCircle(88, 28, 3, SSD1306_BLACK);
  display.fillRoundRect(45, 50, 38, 10, 5, SSD1306_WHITE);
  display.display();
}

void drawFace_Confused() {
  display.clearDisplay();
  display.fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
  display.drawLine(50, 48, 58, 52, SSD1306_WHITE);
  display.drawLine(58, 52, 70, 48, SSD1306_WHITE);
  display.drawLine(70, 48, 78, 52, SSD1306_WHITE);
  display.display();
}

void drawFace_Thinking() {
  display.clearDisplay();
  display.fillRoundRect(35, 22, 18, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(83, 22, 18, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
  display.fillCircle(105, 15, 2, SSD1306_WHITE);
  display.fillCircle(110, 10, 3, SSD1306_WHITE);
  display.fillCircle(115, 5, 4, SSD1306_WHITE);
  display.display();
}

void drawFace_Dead() {
  display.clearDisplay();
  display.drawLine(32, 22, 48, 34, SSD1306_WHITE);
  display.drawLine(48, 22, 32, 34, SSD1306_WHITE);
  display.drawLine(80, 22, 96, 34, SSD1306_WHITE);
  display.drawLine(96, 22, 80, 34, SSD1306_WHITE);
  display.fillRoundRect(58, 46, 12, 14, 3, SSD1306_WHITE);
  display.display();
}

void drawFace_Surprised() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 28);
  display.fillCircle(64, 50, 6, SSD1306_WHITE);
  display.display();
}

// ===== TRANSITION ANIMATIONS =====
void drawEmotionFace(EmotionState emotion) {
  switch(emotion) {
    case EMOTION_IDLE:
      drawFace_Normal();
      break;
    case EMOTION_HAPPY:
      drawFace_Happy();
      break;
    case EMOTION_SLEEPY:
      drawFace_Sleepy();
      break;
    case EMOTION_EXCITED:
      drawFace_Excited();
      break;
    case EMOTION_SAD:
      drawFace_Sad();
      break;
    case EMOTION_ANGRY:
      drawFace_Angry();
      break;
    case EMOTION_CONFUSED:
      drawFace_Confused();
      break;
    case EMOTION_THINKING:
      drawFace_Thinking();
      break;
    case EMOTION_LOVE:
      drawFace_Love();
      break;
    case EMOTION_SURPRISED:
      drawFace_Surprised();
      break;
    case EMOTION_DEAD:
      drawFace_Dead();
      break;
    default:
      drawFace_Normal();
      break;
  }
}

// Smooth multi-frame transition between emotions
void performTransition() {
  if (!isTransitioning) return;
  
  switch(transitionFrame) {
    case 0:
      // Frame 1: Current emotion
      drawEmotionFace(currentEmotion);
      delay(80);
      transitionFrame++;
      break;
      
    case 1:
      // Frame 2: Squint eyes (preparing to blink)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 12);
      display.display();
      delay(60);
      transitionFrame++;
      break;
      
    case 2:
      // Frame 3: Eyes mostly closed
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 6);
      display.display();
      delay(60);
      transitionFrame++;
      break;
      
    case 3:
      // Frame 4: Eyes fully closed (blink)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 3);
      display.display();
      delay(80);
      transitionFrame++;
      break;
      
    case 4:
      // Frame 5: Eyes starting to open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 8);
      display.display();
      delay(60);
      transitionFrame++;
      break;
      
    case 5:
      // Frame 6: Eyes half open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 14);
      display.display();
      delay(60);
      transitionFrame++;
      break;
      
    case 6:
      // Frame 7: Target emotion fully revealed
      drawEmotionFace(targetEmotion);
      delay(80);
      currentEmotion = targetEmotion;
      isTransitioning = false;
      transitionFrame = 0;
      break;
  }
}

// ===== POWER MANAGEMENT =====
void checkSleepConditions() {
  // Disabled until touch sensor is connected - uncomment when ready
  /*
  unsigned long currentTime = millis();
  
  if (currentTime - lastInteraction > SLEEP_TIMEOUT) {
    display.clearDisplay();
    drawFace_Sleepy();
    delay(1000);
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(25, 28);
    display.println("Sleeping...");
    display.display();
    delay(1000);
    
    // ESP32-C3 uses GPIO wakeup instead of ext0
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable((gpio_num_t)TOUCH_PIN, GPIO_INTR_LOW_LEVEL);
    esp_deep_sleep_start();
  }
  */
}

// ===== I2C SCANNER =====
void scanI2C() {
  Serial.println("\nScanning I2C bus...");
  byte error, address;
  int devices = 0;

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      devices++;
    }
  }

  if (devices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("I2C scan complete\n");
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== SANGI Robot Initializing ===");
  
  bootTime = millis();
  lastInteraction = bootTime;
  lastEmotionChange = bootTime;
  lastBlink = bootTime;
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.printf("I2C initialized on SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
  
  scanI2C();
  
  Serial.println("Initializing OLED...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed!");
    for(;;);
  }
  
  Serial.println("OLED initialized successfully!");
  
  setupTouchSensor();
  Serial.printf("Touch sensor configured on GPIO%d\n", TOUCH_PIN);
  
  pinMode(BATTERY_PIN, INPUT);
  analogReadResolution(12);
  Serial.printf("Battery monitoring on GPIO%d\n", BATTERY_PIN);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println("SANGI");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.println("Booting...");
  display.display();
  delay(1500);
  
  // ANIMATION TEST MODE - cycle through all emotions
  currentEmotion = EMOTION_IDLE;
  targetEmotion = EMOTION_IDLE;
  
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
    currentEmotion = testEmotions[emotionIndex];
    targetEmotion = currentEmotion;
    lastEmotionSwitch = currentTime;
    
    // Print which emotion is being displayed
    const char* emotionNames[] = {
      "IDLE", "HAPPY", "SLEEPY", "EXCITED", "CONFUSED",
      "THINKING", "LOVE", "ANGRY", "SAD", "SURPRISED"
    };
    Serial.printf("Now testing: %s\n", emotionNames[emotionIndex]);
  }
  
  handleTouchInteraction();
  
  if (isTransitioning) {
    performTransition();
  } else {
    // Use animated states for specific emotions
    switch(currentEmotion) {
      case EMOTION_SLEEPY:
        animateSleepy();
        break;
      case EMOTION_THINKING:
        animateThinking();
        break;
      case EMOTION_EXCITED:
        animateExcited();
        break;
      case EMOTION_CONFUSED:
        animateConfused();
        break;
      case EMOTION_HAPPY:
        animateHappy();
        break;
      case EMOTION_LOVE:
        animateLove();
        break;
      case EMOTION_ANGRY:
        animateAngry();
        break;
      default:
        // Static display for other emotions
        drawEmotionFace(currentEmotion);
        break;
    }
  }
  
  checkSleepConditions();
  
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 10000) {
    float voltage = readBatteryVoltage();
    Serial.printf("Battery: %.2fV | Emotion: %d | Uptime: %lus\n", 
                  voltage, currentEmotion, (millis() - bootTime) / 1000);
    lastDebug = millis();
  }
  
  delay(50);
}
