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

void DisplayManager::drawBatSignal() {
  display.clearDisplay();
  
  // Batman signal - centered with sharp, angular wings
  // Center point for the bat (x=64, y=32)
  int cx = 64;
  int cy = 32;
  
  // Top bat ears (sharp pointed)
  display.fillTriangle(cx - 5, cy - 14, cx - 1, cy - 6, cx - 9, cy - 6, SSD1306_WHITE);
  display.fillTriangle(cx + 5, cy - 14, cx + 1, cy - 6, cx + 9, cy - 6, SSD1306_WHITE);
  
  // Bat head/body center
  display.fillCircle(cx, cy - 4, 6, SSD1306_WHITE);
  display.fillRect(cx - 6, cy - 4, 12, 14, SSD1306_WHITE);
  
  // LEFT WING - sharp angular design
  // Upper wing sweep with sharp tip
  display.fillTriangle(cx - 6, cy - 2, cx - 54, cy - 16, cx - 50, cy + 2, SSD1306_WHITE);
  display.fillTriangle(cx - 6, cy - 2, cx - 50, cy + 2, cx - 44, cy - 4, SSD1306_WHITE);
  
  // Middle wing section with sharp scallop
  display.fillTriangle(cx - 6, cy + 2, cx - 50, cy + 2, cx - 46, cy + 12, SSD1306_WHITE);
  display.fillTriangle(cx - 6, cy + 2, cx - 46, cy + 12, cx - 38, cy + 8, SSD1306_WHITE);
  
  // Lower wing with sharp point
  display.fillTriangle(cx - 6, cy + 8, cx - 38, cy + 8, cx - 32, cy + 18, SSD1306_WHITE);
  display.fillTriangle(cx - 6, cy + 8, cx - 32, cy + 18, cx - 24, cy + 14, SSD1306_WHITE);
  
  // RIGHT WING - sharp angular design (mirrored)
  // Upper wing sweep with sharp tip
  display.fillTriangle(cx + 6, cy - 2, cx + 54, cy - 16, cx + 50, cy + 2, SSD1306_WHITE);
  display.fillTriangle(cx + 6, cy - 2, cx + 50, cy + 2, cx + 44, cy - 4, SSD1306_WHITE);
  
  // Middle wing section with sharp scallop
  display.fillTriangle(cx + 6, cy + 2, cx + 50, cy + 2, cx + 46, cy + 12, SSD1306_WHITE);
  display.fillTriangle(cx + 6, cy + 2, cx + 46, cy + 12, cx + 38, cy + 8, SSD1306_WHITE);
  
  // Lower wing with sharp point
  display.fillTriangle(cx + 6, cy + 8, cx + 38, cy + 8, cx + 32, cy + 18, SSD1306_WHITE);
  display.fillTriangle(cx + 6, cy + 8, cx + 32, cy + 18, cx + 24, cy + 14, SSD1306_WHITE);
  
  // Bottom center points (3 sharp points)
  display.fillTriangle(cx - 16, cy + 10, cx - 12, cy + 22, cx - 6, cy + 10, SSD1306_WHITE);
  display.fillTriangle(cx - 3, cy + 10, cx, cy + 24, cx + 3, cy + 10, SSD1306_WHITE);
  display.fillTriangle(cx + 6, cy + 10, cx + 12, cy + 22, cx + 16, cy + 10, SSD1306_WHITE);
  
  display.display();
}

void DisplayManager::showBootScreen() {
  // Flicker effect - black screen first, then bat signal twice
  
  // First flicker: black -> bat -> black
  display.clearDisplay();
  display.display();
  delay(80);
  
  drawBatSignal();
  delay(120);
  
  display.clearDisplay();
  display.display();
  delay(500);
  
  // Second flicker: black -> bat -> black
  display.clearDisplay();
  display.display();
  delay(80);
  
  drawBatSignal();
  delay(120);
  
  display.clearDisplay();
  display.display();
  delay(80);
  
  // Show steady bat signal for 3 seconds
  drawBatSignal();
  delay(3000);
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
  display.fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);  // Smaller mouth
  display.display();
}

void DisplayManager::drawFace_Angry() {
  display.clearDisplay();
  
  // Angry narrowed eyes (smaller, more intense)
  drawEyes(40, 32, 88, 32, 12);
  
  // Left eyebrow - thick angled line slanting down toward center
  for(int i = 0; i < 5; i++) {
    display.drawLine(22, 16 + i, 52, 22 + i, SSD1306_WHITE);
  }
  
  // Right eyebrow - thick angled line slanting down toward center  
  for(int i = 0; i < 5; i++) {
    display.drawLine(76, 22 + i, 106, 16 + i, SSD1306_WHITE);
  }
  
  // Simple frown mouth (small horizontal line low on face)
  display.fillRoundRect(52, 50, 24, 5, 2, SSD1306_WHITE);
  
  display.display();
}

void DisplayManager::drawFace_Love() {
  display.clearDisplay();
  // Left heart eye
  display.fillCircle(34, 26, 7, SSD1306_WHITE);      // Left circle
  display.fillCircle(44, 26, 7, SSD1306_WHITE);      // Right circle
  display.fillRect(27, 26, 24, 6, SSD1306_WHITE);    // Connect circles
  display.fillTriangle(27, 32, 39, 40, 51, 32, SSD1306_WHITE);  // Bottom point
  
  // Right heart eye
  display.fillCircle(82, 26, 7, SSD1306_WHITE);      // Left circle
  display.fillCircle(92, 26, 7, SSD1306_WHITE);      // Right circle
  display.fillRect(75, 26, 24, 6, SSD1306_WHITE);    // Connect circles
  display.fillTriangle(75, 32, 87, 40, 99, 32, SSD1306_WHITE);  // Bottom point
  
  display.fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Sleepy() {
  display.clearDisplay();
  drawEyes(40, 30, 88, 30, 8);
  display.drawCircle(64, 48, 6, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(98, 20);
  display.print("z");
  display.setCursor(105, 14);
  display.print("z");
  display.setCursor(112, 10);
  display.print("Z");
  display.setCursor(118, 6);
  display.print("z");
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
  // Simple filled mouth (24px wide, matches other emotions)
  display.fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
  // Question mark to show confusion
  display.setTextSize(2);
  display.setCursor(108, 20);
  display.print("?");
  display.display();
}

void DisplayManager::drawFace_Thinking() {
  display.clearDisplay();
  // Normal eyes (original size)
  display.fillRoundRect(35, 22, 18, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(83, 22, 18, 20, 5, SSD1306_WHITE);
  display.fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
  // Exclamation mark on the right
  display.setTextSize(2);
  display.setCursor(108, 20);
  display.print("!");
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
  // Wide eyes with centered pupils (eyes: y=18, height=28, center y = 32)
  display.fillRoundRect(33, 18, 18, 28, 5, SSD1306_WHITE);
  display.fillRoundRect(81, 18, 18, 28, 5, SSD1306_WHITE);
  display.fillCircle(42, 32, 3, SSD1306_BLACK);  // Left pupil centered
  display.fillCircle(90, 32, 3, SSD1306_BLACK);  // Right pupil centered
  display.fillCircle(64, 50, 6, SSD1306_WHITE);
  display.display();
}

void DisplayManager::drawFace_Notification(const char* title, const char* message) {
  display.clearDisplay();

  // ===== NOTIFICATION BOX ONLY (No peeking eyes) =====
  // The peeking eyes only appear during the animateNotification() animation
  // This static face just shows the notification content box
  
  int boxX = 6;
  int boxY = 10;  // Centered vertically on 64px screen
  int boxWidth = 116;  // Fits in 128px screen with margin
  int boxHeight = 44;  // Taller box for better content display

  // Draw double border for emphasis
  display.drawRect(boxX, boxY, boxWidth, boxHeight, SSD1306_WHITE);
  display.drawRect(boxX + 1, boxY + 1, boxWidth - 2, boxHeight - 2, SSD1306_WHITE);

  // ===== TEXT CONTENT (WITH PROPER PADDING) =====
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Calculate safe text area (inside double border with padding)
  int textX = boxX + 6;  // 6px from left edge (beyond double border + padding)
  int textY_title = boxY + 8;  // 8px from top edge
  int textY_message = boxY + 22;  // Below title with spacing
  int maxTextWidth = boxWidth - 12;  // Leave 6px padding on each side

  // Calculate max characters (6px per char in size 1 font)
  int maxChars = (maxTextWidth - 2) / 6;  // Subtract 2px for bold effect

  // Title (bold effect by double printing) - bounds check
  if (title && strlen(title) > 0) {
    // Truncate title if too long (reserve space for null terminator)
    char truncTitle[20];
    int copyLen = (strlen(title) < maxChars) ? strlen(title) : maxChars;
    strncpy(truncTitle, title, copyLen);
    truncTitle[copyLen] = '\0';
    
    display.setCursor(textX, textY_title);
    display.print(truncTitle);
    display.setCursor(textX + 1, textY_title);  // Bold effect
    display.print(truncTitle);
  }

  // Message text - bounds check
  if (message && strlen(message) > 0) {
    // Truncate message if too long (reserve space for null terminator)
    char truncMessage[20];
    int copyLen = (strlen(message) < maxChars) ? strlen(message) : maxChars;
    strncpy(truncMessage, message, copyLen);
    truncMessage[copyLen] = '\0';
    
    display.setCursor(textX, textY_message);
    display.print(truncMessage);
  }

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
    case EMOTION_NOTIFICATION:
      drawFace_Notification("", "");  // Default empty notification
      break;
    default:
      drawFace_Normal();
      break;
  }
}

void DisplayManager::performTransition() {
  if (!emotionManager.isTransitionActive()) return;
  
  int transitionFrame = emotionManager.getTransitionFrame();
  EmotionState currentEmotion = emotionManager.getCurrentEmotion();
  EmotionState targetEmotion = emotionManager.getTargetEmotion();
  
  // Special handling for sleepy transitions
  if (currentEmotion == EMOTION_SLEEPY || targetEmotion == EMOTION_SLEEPY) {
    performSleepyTransition(transitionFrame, targetEmotion);
    return;
  }
  
  // Special handling ONLY when transitioning TO notification (surprise → run away → notification appears)
  // When transitioning FROM notification to other emotions, use standard transition
  if (targetEmotion == EMOTION_NOTIFICATION && currentEmotion != EMOTION_NOTIFICATION) {
    performNotificationTransition(transitionFrame, currentEmotion);
    return;
  }
  
  // Standard transition for all other emotions
  switch(transitionFrame) {
    case 0:
      // Frame 1: Current emotion
      drawEmotionFace(currentEmotion);
      delay(200);
      emotionManager.advanceTransition();
      break;
      
    case 1:
      // Frame 2: Squint eyes (preparing to blink)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 12);
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 2:
      // Frame 3: Eyes mostly closed
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 6);
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 3:
      // Frame 4: Eyes fully closed (blink)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 3);
      display.display();
      delay(200);
      emotionManager.advanceTransition();
      break;
      
    case 4:
      // Frame 5: Eyes starting to open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 8);
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 5:
      // Frame 6: Eyes half open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 14);
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 6:
      // Frame 7: Target emotion fully revealed
      drawEmotionFace(targetEmotion);
      delay(200);
      emotionManager.completeTransition();
      break;
  }
}

// Special transition handler for sleepy emotion (uses round mouth)
void DisplayManager::performSleepyTransition(int transitionFrame, EmotionState targetEmotion) {
  switch(transitionFrame) {
    case 0:
      // Frame 1: Current emotion
      drawEmotionFace(emotionManager.getCurrentEmotion());
      delay(200);
      emotionManager.advanceTransition();
      break;
      
    case 1:
      // Frame 2: Squint eyes (preparing to blink) with round mouth
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 12);
      display.drawCircle(64, 48, 5, SSD1306_WHITE);  // Round mouth for sleepy
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 2:
      // Frame 3: Eyes mostly closed with round mouth
      display.clearDisplay();
      drawEyes(40, 29, 88, 29, 8);
      display.drawCircle(64, 48, 6, SSD1306_WHITE);  // Round mouth
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 3:
      // Frame 4: Eyes fully closed (blink) with round mouth
      display.clearDisplay();
      drawEyes(40, 30, 88, 30, 4);
      display.drawCircle(64, 48, 6, SSD1306_WHITE);  // Round mouth
      display.display();
      delay(200);
      emotionManager.advanceTransition();
      break;
      
    case 4:
      // Frame 5: Eyes starting to open with round mouth
      display.clearDisplay();
      drawEyes(40, 29, 88, 29, 8);
      display.drawCircle(64, 48, 6, SSD1306_WHITE);  // Round mouth
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 5:
      // Frame 6: Eyes half open
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 12);
      display.drawCircle(64, 48, 5, SSD1306_WHITE);  // Round mouth
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 6:
      // Frame 7: Target emotion fully revealed
      drawEmotionFace(targetEmotion);
      delay(200);
      emotionManager.completeTransition();
      break;
  }
}

// Special transition handler for notification emotion (surprise → run away → notification appears)
void DisplayManager::performNotificationTransition(int transitionFrame, EmotionState currentEmotion) {
  switch(transitionFrame) {
    case 0:
      // Frame 1: Current emotion (normal idle state)
      drawEmotionFace(currentEmotion);
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 1:
      // Frame 2: Eyes start widening (notification alert!)
      display.clearDisplay();
      drawEyes(40, 27, 88, 27, 22);
      display.fillCircle(40, 27, 2, SSD1306_BLACK);
      display.fillCircle(88, 27, 2, SSD1306_BLACK);
      display.drawCircle(64, 48, 6, SSD1306_WHITE);
      display.display();
      delay(100);
      emotionManager.advanceTransition();
      break;
      
    case 2:
      // Frame 3: Eyes WIDE (startled!)
      display.clearDisplay();
      drawEyes(40, 26, 88, 26, 26);
      display.fillCircle(40, 26, 3, SSD1306_BLACK);
      display.fillCircle(88, 26, 3, SSD1306_BLACK);
      display.fillCircle(64, 50, 8, SSD1306_WHITE);
      display.display();
      delay(150);
      emotionManager.advanceTransition();
      break;
      
    case 3:
      // Frame 4: Eyes squinting (preparing to run!)
      display.clearDisplay();
      drawEyes(40, 28, 88, 28, 12);
      display.drawLine(52, 50, 76, 50, SSD1306_WHITE);
      display.display();
      delay(100);
      emotionManager.advanceTransition();
      break;
      
    case 4:
      // Frame 5: Start moving right (running away)
      display.clearDisplay();
      drawEyes(60, 28, 108, 28, 14);
      display.drawLine(72, 50, 96, 50, SSD1306_WHITE);
      display.display();
      delay(100);
      emotionManager.advanceTransition();
      break;
      
    case 5:
      // Frame 6: Almost off-screen
      display.clearDisplay();
      drawEyes(85, 28, 133, 28, 14);  // Partially off-screen
      display.display();
      delay(100);
      emotionManager.advanceTransition();
      break;
      
    case 6:
      // Frame 7: Notification appears (target emotion fully revealed)
      drawEmotionFace(EMOTION_NOTIFICATION);
      delay(200);
      emotionManager.completeTransition();
      break;
  }
}
