#include "animations.h"
#include "display.h"

AnimationManager animationManager;

AnimationManager::AnimationManager()
  : lastSleepyAnim(0),
    lastThinkAnim(0),
    lastExciteAnim(0),
    lastConfuseAnim(0),
    lastHappyAnim(0),
    lastLoveAnim(0),
    lastAngryAnim(0),
    lastSadAnim(0),
    lastSurprisedAnim(0),
    sleepyFrame(0),
    thinkFrame(0),
    exciteFrame(0),
    confuseFrame(0),
    happyFrame(0),
    loveFrame(0),
    angryFrame(0),
    sadFrame(0),
    surprisedFrame(0) {
}

// Animated sleepy state - smooth drowsy animations with synchronized mouth
void AnimationManager::animateSleepy() {
  unsigned long currentTime = millis();
  
  // Animate every 30ms to match blinking animation frame rate
  if (currentTime - lastSleepyAnim > 30) {
    displayManager.clearDisplay();
    
    switch(sleepyFrame) {
      // === CLOSING SEQUENCE (frames 0-8) ===
      case 0:
        // Eyes fully open - normal idle mouth
        displayManager.drawEyes(40, 28, 88, 28, 20);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes start drooping
        displayManager.drawEyes(40, 29, 88, 29, 16);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 2:
        // Eyes more droopy
        displayManager.drawEyes(40, 29, 88, 29, 12);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 3:
        // Eyes getting heavy
        displayManager.drawEyes(40, 30, 88, 30, 10);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 4:
        // Eyes barely open - mouth starting to open
        displayManager.drawEyes(40, 30, 88, 30, 8);
        displayManager.getDisplay().drawCircle(64, 48, 6, SSD1306_WHITE);
        break;
        
      case 5:
        // Eyes almost closed - mouth opening more
        displayManager.drawEyes(40, 31, 88, 31, 6);
        displayManager.getDisplay().drawCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 6:
        // Eyes nearly shut - mouth transitioning to filled
        displayManager.drawEyes(40, 31, 88, 31, 4);
        displayManager.getDisplay().fillCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 7:
        // Eyes closing - mouth fully open round
        displayManager.drawEyes(40, 31, 88, 31, 3);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        break;
        
      case 8:
        // Eyes fully closed - mouth round, zzz appear
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(105, 12);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(110, 8);
        displayManager.getDisplay().print("Z");
        break;
        
      // === STAY CLOSED (frames 9-42) - approximately 1 second at 30ms per frame ===
      case 9: case 10: case 11: case 12: case 13:
      case 14: case 15: case 16: case 17: case 18:
      case 19: case 20: case 21: case 22: case 23:
      case 24: case 25: case 26: case 27: case 28:
      case 29: case 30: case 31: case 32: case 33:
        // Eyes stay closed, mouth round, zzz active
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(105, 12);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(110, 8);
        displayManager.getDisplay().print("Z");
        displayManager.getDisplay().setCursor(115, 4);
        displayManager.getDisplay().print("Z");
        break;
        
      case 34: case 35: case 36: case 37: case 38:
      case 39: case 40: case 41: case 42:
        // Eyes stay closed, mouth round, zzz active (variation)
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(106, 10);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(111, 6);
        displayManager.getDisplay().print("Z");
        displayManager.getDisplay().setCursor(116, 2);
        displayManager.getDisplay().print("Z");
        break;
        
      // === OPENING SEQUENCE (frames 43-50) ===
      case 43:
        // Eyes start opening - zzz fading
        displayManager.drawEyes(40, 31, 88, 31, 3);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(110, 8);
        displayManager.getDisplay().print("Z");
        break;
        
      case 44:
        // Eyes opening more - mouth transitioning
        displayManager.drawEyes(40, 31, 88, 31, 4);
        displayManager.getDisplay().fillCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 45:
        // Eyes opening - mouth becoming circle outline
        displayManager.drawEyes(40, 31, 88, 31, 6);
        displayManager.getDisplay().drawCircle(64, 49, 7, SSD1306_WHITE);
        break;
        
      case 46:
        // Eyes half open
        displayManager.drawEyes(40, 30, 88, 30, 8);
        displayManager.getDisplay().drawCircle(64, 48, 6, SSD1306_WHITE);
        break;
        
      case 47:
        // Eyes more open
        displayManager.drawEyes(40, 30, 88, 30, 10);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 48:
        // Eyes almost fully open
        displayManager.drawEyes(40, 29, 88, 29, 14);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 49:
        // Eyes fully open - back to normal idle mouth
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 50:
        // Eyes fully open - normal idle mouth
        displayManager.drawEyes(40, 28, 88, 28, 20);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    sleepyFrame = (sleepyFrame + 1) % 51;
    lastSleepyAnim = currentTime;
  }
}

// Animated thinking state - contemplative eye movements and floating thought bubbles
void AnimationManager::animateThinking() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastThinkAnim > 30) {
    displayManager.clearDisplay();
    
    switch(thinkFrame) {
      // === INITIAL PONDERING (frames 0-8) ===
      case 0:
        // Eyes normal, looking forward
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes start looking up-left
        displayManager.drawEyes(38, 26, 86, 26, 18);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        break;
        
      case 3: case 4: case 5:
        // Eyes fully looking up
        displayManager.drawEyes(37, 24, 85, 24, 18);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        // First thought bubble appears
        displayManager.getDisplay().fillCircle(100, 20, 2, SSD1306_WHITE);
        break;
        
      case 6: case 7: case 8:
        // Eyes looking up, eyebrows appear (concentrated look)
        displayManager.drawEyes(37, 24, 85, 24, 18);
        displayManager.getDisplay().drawLine(30, 18, 50, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 16, 98, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        // Thought bubbles growing
        displayManager.getDisplay().fillCircle(102, 18, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(108, 14, 3, SSD1306_WHITE);
        break;
        
      // === DEEP THINKING (frames 9-35) ===
      case 9: case 10: case 11: case 12: case 13:
      case 14: case 15: case 16: case 17: case 18:
        // Maintain thinking pose, bubbles floating
        displayManager.drawEyes(37, 24, 85, 24, 18);
        displayManager.getDisplay().drawLine(30, 18, 50, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 16, 98, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        // Thought bubble chain grows and animates
        displayManager.getDisplay().fillCircle(104, 16, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(110, 12, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(116, 8, 4, SSD1306_WHITE);
        if (thinkFrame % 4 < 2) {
          displayManager.getDisplay().fillCircle(122, 4, 3, SSD1306_WHITE);
        }
        break;
        
      case 19: case 20: case 21: case 22: case 23:
      case 24: case 25: case 26: case 27: case 28:
        // Eyes shift slightly (contemplating)
        displayManager.drawEyes(39, 24, 87, 24, 18);
        displayManager.getDisplay().drawLine(32, 18, 52, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(76, 16, 96, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        // Bubbles continue
        displayManager.getDisplay().fillCircle(106, 14, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(112, 10, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(118, 6, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(124, 3, 4, SSD1306_WHITE);
        break;
        
      case 29: case 30: case 31: case 32: case 33:
      case 34: case 35:
        // Eyes looking other direction
        displayManager.drawEyes(35, 24, 83, 24, 18);
        displayManager.getDisplay().drawLine(28, 18, 48, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(80, 16, 100, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        // More thought bubbles
        displayManager.getDisplay().fillCircle(108, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(114, 8, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(120, 4, 4, SSD1306_WHITE);
        break;
        
      // === REALIZATION MOMENT (frames 36-42) ===
      case 36: case 37: case 38:
        // Eyes widen slightly (idea forming)
        displayManager.drawEyes(37, 25, 85, 25, 20);
        displayManager.getDisplay().drawLine(30, 18, 50, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 16, 98, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 6, 2, SSD1306_WHITE);
        // Big thought cloud
        displayManager.getDisplay().fillCircle(110, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(118, 8, 4, SSD1306_WHITE);
        break;
        
      case 39: case 40: case 41: case 42:
        // Eyes fully open (eureka moment building)
        displayManager.drawEyes(38, 26, 86, 26, 22);
        displayManager.getDisplay().drawLine(30, 18, 50, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 16, 98, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 6, 2, SSD1306_WHITE);
        // Thought cloud with idea symbol
        displayManager.getDisplay().fillCircle(112, 8, 6, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setTextColor(SSD1306_BLACK);
        displayManager.getDisplay().setCursor(110, 5);
        displayManager.getDisplay().print("!");
        displayManager.getDisplay().setTextColor(SSD1306_WHITE);
        break;
        
      // === RETURN TO NEUTRAL (frames 43-50) ===
      case 43:
        displayManager.drawEyes(39, 27, 87, 27, 20);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(115, 10, 3, SSD1306_WHITE);
        break;
        
      case 44:
        displayManager.drawEyes(40, 27, 88, 27, 19);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(118, 12, 2, SSD1306_WHITE);
        break;
        
      case 45: case 46: case 47: case 48: case 49: case 50:
        // Back to neutral thinking pose
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().fillRoundRect(58, 50, 12, 5, 2, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    thinkFrame = (thinkFrame + 1) % 51;
    lastThinkAnim = currentTime;
  }
}

// Animated excited state - energetic bouncing with sparkles and energy
void AnimationManager::animateExcited() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastExciteAnim > 30) {
    displayManager.clearDisplay();
    
    switch(exciteFrame) {
      // === INITIAL BURST (frames 0-8) ===
      case 0:
        // Eyes normal, mouth opening
        displayManager.drawEyes(40, 28, 88, 28, 20);
        displayManager.getDisplay().fillCircle(40, 28, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 28, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 8, 4, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes growing, mouth opening wider
        displayManager.drawEyes(40, 27, 88, 27, 22);
        displayManager.getDisplay().fillCircle(40, 27, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 27, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(50, 50, 28, 9, 4, SSD1306_WHITE);
        // First sparkles appear
        displayManager.getDisplay().fillCircle(18, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(110, 15, 2, SSD1306_WHITE);
        break;
        
      case 2:
        // Eyes even bigger
        displayManager.drawEyes(40, 26, 88, 26, 24);
        displayManager.getDisplay().fillCircle(40, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(20, 20, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(108, 20, 2, SSD1306_WHITE);
        break;
        
      case 3:
        // Maximum eye size, big smile
        displayManager.drawEyes(40, 26, 88, 26, 26);
        displayManager.getDisplay().fillCircle(40, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(45, 50, 38, 10, 5, SSD1306_WHITE);
        // Energy burst sparkles
        displayManager.getDisplay().fillCircle(12, 10, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(116, 10, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 25, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 25, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(10, 35, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(118, 35, 2, SSD1306_WHITE);
        break;
        
      case 4: case 5: case 6: case 7: case 8:
        // Hold maximum excitement
        displayManager.drawEyes(40, 26, 88, 26, 26);
        displayManager.getDisplay().fillCircle(40, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(45, 50, 38, 10, 5, SSD1306_WHITE);
        // Sparkles all around
        displayManager.getDisplay().fillCircle(12, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(116, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 28, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 28, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(8, 38, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(120, 38, 2, SSD1306_WHITE);
        break;
        
      // === BOUNCING SEQUENCE (frames 9-38) ===
      case 9: case 11: case 13: case 15: case 17:
      case 19: case 21: case 23: case 25: case 27:
      case 29: case 31: case 33: case 35: case 37:
        // Bounce UP position
        displayManager.drawEyes(40, 24, 88, 24, 26);
        displayManager.getDisplay().fillCircle(40, 24, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 24, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(45, 48, 38, 10, 5, SSD1306_WHITE);
        // Sparkles move with bounce
        displayManager.getDisplay().fillCircle(15, 10, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 10, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(12, 30, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(116, 30, 2, SSD1306_WHITE);
        break;
        
      case 10: case 12: case 14: case 16: case 18:
      case 20: case 22: case 24: case 26: case 28:
      case 30: case 32: case 34: case 36: case 38:
        // Bounce DOWN position
        displayManager.drawEyes(40, 28, 88, 28, 26);
        displayManager.getDisplay().fillCircle(40, 28, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 28, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(45, 52, 38, 10, 5, SSD1306_WHITE);
        // Sparkles in different positions
        displayManager.getDisplay().fillCircle(18, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(110, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(10, 38, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(118, 38, 2, SSD1306_WHITE);
        break;
        
      // === SETTLING DOWN (frames 39-50) ===
      case 39:
        displayManager.drawEyes(40, 27, 88, 27, 25);
        displayManager.getDisplay().fillCircle(40, 27, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 27, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(46, 50, 36, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 20, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 20, 2, SSD1306_WHITE);
        break;
        
      case 40:
        displayManager.drawEyes(40, 27, 88, 27, 24);
        displayManager.getDisplay().fillCircle(40, 27, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 27, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(47, 50, 34, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(18, 18, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(110, 18, 2, SSD1306_WHITE);
        break;
        
      case 41:
        displayManager.drawEyes(40, 27, 88, 27, 23);
        displayManager.getDisplay().fillCircle(40, 27, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 27, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 9, 5, SSD1306_WHITE);
        break;
        
      case 42:
        displayManager.drawEyes(40, 28, 88, 28, 22);
        displayManager.getDisplay().fillCircle(40, 28, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 28, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(50, 50, 28, 9, 4, SSD1306_WHITE);
        break;
        
      case 43: case 44: case 45: case 46: case 47:
      case 48: case 49: case 50:
        // Return to calm but still excited state
        displayManager.drawEyes(40, 28, 88, 28, 20);
        displayManager.getDisplay().fillCircle(40, 28, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 28, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 8, 4, SSD1306_WHITE);
        // Occasional sparkle
        if (exciteFrame % 3 == 0) {
          displayManager.getDisplay().fillCircle(15, 20, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(113, 20, 2, SSD1306_WHITE);
        }
        break;
    }
    
    displayManager.updateDisplay();
    exciteFrame = (exciteFrame + 1) % 51;
    lastExciteAnim = currentTime;
  }
}

// Animated confused state - head tilting with alternating eyes and question marks
void AnimationManager::animateConfused() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastConfuseAnim > 30) {
    displayManager.clearDisplay();
    
    switch(confuseFrame) {
      // === INITIAL CONFUSION (frames 0-8) ===
      case 0:
        // Normal eyes, neutral mouth
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawLine(52, 50, 76, 50, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // One eye starts squinting
        displayManager.getDisplay().fillRoundRect(30, 26, 20, 16, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 18, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(52, 50, 76, 50, SSD1306_WHITE);
        break;
        
      case 3: case 4:
        // Eyes become uneven, mouth wavy
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 27, 20, 14, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 48, 58, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 52, 70, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 48, 78, 52, SSD1306_WHITE);
        break;
        
      case 5: case 6: case 7: case 8:
        // Maximum unevenness
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 48, 58, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 52, 70, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 48, 78, 52, SSD1306_WHITE);
        // First question mark appears
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(108, 20);
        displayManager.getDisplay().print("?");
        break;
        
      // === DEEP CONFUSION WITH TILTING (frames 9-25) ===
      case 9: case 10: case 11: case 12: case 13:
      case 14: case 15: case 16:
        // Eyes reversed (other eye bigger)
        displayManager.getDisplay().fillRoundRect(30, 26, 20, 12, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 48, 58, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 52, 70, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 48, 78, 52, SSD1306_WHITE);
        // Question mark grows
        displayManager.getDisplay().setTextSize(2);
        displayManager.getDisplay().setCursor(105, 15);
        displayManager.getDisplay().print("?");
        break;
        
      case 17: case 18: case 19: case 20: case 21:
      case 22: case 23: case 24: case 25:
        // Back to first configuration
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 48, 58, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 52, 70, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 48, 78, 52, SSD1306_WHITE);
        // Multiple question marks
        displayManager.getDisplay().setTextSize(2);
        displayManager.getDisplay().setCursor(105, 10);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(115, 25);
        displayManager.getDisplay().print("?");
        break;
        
      // === MAXIMUM CONFUSION (frames 26-38) ===
      case 26: case 27: case 28: case 29: case 30:
      case 31: case 32: case 33: case 34: case 35:
      case 36: case 37: case 38:
        // Rapid eye alternation
        if (confuseFrame % 4 < 2) {
          displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
          displayManager.getDisplay().fillRoundRect(78, 27, 20, 14, 5, SSD1306_WHITE);
        } else {
          displayManager.getDisplay().fillRoundRect(30, 27, 20, 14, 5, SSD1306_WHITE);
          displayManager.getDisplay().fillRoundRect(78, 24, 20, 20, 5, SSD1306_WHITE);
        }
        // Wavy mouth more pronounced
        displayManager.getDisplay().drawLine(48, 46, 56, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(56, 52, 64, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(64, 48, 72, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(72, 52, 80, 46, SSD1306_WHITE);
        // Question marks everywhere
        displayManager.getDisplay().setTextSize(2);
        displayManager.getDisplay().setCursor(105, 8);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(10, 15);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setCursor(118, 28);
        displayManager.getDisplay().print("?");
        break;
        
      // === SETTLING (frames 39-50) ===
      case 39: case 40:
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 48, 58, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 52, 70, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 48, 78, 52, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(110, 20);
        displayManager.getDisplay().print("?");
        break;
        
      case 41: case 42:
        displayManager.getDisplay().fillRoundRect(32, 25, 20, 18, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 16, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 49, 58, 51, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 51, 70, 49, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 49, 78, 51, SSD1306_WHITE);
        break;
        
      case 43: case 44: case 45: case 46: case 47:
      case 48: case 49: case 50:
        // Return to mildly confused state
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawLine(50, 48, 58, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(58, 52, 70, 48, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(70, 48, 78, 52, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    confuseFrame = (confuseFrame + 1) % 51;
    lastConfuseAnim = currentTime;
  }
}

// Animated happy state - joyful smile with squinting eyes and blush
void AnimationManager::animateHappy() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastHappyAnim > 30) {
    displayManager.clearDisplay();
    
    switch(happyFrame) {
      // === SMILE BUILDING SEQUENCE (frames 0-10) ===
      case 0:
        // Normal eyes, small smile
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes start squinting, smile grows
        displayManager.drawEyes(40, 28, 88, 28, 16);
        displayManager.getDisplay().fillRoundRect(54, 49, 20, 7, 3, SSD1306_WHITE);
        break;
        
      case 2:
        displayManager.drawEyes(40, 28, 88, 28, 14);
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 8, 4, SSD1306_WHITE);
        break;
        
      case 3:
        displayManager.drawEyes(40, 29, 88, 29, 12);
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        break;
        
      case 4:
        // Eyes squinting more, smile widening
        displayManager.drawEyes(40, 29, 88, 29, 11);
        displayManager.getDisplay().fillRoundRect(48, 48, 32, 8, 4, SSD1306_WHITE);
        break;
        
      case 5:
        displayManager.drawEyes(40, 29, 88, 29, 10);
        displayManager.getDisplay().fillRoundRect(46, 48, 36, 9, 4, SSD1306_WHITE);
        // Blush appears
        displayManager.getDisplay().fillCircle(20, 40, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(108, 40, 3, SSD1306_WHITE);
        break;
        
      case 6:
        // Maximum joy expression
        displayManager.drawEyes(40, 30, 88, 30, 8);
        displayManager.getDisplay().fillRoundRect(45, 47, 38, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(19, 40, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(109, 40, 4, SSD1306_WHITE);
        break;
        
      case 7:
      case 8:
      case 9:
      case 10:
        // Hold peak expression with sparkles
        displayManager.drawEyes(40, 30, 88, 30, 8);
        displayManager.getDisplay().fillRoundRect(45, 47, 38, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(18, 40, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(110, 40, 4, SSD1306_WHITE);
        // Sparkles
        if (happyFrame % 2 == 0) {
          displayManager.getDisplay().fillCircle(15, 20, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(113, 20, 2, SSD1306_WHITE);
        }
        break;
        
      // === HOLD JOYFUL STATE (frames 11-40) ===
      case 11: case 12: case 13: case 14: case 15:
      case 16: case 17: case 18: case 19: case 20:
      case 21: case 22: case 23: case 24: case 25:
      case 26: case 27: case 28: case 29: case 30:
        // Maintain happy expression with alternating sparkles
        displayManager.drawEyes(40, 30, 88, 30, 8);
        displayManager.getDisplay().fillRoundRect(45, 47, 38, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(18, 40, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(110, 40, 4, SSD1306_WHITE);
        // Alternating sparkle positions
        if (happyFrame % 4 == 0) {
          displayManager.getDisplay().fillCircle(12, 18, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(116, 18, 2, SSD1306_WHITE);
        } else if (happyFrame % 4 == 2) {
          displayManager.getDisplay().fillCircle(15, 25, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(113, 25, 2, SSD1306_WHITE);
        }
        break;
        
      case 31: case 32: case 33: case 34: case 35:
      case 36: case 37: case 38: case 39: case 40:
        // Continue holding with pulsing blush
        {
          displayManager.drawEyes(40, 30, 88, 30, 8);
          displayManager.getDisplay().fillRoundRect(45, 47, 38, 10, 5, SSD1306_WHITE);
          int blushSize = (happyFrame % 6 < 3) ? 4 : 5;
          displayManager.getDisplay().fillCircle(18, 40, blushSize, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(110, 40, blushSize, SSD1306_WHITE);
        }
        break;
        
      // === RELAXING SEQUENCE (frames 41-50) ===
      case 41:
        displayManager.drawEyes(40, 30, 88, 30, 9);
        displayManager.getDisplay().fillRoundRect(46, 47, 36, 9, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(19, 40, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(109, 40, 4, SSD1306_WHITE);
        break;
        
      case 42:
        displayManager.drawEyes(40, 29, 88, 29, 10);
        displayManager.getDisplay().fillRoundRect(48, 48, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(20, 40, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(108, 40, 3, SSD1306_WHITE);
        break;
        
      case 43:
        displayManager.drawEyes(40, 29, 88, 29, 12);
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(20, 40, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(108, 40, 3, SSD1306_WHITE);
        break;
        
      case 44:
        displayManager.drawEyes(40, 28, 88, 28, 14);
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 8, 4, SSD1306_WHITE);
        break;
        
      case 45:
        displayManager.drawEyes(40, 28, 88, 28, 16);
        displayManager.getDisplay().fillRoundRect(54, 49, 20, 7, 3, SSD1306_WHITE);
        break;
        
      case 46: case 47: case 48: case 49: case 50:
        // Return to calm happy state
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    happyFrame = (happyFrame + 1) % 51;
    lastHappyAnim = currentTime;
  }
}

// Animated love state - pulsing heart eyes with floating hearts and blush
void AnimationManager::animateLove() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastLoveAnim > 30) {
    displayManager.clearDisplay();
    
    switch(loveFrame) {
      // === HEARTS FORMING (frames 0-10) ===
      case 0:
        // Normal eyes, small smile
        displayManager.drawEyes(40, 28, 88, 28, 16);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes transforming to hearts (circles appear)
        displayManager.getDisplay().fillCircle(35, 28, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 28, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 28, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 28, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(50, 50, 28, 7, 3, SSD1306_WHITE);
        break;
        
      case 3: case 4:
        // Hearts growing
        displayManager.getDisplay().fillCircle(35, 27, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 27, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 27, 40, 37, 52, 27, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 27, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 27, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 27, 88, 37, 100, 27, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        break;
        
      case 5: case 6: case 7:
        // Full heart eyes formed
        displayManager.getDisplay().fillCircle(35, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        // Blush appears
        displayManager.getDisplay().fillCircle(15, 42, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 42, 3, SSD1306_WHITE);
        break;
        
      case 8: case 9: case 10:
        // Hearts at max with first floating heart
        displayManager.getDisplay().fillCircle(35, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(14, 42, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(114, 42, 4, SSD1306_WHITE);
        // Small floating heart
        displayManager.getDisplay().drawCircle(108, 18, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(112, 18, 2, SSD1306_WHITE);
        break;
        
      // === PULSING HEARTS (frames 11-35) ===
      case 11: case 13: case 15: case 17: case 19:
      case 21: case 23: case 25: case 27: case 29:
      case 31: case 33: case 35:
        // Hearts pulse larger
        displayManager.getDisplay().fillCircle(35, 26, 9, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 9, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(27, 26, 40, 39, 53, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 9, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 9, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(75, 26, 88, 39, 101, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(14, 42, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(114, 42, 5, SSD1306_WHITE);
        // Multiple floating hearts
        displayManager.getDisplay().drawCircle(106, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(110, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(115, 10, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(119, 10, 2, SSD1306_WHITE);
        break;
        
      case 12: case 14: case 16: case 18: case 20:
      case 22: case 24: case 26: case 28: case 30:
      case 32: case 34:
        // Hearts pulse smaller
        displayManager.getDisplay().fillCircle(35, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(29, 26, 40, 37, 51, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(77, 26, 88, 37, 99, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(14, 42, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(114, 42, 4, SSD1306_WHITE);
        // Different floating hearts positions
        displayManager.getDisplay().drawCircle(108, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(112, 12, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(18, 18, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(22, 18, 2, SSD1306_WHITE);
        break;
        
      // === MAXIMUM LOVE (frames 36-44) ===
      case 36: case 37: case 38: case 39: case 40:
      case 41: case 42: case 43: case 44:
        // Hearts stable at max
        displayManager.getDisplay().fillCircle(35, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(14, 42, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(114, 42, 5, SSD1306_WHITE);
        // Hearts floating all around
        if (loveFrame % 2 == 0) {
          displayManager.getDisplay().drawCircle(105, 10, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(109, 10, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(115, 5, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(119, 5, 2, SSD1306_WHITE);
        } else {
          displayManager.getDisplay().drawCircle(18, 15, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(22, 15, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(12, 8, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(16, 8, 2, SSD1306_WHITE);
        }
        break;
        
      // === CALMING (frames 45-50) ===
      case 45: case 46:
        displayManager.getDisplay().fillCircle(35, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 42, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 42, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(110, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(114, 15, 2, SSD1306_WHITE);
        break;
        
      case 47: case 48: case 49: case 50:
        // Back to calm love state
        displayManager.getDisplay().fillCircle(35, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(45, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 26, 40, 38, 52, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(83, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(93, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 26, 88, 38, 100, 26, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 42, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 42, 3, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    loveFrame = (loveFrame + 1) % 51;
    lastLoveAnim = currentTime;
  }
}

// Animated angry state - intense shaking with lowering eyebrows and steam
void AnimationManager::animateAngry() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastAngryAnim > 30) {
    displayManager.clearDisplay();
    
    switch(angryFrame) {
      // === ANGER BUILDING (frames 0-10) ===
      case 0:
        // Normal but narrowed eyes
        displayManager.drawEyes(40, 29, 88, 29, 14);
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 5, 2, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes narrowing, eyebrows appear
        displayManager.drawEyes(40, 30, 88, 30, 12);
        displayManager.getDisplay().drawLine(28, 20, 32, 24, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(32, 24, 50, 20, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 20, 96, 24, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(96, 24, 100, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(54, 50, 20, 5, 2, SSD1306_WHITE);
        break;
        
      case 3: case 4: case 5:
        // Eyebrows lowering, eyes more narrow
        displayManager.drawEyes(40, 30, 88, 30, 11);
        displayManager.getDisplay().fillTriangle(26, 19, 31, 23, 50, 19, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(78, 19, 97, 23, 102, 19, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(54, 50, 20, 6, 3, SSD1306_WHITE);
        // First anger lines
        displayManager.getDisplay().drawLine(14, 26, 19, 30, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(109, 26, 114, 30, SSD1306_WHITE);
        break;
        
      case 6: case 7: case 8: case 9: case 10:
        // Full angry expression
        displayManager.drawEyes(40, 30, 88, 30, 10);
        displayManager.getDisplay().fillTriangle(25, 18, 30, 22, 50, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(78, 18, 98, 22, 103, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        // Anger lines intensify
        displayManager.getDisplay().drawLine(13, 24, 18, 28, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(15, 28, 20, 32, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(110, 24, 115, 28, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(108, 28, 113, 32, SSD1306_WHITE);
        break;
        
      // === VIOLENT SHAKING (frames 11-38) ===
      case 11: case 13: case 15: case 17: case 19:
      case 21: case 23: case 25: case 27: case 29:
      case 31: case 33: case 35: case 37:
        // Shake LEFT
        {
          int xLeft = -3;
          displayManager.drawEyes(40 + xLeft, 30, 88 + xLeft, 30, 10);
          displayManager.getDisplay().fillTriangle(25 + xLeft, 18, 30 + xLeft, 22, 50 + xLeft, 18, SSD1306_WHITE);
          displayManager.getDisplay().fillTriangle(78 + xLeft, 18, 98 + xLeft, 22, 103 + xLeft, 18, SSD1306_WHITE);
          displayManager.getDisplay().fillRoundRect(52 + xLeft, 50, 24, 6, 3, SSD1306_WHITE);
          // Anger lines
          displayManager.getDisplay().drawLine(10, 24, 15, 28, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(12, 28, 17, 32, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(113, 24, 118, 28, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(111, 28, 116, 32, SSD1306_WHITE);
          // Steam starting to rise
          if (angryFrame > 20) {
            displayManager.getDisplay().drawLine(60, 10, 62, 5, SSD1306_WHITE);
            displayManager.getDisplay().drawLine(68, 10, 66, 5, SSD1306_WHITE);
          }
        }
        break;
        
      case 12: case 14: case 16: case 18: case 20:
      case 22: case 24: case 26: case 28: case 30:
      case 32: case 34: case 36: case 38:
        // Shake RIGHT
        {
          int xRight = 3;
          displayManager.drawEyes(40 + xRight, 30, 88 + xRight, 30, 10);
          displayManager.getDisplay().fillTriangle(25 + xRight, 18, 30 + xRight, 22, 50 + xRight, 18, SSD1306_WHITE);
          displayManager.getDisplay().fillTriangle(78 + xRight, 18, 98 + xRight, 22, 103 + xRight, 18, SSD1306_WHITE);
          displayManager.getDisplay().fillRoundRect(52 + xRight, 50, 24, 6, 3, SSD1306_WHITE);
          // Anger lines opposite side
          displayManager.getDisplay().drawLine(16, 24, 21, 28, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(18, 28, 23, 32, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(107, 24, 112, 28, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(105, 28, 110, 32, SSD1306_WHITE);
          // Steam waves
          if (angryFrame > 20) {
            displayManager.getDisplay().drawLine(58, 8, 60, 3, SSD1306_WHITE);
            displayManager.getDisplay().drawLine(70, 8, 68, 3, SSD1306_WHITE);
            displayManager.getDisplay().drawLine(64, 6, 64, 1, SSD1306_WHITE);
          }
        }
        break;
        
      // === MAXIMUM RAGE (frames 39-45) ===
      case 39: case 40: case 41: case 42: case 43:
      case 44: case 45:
        // Extreme shaking
        {
          int xExtreme = (angryFrame % 2 == 0) ? -4 : 4;
          displayManager.drawEyes(40 + xExtreme, 30, 88 + xExtreme, 30, 9);
          displayManager.getDisplay().fillTriangle(25 + xExtreme, 17, 30 + xExtreme, 21, 50 + xExtreme, 17, SSD1306_WHITE);
          displayManager.getDisplay().fillTriangle(78 + xExtreme, 17, 98 + xExtreme, 21, 103 + xExtreme, 17, SSD1306_WHITE);
          displayManager.getDisplay().fillRoundRect(52 + xExtreme, 51, 24, 7, 3, SSD1306_WHITE);
          // Heavy anger veins
          displayManager.getDisplay().fillCircle(15, 28, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(113, 28, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(12, 24, 17, 29, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(111, 24, 116, 29, SSD1306_WHITE);
          // Heavy steam
          displayManager.getDisplay().drawLine(56, 10, 58, 4, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(60, 8, 62, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(64, 10, 64, 3, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(68, 8, 66, 2, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(72, 10, 70, 4, SSD1306_WHITE);
        }
        break;
        
      // === CALMING SLIGHTLY (frames 46-50) ===
      case 46: case 47:
        displayManager.drawEyes(40, 30, 88, 30, 10);
        displayManager.getDisplay().fillTriangle(25, 18, 30, 22, 50, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(78, 18, 98, 22, 103, 18, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(14, 26, 19, 30, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(109, 26, 114, 30, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(62, 10, 64, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(66, 10, 64, 5, SSD1306_WHITE);
        break;
        
      case 48: case 49: case 50:
        // Still angry but less intense
        displayManager.drawEyes(40, 30, 88, 30, 11);
        displayManager.getDisplay().fillTriangle(26, 19, 31, 23, 50, 19, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(78, 19, 97, 23, 102, 19, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(54, 50, 20, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(15, 27, 20, 31, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(108, 27, 113, 31, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    angryFrame = (angryFrame + 1) % 51;
    lastAngryAnim = currentTime;
  }
}

// Animated sad state - drooping features with tears falling
void AnimationManager::animateSad() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastSadAnim > 30) {
    displayManager.clearDisplay();
    
    switch(sadFrame) {
      // === SADNESS BUILDING (frames 0-8) ===
      case 0:
        // Normal eyes, neutral mouth
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawLine(52, 50, 76, 50, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes starting to droop
        displayManager.drawEyes(40, 29, 88, 29, 17);
        displayManager.getDisplay().drawLine(52, 51, 76, 51, SSD1306_WHITE);
        break;
        
      case 3: case 4:
        // Eyes drooping more, mouth turning down
        displayManager.drawEyes(40, 30, 88, 30, 16);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        break;
        
      case 5: case 6:
        // Eyes very droopy, frown forming
        displayManager.drawEyes(40, 31, 88, 31, 15);
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 6, 3, SSD1306_WHITE);
        // Eyebrows drooping
        displayManager.getDisplay().drawLine(30, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 98, 20, SSD1306_WHITE);
        break;
        
      case 7: case 8:
        // Maximum droop, tear forming
        displayManager.drawEyes(40, 32, 88, 32, 14);
        displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        // First tear appears
        displayManager.getDisplay().fillCircle(50, 40, 2, SSD1306_WHITE);
        break;
        
      // === CRYING (frames 9-40) ===
      case 9: case 10: case 11: case 12:
        // Tears starting to fall
        displayManager.drawEyes(40, 32, 88, 32, 14);
        displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        // Left tear falling
        displayManager.getDisplay().fillCircle(50, 42, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(50, 46, 1, SSD1306_WHITE);
        break;
        
      case 13: case 14: case 15: case 16:
        // Tears continuing
        displayManager.drawEyes(40, 32, 88, 32, 13);
        displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        // Left tear further down
        displayManager.getDisplay().fillCircle(50, 48, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(50, 52, 1, SSD1306_WHITE);
        // Right tear starts
        displayManager.getDisplay().fillCircle(78, 40, 2, SSD1306_WHITE);
        break;
        
      case 17: case 18: case 19: case 20:
        // Both tears falling
        displayManager.drawEyes(40, 32, 88, 32, 13);
        displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        // Left tear at bottom
        displayManager.getDisplay().fillCircle(50, 56, 1, SSD1306_WHITE);
        // Right tear mid-fall
        displayManager.getDisplay().fillCircle(78, 44, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(78, 48, 1, SSD1306_WHITE);
        break;
        
      case 21: case 22: case 23: case 24: case 25:
      case 26: case 27: case 28: case 29: case 30:
        // Heavy crying - tears streaming
        displayManager.drawEyes(40, 32, 88, 32, 12);
        displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        // Multiple tear drops
        if (sadFrame % 4 < 2) {
          displayManager.getDisplay().fillCircle(50, 42, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(50, 50, 1, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(78, 46, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(78, 54, 1, SSD1306_WHITE);
        } else {
          displayManager.getDisplay().fillCircle(50, 38, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(50, 46, 1, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(50, 54, 1, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(78, 40, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(78, 48, 1, SSD1306_WHITE);
        }
        break;
        
      case 31: case 32: case 33: case 34: case 35:
      case 36: case 37: case 38: case 39: case 40:
        // Continuous crying
        {
          displayManager.drawEyes(40, 32, 88, 32, 12);
          displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
          // Tear trails
          displayManager.getDisplay().drawLine(50, 38, 50, 58, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 38, 78, 58, SSD1306_WHITE);
          // Animated tears
          int tearPos = (sadFrame % 6) * 4;
          displayManager.getDisplay().fillCircle(50, 38 + tearPos, 2, SSD1306_WHITE);
          displayManager.getDisplay().fillCircle(78, 40 + tearPos, 2, SSD1306_WHITE);
        }
        break;
        
      // === SLIGHT RECOVERY (frames 41-50) ===
      case 41: case 42:
        displayManager.drawEyes(40, 32, 88, 32, 13);
        displayManager.getDisplay().fillRoundRect(50, 52, 28, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        // Fewer tears
        displayManager.getDisplay().fillCircle(50, 48, 1, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(78, 50, 1, SSD1306_WHITE);
        break;
        
      case 43: case 44:
        displayManager.drawEyes(40, 31, 88, 31, 14);
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 100, 20, SSD1306_WHITE);
        break;
        
      case 45: case 46: case 47: case 48: case 49: case 50:
        // Still sad but calmer
        displayManager.drawEyes(40, 30, 88, 30, 16);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(30, 20, 50, 22, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 22, 98, 20, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    sadFrame = (sadFrame + 1) % 51;
    lastSadAnim = currentTime;
  }
}

// Animated surprised state - sudden shock with expanding features
void AnimationManager::animateSurprised() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastSurprisedAnim > 30) {
    displayManager.clearDisplay();
    
    switch(surprisedFrame) {
      // === SUDDEN REALIZATION (frames 0-5) ===
      case 0:
        // Normal state
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawCircle(64, 50, 4, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes start widening
        displayManager.drawEyes(40, 28, 88, 28, 20);
        displayManager.getDisplay().drawCircle(64, 50, 5, SSD1306_WHITE);
        break;
        
      case 2:
        // Eyes wider
        displayManager.drawEyes(40, 27, 88, 27, 22);
        displayManager.getDisplay().fillCircle(64, 50, 5, SSD1306_WHITE);
        // Eyebrows raising
        displayManager.getDisplay().drawLine(30, 18, 50, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 16, 98, 18, SSD1306_WHITE);
        break;
        
      case 3:
        // Eyes much wider
        displayManager.drawEyes(40, 26, 88, 26, 24);
        displayManager.getDisplay().fillCircle(64, 50, 6, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 16, 50, 14, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 14, 100, 16, SSD1306_WHITE);
        break;
        
      case 4:
        // Eyes at max, mouth opening
        displayManager.drawEyes(40, 25, 88, 25, 26);
        displayManager.getDisplay().fillCircle(64, 51, 7, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(26, 14, 50, 12, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 12, 102, 14, SSD1306_WHITE);
        // First shock wave
        displayManager.getDisplay().drawCircle(64, 32, 40, SSD1306_WHITE);
        break;
        
      case 5:
        // Maximum surprise
        displayManager.drawEyes(40, 24, 88, 24, 28);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(25, 12, 50, 10, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 10, 103, 12, SSD1306_WHITE);
        // Shock wave expanding
        displayManager.getDisplay().drawCircle(64, 32, 45, SSD1306_WHITE);
        break;
        
      // === MAINTAINING SHOCK (frames 6-35) ===
      case 6: case 8: case 10: case 12: case 14:
      case 16: case 18: case 20: case 22: case 24:
      case 26: case 28: case 30: case 32: case 34:
        // Eyes at maximum width
        displayManager.drawEyes(40, 24, 88, 24, 28);
        displayManager.getDisplay().fillCircle(40, 24, 4, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 24, 4, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(25, 12, 50, 10, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 10, 103, 12, SSD1306_WHITE);
        // Pulsing shock waves
        displayManager.getDisplay().drawCircle(64, 32, 50, SSD1306_WHITE);
        break;
        
      case 7: case 9: case 11: case 13: case 15:
      case 17: case 19: case 21: case 23: case 25:
      case 27: case 29: case 31: case 33: case 35:
        // Slight variation
        displayManager.drawEyes(40, 24, 88, 24, 28);
        displayManager.getDisplay().fillCircle(40, 24, 4, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 24, 4, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(25, 12, 50, 10, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 10, 103, 12, SSD1306_WHITE);
        // Different wave positions
        displayManager.getDisplay().drawCircle(64, 32, 55, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(64, 32, 45, SSD1306_WHITE);
        break;
        
      // === CALMING DOWN (frames 36-50) ===
      case 36: case 37:
        displayManager.drawEyes(40, 25, 88, 25, 26);
        displayManager.getDisplay().fillCircle(64, 51, 7, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(26, 14, 50, 12, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 12, 102, 14, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(64, 32, 50, SSD1306_WHITE);
        break;
        
      case 38: case 39:
        displayManager.drawEyes(40, 26, 88, 26, 24);
        displayManager.getDisplay().fillCircle(64, 50, 6, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(28, 16, 50, 14, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 14, 100, 16, SSD1306_WHITE);
        break;
        
      case 40: case 41:
        displayManager.drawEyes(40, 27, 88, 27, 22);
        displayManager.getDisplay().fillCircle(64, 50, 5, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(30, 18, 50, 16, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(78, 16, 98, 18, SSD1306_WHITE);
        break;
        
      case 42: case 43: case 44: case 45: case 46:
      case 47: case 48: case 49: case 50:
        // Return to mild surprise
        displayManager.drawEyes(40, 28, 88, 28, 20);
        displayManager.getDisplay().drawCircle(64, 50, 5, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    surprisedFrame = (surprisedFrame + 1) % 51;
    lastSurprisedAnim = currentTime;
  }
}
