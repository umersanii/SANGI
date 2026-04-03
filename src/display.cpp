#include "display.h"

DisplayManager displayManager;

// Constructs the Adafruit_SSD1306 instance with configured screen dimensions and I2C reset pin.
DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

// Initializes I2C bus, scans for devices, and starts the SSD1306 OLED. Returns false if init fails.
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

// Scans all I2C addresses and logs any detected devices to Serial.
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

// Draws the confused awake face used as the held final boot frame.
void DisplayManager::drawBootFace() {
  this->clear();
  // Left eye tall (wide awake), right eye squinting (still half-asleep)
  this->fillRoundRect(26, 9,  24, 26, 7, COLOR_WHITE);   // left: tall
  this->fillRoundRect(78, 23, 24, 12, 7, COLOR_WHITE);   // right: squat
  // Small flat neutral mouth
  this->drawMouth(60, 53, 12, 4);
  // "?" drifting up top-right
  this->setTextColor(COLOR_WHITE);
  this->setTextSize(1);
  this->setCursor(110, 8);
  this->print("?");
  this->flush();
}

// Waking-up boot animation:
//   Phase 0 — black (300ms)
//   Phase 1 — slit eyes, no mouth (asleep, 600ms)
//   Phase 2 — flutter x2 (stirring, ~580ms)
//   Phase 3 — eyes ease open unevenly, groggy (6 steps, ~360ms)
//   Phase 4 — hold confused face + "?" appears (900ms)
void DisplayManager::showBootScreen() {
  // Phase 0: black
  this->clear();
  this->flush();
  delay(300);

  // Phase 1: asleep — slit eyes, no mouth
  this->clear();
  this->drawEyes(38, 32, 90, 32, 2);
  this->flush();
  delay(600);

  // Phase 2: stir — flutter twice
  for (int i = 0; i < 2; i++) {
    this->clear();
    this->drawEyes(38, 31, 90, 31, 10);
    this->flush();
    delay(140);

    this->clear();
    this->drawEyes(38, 32, 90, 32, 2);
    this->flush();
    delay(150);
  }

  // Phase 3: open unevenly — left opens fast (→H=26), right drags behind (→H=12)
  for (int step = 1; step <= 6; step++) {
    int leftH  = 2 + (24 * step / 6);   // 2 → 26
    int leftY  = 22 - leftH / 2;
    int rightH = 2 + (10 * step / 6);   // 2 → 12
    int rightY = 29 - rightH / 2;
    this->clear();
    this->fillRoundRect(26, leftY,  24, leftH,  7, COLOR_WHITE);
    this->fillRoundRect(78, rightY, 24, rightH, 7, COLOR_WHITE);
    this->flush();
    delay(60);
  }

  // Phase 4: hold confused — flat mouth then "?" pops in
  this->clear();
  this->fillRoundRect(26, 9,  24, 26, 7, COLOR_WHITE);
  this->fillRoundRect(78, 23, 24, 12, 7, COLOR_WHITE);
  this->drawMouth(60, 53, 12, 4);
  this->flush();
  delay(400);

  // "?" pops in
  drawBootFace();
  delay(900);

  // Blink-out into IDLE — reuses the standard transition timing so the seam is invisible.
  // Frame 1-3: close eyes from confused face
  display.clearDisplay(); drawEyes(38, 28, 90, 28, 16); display.display(); delay(100);
  display.clearDisplay(); drawEyes(38, 29, 90, 29, 10); display.display(); delay(100);
  display.clearDisplay(); drawEyes(38, 30, 90, 30,  4); display.display(); delay(200);
  // Frame 4-5: open eyes symmetrically
  display.clearDisplay(); drawEyes(38, 29, 90, 29, 10); display.display(); delay(120);
  display.clearDisplay(); drawEyes(38, 28, 90, 28, 18); display.display(); delay(120);
  // Frame 6: land on IDLE — loop() picks up from here
  drawEmotionFace(EMOTION_IDLE);
  delay(200);
}

// --- ICanvas implementation (delegates to Adafruit_SSD1306) ---

// Clears the display buffer.
void DisplayManager::clear() { display.clearDisplay(); }

// Flushes the display buffer to the OLED hardware.
void DisplayManager::flush() { display.display(); }

// Draws a filled rounded rectangle.
void DisplayManager::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                   int16_t r, uint16_t color) {
  display.fillRoundRect(x, y, w, h, r, color);
}

// Draws an outlined rounded rectangle.
void DisplayManager::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                   int16_t r, uint16_t color) {
  display.drawRoundRect(x, y, w, h, r, color);
}

// Draws a filled rectangle.
void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  display.fillRect(x, y, w, h, color);
}

// Draws an outlined rectangle.
void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  display.drawRect(x, y, w, h, color);
}

// Draws a filled circle.
void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r,
                                 uint16_t color) {
  display.fillCircle(x, y, r, color);
}

// Draws an outlined circle.
void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r,
                                 uint16_t color) {
  display.drawCircle(x, y, r, color);
}

// Draws a line between two points.
void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                               uint16_t color) {
  display.drawLine(x0, y0, x1, y1, color);
}

// Draws a filled triangle.
void DisplayManager::fillTriangle(int16_t x0, int16_t y0, int16_t x1,
                                   int16_t y1, int16_t x2, int16_t y2,
                                   uint16_t color) {
  display.fillTriangle(x0, y0, x1, y1, x2, y2, color);
}

// Sets the text render size multiplier.
void DisplayManager::setTextSize(uint8_t size) { display.setTextSize(size); }

// Sets the text cursor position.
void DisplayManager::setCursor(int16_t x, int16_t y) {
  display.setCursor(x, y);
}

// Sets the text foreground color.
void DisplayManager::setTextColor(uint16_t color) {
  display.setTextColor(color);
}

// Prints a null-terminated string at the current cursor position.
void DisplayManager::print(const char* text) { display.print(text); }

// Prints a null-terminated string followed by a newline.
void DisplayManager::println(const char* text) { display.println(text); }

// drawEyes is now inherited from ICanvas (concrete helper calling fillRoundRect)

// Draws the neutral idle face: two standard-size eyes, no mouth decoration.
void DisplayManager::drawFace_Normal() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 20);
  display.display();
}

// Draws the happy face: slightly squinted eyes and a wide smile bar.
void DisplayManager::drawFace_Happy() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 16);
  display.fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
  display.display();
}

// Draws the blink face: eyes nearly shut (height=4), used during transition animations.
void DisplayManager::drawFace_Blink() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 4);
  display.display();
}

// Draws the sad face: normal eyes with a narrow frown bar.
void DisplayManager::drawFace_Sad() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 20);
  display.fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);  // Smaller mouth
  display.display();
}

// Draws the angry face: narrowed eyes, thick angled brows, and a flat frown.
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

// Draws the love face: heart-shaped eyes and a wide smile bar.
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

// Draws the sleepy face: half-closed eyes, a yawn circle, and floating Z letters.
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

// Draws the excited face: wide eyes with pupils and a large open mouth.
void DisplayManager::drawFace_Excited() {
  display.clearDisplay();
  drawEyes(40, 28, 88, 28, 24);
  display.fillCircle(40, 28, 3, SSD1306_BLACK);
  display.fillCircle(88, 28, 3, SSD1306_BLACK);
  display.fillRoundRect(45, 50, 38, 10, 5, SSD1306_WHITE);
  display.display();
}

// Draws the confused face: asymmetric eyes (one tall, one short) and a question mark.
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

// Draws the thinking face: normal eyes with a small mouth and an exclamation mark.
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

// Draws the dead face: X-shaped eyes and a small rectangular tongue.
void DisplayManager::drawFace_Dead() {
  display.clearDisplay();
  display.drawLine(32, 22, 48, 34, SSD1306_WHITE);
  display.drawLine(48, 22, 32, 34, SSD1306_WHITE);
  display.drawLine(80, 22, 96, 34, SSD1306_WHITE);
  display.drawLine(96, 22, 80, 34, SSD1306_WHITE);
  display.fillRoundRect(58, 46, 12, 14, 3, SSD1306_WHITE);
  display.display();
}

// Draws the surprised face: wide eyes with centered pupils and a circular open mouth.
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

// Draws a static notification box with title and message text; no animated eyes in this frame.
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

// Dispatches to the appropriate static drawFace_* function based on the given emotion state.
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
    case EMOTION_BORED:
      drawFace_Normal();  // half-lid bored is handled by animation; Normal is fine for 200ms transition frame
      break;
    case EMOTION_BLINK:
      drawFace_Blink();
      break;
    default:
      drawFace_Normal();
      break;
  }
}

// Transition animation — decoupled from EmotionManager.
// Returns TR_DREW_FRAME (caller should advance) or TR_COMPLETE (caller should complete).
TransitionResult DisplayManager::performTransitionFrame(int frame,
                                                         EmotionState current,
                                                         EmotionState target) {
  // Special handling for sleepy transitions
  if (current == EMOTION_SLEEPY || target == EMOTION_SLEEPY) {
    return sleepyTransitionFrame(frame, target);
  }

  // Standard 7-frame blink transition — new face grammar (eyes at 38/90, w=24, r=7)
  // Close: 22→16→10→4 (fast, eased). Open: 4→10→18 (slightly slower). Total ~1040ms.
  switch (frame) {
    case 0:
      drawEmotionFace(current);
      delay(200);
      return TR_DREW_FRAME;
    case 1:
      display.clearDisplay();
      drawEyes(38, 28, 90, 28, 16);
      display.display();
      delay(100);
      return TR_DREW_FRAME;
    case 2:
      display.clearDisplay();
      drawEyes(38, 29, 90, 29, 10);
      display.display();
      delay(100);
      return TR_DREW_FRAME;
    case 3:
      display.clearDisplay();
      drawEyes(38, 30, 90, 30, 4);
      display.display();
      delay(200);
      return TR_DREW_FRAME;
    case 4:
      display.clearDisplay();
      drawEyes(38, 29, 90, 29, 10);
      display.display();
      delay(120);
      return TR_DREW_FRAME;
    case 5:
      display.clearDisplay();
      drawEyes(38, 28, 90, 28, 18);
      display.display();
      delay(120);
      return TR_DREW_FRAME;
    case 6:
      drawEmotionFace(target);
      delay(200);
      return TR_COMPLETE;
  }
  return TR_COMPLETE;
}

// Sleepy transition — same as standard but retains yawn mouth circle
TransitionResult DisplayManager::sleepyTransitionFrame(int frame,
                                                        EmotionState target) {
  switch (frame) {
    case 0:
      drawEmotionFace(EMOTION_SLEEPY);
      delay(200);
      return TR_DREW_FRAME;
    case 1:
      display.clearDisplay();
      drawEyes(38, 28, 90, 28, 16);
      display.drawCircle(64, 50, 5, SSD1306_WHITE);
      display.display();
      delay(100);
      return TR_DREW_FRAME;
    case 2:
      display.clearDisplay();
      drawEyes(38, 29, 90, 29, 10);
      display.drawCircle(64, 51, 6, SSD1306_WHITE);
      display.display();
      delay(100);
      return TR_DREW_FRAME;
    case 3:
      display.clearDisplay();
      drawEyes(38, 30, 90, 30, 4);
      display.drawCircle(64, 52, 7, SSD1306_WHITE);
      display.display();
      delay(200);
      return TR_DREW_FRAME;
    case 4:
      display.clearDisplay();
      drawEyes(38, 29, 90, 29, 10);
      display.drawCircle(64, 51, 6, SSD1306_WHITE);
      display.display();
      delay(120);
      return TR_DREW_FRAME;
    case 5:
      display.clearDisplay();
      drawEyes(38, 28, 90, 28, 18);
      display.drawCircle(64, 50, 5, SSD1306_WHITE);
      display.display();
      delay(120);
      return TR_DREW_FRAME;
    case 6:
      drawEmotionFace(target);
      delay(200);
      return TR_COMPLETE;
  }
  return TR_COMPLETE;
}
