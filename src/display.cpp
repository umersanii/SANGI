#include "display.h"

DisplayManager displayManager;

DisplayManager::DisplayManager() 
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

bool DisplayManager::init() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.printf("I2C initialized on SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
  
  scanI2C();
  
  Serial.println("Initializing OLED...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 allocation failed!");
    return false;
  }
  
  Serial.println("OLED initialized successfully!");
  return true;
}

void DisplayManager::scanI2C() {
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

void DisplayManager::showBootScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 20);
  display.println("SANGI");
  display.setTextSize(1);
  display.setCursor(30, 45);
  display.println("Booting...");
  display.display();
}

void DisplayManager::clearDisplay() {
  display.clearDisplay();
}

void DisplayManager::updateDisplay() {
  display.display();
}

void DisplayManager::drawEyes(int leftX, int leftY, int rightX, int rightY, int eyeHeight) {
  display.fillRoundRect(leftX - 10, leftY - eyeHeight/2, 20, eyeHeight, 5, SSD1306_WHITE);
  display.fillRoundRect(rightX - 10, rightY - eyeHeight/2, 20, eyeHeight, 5, SSD1306_WHITE);
}

void DisplayManager::drawFace_Normal() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 20);
  display.display();
}

void DisplayManager::drawFace_Happy() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 16);
  display.fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Blink() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 4);
  display.display();
}

void DisplayManager::drawFace_Sad() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 20);
  display.fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Angry() {
  display.clearDisplay();
  drawEyes(40, 30, 88, 30, 12);
  display.fillTriangle(25, 18, 30, 22, 50, 18, SSD1306_WHITE);
  display.fillTriangle(78, 18, 98, 22, 103, 18, SSD1306_WHITE);
  display.fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Love() {
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

void DisplayManager::drawFace_Sleepy() {
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

void DisplayManager::drawFace_Excited() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 24);
  display.fillCircle(40, 28, 3, SSD1306_BLACK);
  display.fillCircle(88, 28, 3, SSD1306_BLACK);
  display.fillRoundRect(45, 50, 38, 10, 5, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Confused() {
  display.clearDisplay();
  display.fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
  display.drawLine(50, 48, 58, 52, SSD1306_WHITE);
  display.drawLine(58, 52, 70, 48, SSD1306_WHITE);
  display.drawLine(70, 48, 78, 52, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Thinking() {
  display.clearDisplay();
  display.fillRoundRect(35, 22, 18, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(83, 22, 18, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
  display.fillCircle(105, 15, 2, SSD1306_WHITE);
  display.fillCircle(110, 10, 3, SSD1306_WHITE);
  display.fillCircle(115, 5, 4, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Dead() {
  display.clearDisplay();
  display.drawLine(32, 22, 48, 34, SSD1306_WHITE);
  display.drawLine(48, 22, 32, 34, SSD1306_WHITE);
  display.drawLine(80, 22, 96, 34, SSD1306_WHITE);
  display.drawLine(96, 22, 80, 34, SSD1306_WHITE);
  display.fillRoundRect(58, 46, 12, 14, 3, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Surprised() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 28);
  display.fillCircle(64, 50, 6, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawEmotionFace(EmotionState emotion) {
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

void DisplayManager::performTransition() {
  if (!emotionManager.isTransitionActive()) return;
  
  int transitionFrame = emotionManager.getTransitionFrame();
  
  switch(transitionFrame) {
    case 0:
      // Frame 1: Current emotion
      drawEmotionFace(emotionManager.getCurrentEmotion());
      delay(80);
      emotionManager.advanceTransition();
      break;
      
    case 1:
      // Frame 2: Squint eyes (preparing to blink)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 12);
      display.display();
      delay(60);
      emotionManager.advanceTransition();
      break;
      
    case 2:
      // Frame 3: Eyes mostly closed
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 6);
      display.display();
      delay(60);
      emotionManager.advanceTransition();
      break;
      
    case 3:
      // Frame 4: Eyes fully closed (blink)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 3);
      display.display();
      delay(80);
      emotionManager.advanceTransition();
      break;
      
    case 4:
      // Frame 5: Eyes starting to open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 8);
      display.display();
      delay(60);
      emotionManager.advanceTransition();
      break;
      
    case 5:
      // Frame 6: Eyes half open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 14);
      display.display();
      delay(60);
      emotionManager.advanceTransition();
      break;
      
    case 6:
      // Frame 7: Target emotion fully revealed
      drawEmotionFace(emotionManager.getTargetEmotion());
      delay(80);
      emotionManager.completeTransition();
      break;
  }
}
