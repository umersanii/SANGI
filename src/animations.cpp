#include "animations.h"
#include "display.h"
#include "network.h"

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
    lastMusicAnim(0),
    lastDeadAnim(0),
    lastNotificationAnim(0),
    lastCodingAnim(0),
    lastGitHubStatsAnim(0),
    sleepyFrame(0),
    thinkFrame(0),
    exciteFrame(0),
    confuseFrame(0),
    happyFrame(0),
    loveFrame(0),
    angryFrame(0),
    sadFrame(0),
    surprisedFrame(0),
    musicFrame(0),
    deadFrame(0),
    notificationFrame(0),
    codingFrame(0),
    githubStatsFrame(0) {
}

// Reset animation frame to start smoothly from beginning
void AnimationManager::resetAnimation(EmotionState emotion) {
  switch(emotion) {
    case EMOTION_SLEEPY:
      sleepyFrame = 0;
      lastSleepyAnim = 0;
      break;
    case EMOTION_THINKING:
      thinkFrame = 0;
      lastThinkAnim = 0;
      break;
    case EMOTION_EXCITED:
      exciteFrame = 0;
      lastExciteAnim = 0;
      break;
    case EMOTION_CONFUSED:
      confuseFrame = 0;
      lastConfuseAnim = 0;
      break;
    case EMOTION_HAPPY:
      happyFrame = 0;
      lastHappyAnim = 0;
      break;
    case EMOTION_LOVE:
      loveFrame = 0;
      lastLoveAnim = 0;
      break;
    case EMOTION_ANGRY:
      angryFrame = 0;
      lastAngryAnim = 0;
      break;
    case EMOTION_SAD:
      sadFrame = 0;
      lastSadAnim = 0;
      break;
    case EMOTION_SURPRISED:
      surprisedFrame = 0;
      lastSurprisedAnim = 0;
      break;
    case EMOTION_MUSIC:
      musicFrame = 0;
      lastMusicAnim = 0;
      break;
    case EMOTION_DEAD:
      deadFrame = 0;
      lastDeadAnim = 0;
      break;
    case EMOTION_NOTIFICATION:
      notificationFrame = 0;
      lastNotificationAnim = 0;
      break;
    case EMOTION_CODING:
      codingFrame = 0;
      lastCodingAnim = 0;
      break;
    default:
      break;
  }
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
        // Eyes fully closed - mouth round, first zzz appear near face
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(85, 35);
        displayManager.getDisplay().print("z");
        break;
        
      // === STAY CLOSED (frames 9-42) - slow floating zzz ===
      case 9: case 10: case 11: case 12: case 13:
        // Eyes stay closed, zzz floating from face toward top right
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(88, 32);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(95, 30);
        displayManager.getDisplay().print("z");
        break;
        
      case 14: case 15: case 16: case 17: case 18:
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(92, 28);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(100, 25);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(108, 22);
        displayManager.getDisplay().print("z");
        break;
        
      case 19: case 20: case 21: case 22: case 23:
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(95, 24);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(104, 20);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(112, 16);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(118, 12);
        displayManager.getDisplay().print("z");
        break;
        
      case 24: case 25: case 26: case 27: case 28:
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(98, 20);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(106, 16);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(114, 12);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(120, 8);
        displayManager.getDisplay().print("z");
        break;
        
      case 29: case 30: case 31: case 32: case 33:
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(100, 18);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(108, 14);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(115, 10);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(122, 6);
        displayManager.getDisplay().print("z");
        break;
        
      case 34: case 35: case 36: case 37: case 38:
      case 39: case 40: case 41: case 42:
        // zzz continue floating to top right corner
        displayManager.drawEyes(40, 31, 88, 31, 2);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(102, 16);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(110, 12);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(116, 8);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(122, 4);
        displayManager.getDisplay().print("z");
        displayManager.getDisplay().setCursor(126, 2);
        displayManager.getDisplay().print("z");
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
        // Normal eyes, looking forward
        displayManager.getDisplay().fillRoundRect(33, 22, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 22, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        // Small exclamation mark starting to appear (drawn with primitives)
        displayManager.getDisplay().fillRect(118, 12, 2, 10, SSD1306_WHITE); // stem
        displayManager.getDisplay().fillRect(118, 24, 2, 2, SSD1306_WHITE);  // dot
        break;
        
      case 1: case 2:
        // Eyes start looking up-left
        displayManager.getDisplay().fillRoundRect(31, 20, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(79, 20, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(50, 50, 24, 6, 3, SSD1306_WHITE);  // Mouth moves left
        // Exclamation mark growing (primitives)
        displayManager.getDisplay().fillRect(115, 10, 3, 14, SSD1306_WHITE); // stem
        displayManager.getDisplay().fillRect(115, 26, 3, 3, SSD1306_WHITE);  // dot
        break;
        
      case 3: case 4: case 5:
        // Eyes fully looking up-left
        displayManager.getDisplay().fillRoundRect(30, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);  // Static mouth
        // Big exclamation mark appears (ensure within bounds 128x64)
        displayManager.getDisplay().fillRect(110, 6, 4, 20, SSD1306_WHITE); // stem
        displayManager.getDisplay().fillRect(110, 28, 4, 4, SSD1306_WHITE); // dot
        break;
        
      case 6: case 7: case 8:
        // Eyes looking up-left (concentrated)
        displayManager.getDisplay().fillRoundRect(30, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);  // Static mouth
        // Big exclamation mark (primitives)
        displayManager.getDisplay().fillRect(110, 6, 4, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(110, 28, 4, 4, SSD1306_WHITE);
        break;
        
      // === DEEP THINKING (frames 9-35) ===
      case 9: case 10: case 11: case 12: case 13:
      case 14: case 15: case 16: case 17: case 18:
        // Maintain thinking pose - eyes looking up-left
        displayManager.getDisplay().fillRoundRect(30, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);  // Static mouth
        // Big exclamation mark
        displayManager.getDisplay().fillRect(110, 6, 4, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(110, 28, 4, 4, SSD1306_WHITE);
        break;
        
      case 19: case 20: case 21: case 22: case 23:
      case 24: case 25: case 26: case 27: case 28:
        // Eyes shift slightly more left (contemplating)
        displayManager.getDisplay().fillRoundRect(28, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(76, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);  // Static mouth
        // Big exclamation mark
        displayManager.getDisplay().fillRect(110, 6, 4, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(110, 28, 4, 4, SSD1306_WHITE);
        break;
        
      case 29: case 30: case 31: case 32: case 33:
      case 34: case 35:
        // Eyes looking most left
        displayManager.getDisplay().fillRoundRect(26, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(74, 18, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);  // Static mouth
        // Big exclamation mark
        displayManager.getDisplay().fillRect(110, 6, 4, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(110, 28, 4, 4, SSD1306_WHITE);
        break;
        
      // === REALIZATION MOMENT (frames 36-42) ===
      case 36: case 37: case 38:
        // Eyes widen slightly (idea forming)
        displayManager.getDisplay().fillRoundRect(30, 19, 18, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 19, 18, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(49, 50, 24, 6, 3, SSD1306_WHITE);  // Mouth normal & left
        // Larger exclamation mark (slightly shifted right)
        displayManager.getDisplay().fillRect(112, 8, 4, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(112, 30, 4, 4, SSD1306_WHITE);
        break;
        
      case 39: case 40: case 41: case 42:
        // Eyes fully open (eureka moment!)
        displayManager.getDisplay().fillRoundRect(31, 20, 18, 24, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(79, 20, 18, 24, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(50, 50, 24, 5, 2, SSD1306_WHITE);  // Mouth smaller & left
        // Big exclamation mark
        displayManager.getDisplay().fillRect(112, 6, 4, 20, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(112, 28, 4, 4, SSD1306_WHITE);
        break;
        
      // === RETURN TO NEUTRAL (frames 43-50) ===
      case 43:
        displayManager.getDisplay().fillRoundRect(32, 21, 18, 21, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(80, 21, 18, 21, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(51, 50, 24, 6, 3, SSD1306_WHITE);  // Mouth normal & moving back
        // Exclamation mark fading (smaller)
        displayManager.getDisplay().fillRect(115, 12, 3, 14, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(115, 26, 3, 3, SSD1306_WHITE);
        break;
        
      case 44:
        displayManager.getDisplay().fillRoundRect(33, 21, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 21, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 5, 2, SSD1306_WHITE);  // Mouth smaller & centered
        // Smaller exclamation mark
        displayManager.getDisplay().fillRect(115, 12, 3, 14, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(115, 26, 3, 3, SSD1306_WHITE);
        break;
        
      case 45: case 46: case 47: case 48: case 49: case 50:
        // Back to neutral thinking pose
        displayManager.getDisplay().fillRoundRect(33, 22, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 22, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);  // Mouth normal & centered
        // Small exclamation mark remains
        displayManager.getDisplay().fillRect(118, 14, 2, 10, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(118, 26, 2, 2, SSD1306_WHITE);
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
        // Simple filled mouth (24px wide, matches other emotions)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        break;
        
      case 5: case 6: case 7: case 8:
        // Maximum unevenness
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        // First question mark appears low
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(108, 30);
        displayManager.getDisplay().print("?");
        break;
        
      // === DEEP CONFUSION WITH TILTING (frames 9-25) ===
      case 9: case 10: case 11: case 12: case 13:
      case 14: case 15: case 16:
        // Eyes reversed (other eye bigger)
        displayManager.getDisplay().fillRoundRect(30, 26, 20, 12, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 24, 20, 20, 5, SSD1306_WHITE);
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        // Question mark floating up
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(108, 26);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setCursor(115, 30);
        displayManager.getDisplay().print("?");
        break;
        
      case 17: case 18: case 19: case 20: case 21:
      case 22: case 23: case 24: case 25:
        // Back to first configuration
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        // Multiple question marks floating
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(106, 22);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setCursor(113, 26);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setTextSize(2);
        displayManager.getDisplay().setCursor(118, 18);
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
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        // Question marks floating at highest position
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(105, 18);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setCursor(112, 22);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setTextSize(2);
        displayManager.getDisplay().setCursor(118, 14);
        displayManager.getDisplay().print("?");
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(123, 10);
        displayManager.getDisplay().print("?");
        break;
        
      // === SETTLING (frames 39-50) ===
      case 39: case 40:
        displayManager.getDisplay().fillRoundRect(30, 24, 20, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 12, 5, SSD1306_WHITE);
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setCursor(110, 20);
        displayManager.getDisplay().print("?");
        break;
        
      case 41: case 42:
        displayManager.getDisplay().fillRoundRect(32, 25, 20, 18, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(78, 26, 20, 16, 5, SSD1306_WHITE);
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
        break;
        
      case 43: case 44: case 45: case 46: case 47:
      case 48: case 49: case 50:
        // Return to mildly confused state
        displayManager.drawEyes(40, 28, 88, 28, 18);
        // Simple filled mouth (24px wide)
        displayManager.getDisplay().fillRoundRect(52, 48, 24, 5, 2, SSD1306_WHITE);
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
        displayManager.getDisplay().fillCircle(34, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(82, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(50, 50, 28, 7, 3, SSD1306_WHITE);
        break;
        
      case 3: case 4:
        // Hearts growing with connection
        displayManager.getDisplay().fillCircle(34, 27, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 27, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(28, 27, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 32, 39, 38, 50, 32, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 27, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 27, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(76, 27, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 32, 87, 38, 98, 32, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        break;
        
      case 5: case 6: case 7:
        // Full heart eyes formed
        displayManager.getDisplay().fillCircle(34, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(27, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(27, 32, 39, 40, 51, 32, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(75, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(75, 32, 87, 40, 99, 32, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        // Blush appears
        displayManager.getDisplay().fillCircle(15, 42, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 42, 3, SSD1306_WHITE);
        break;
        
      case 8: case 9: case 10:
        // Hearts at max with first floating heart
        displayManager.getDisplay().fillCircle(34, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(27, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(27, 32, 39, 40, 51, 32, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(75, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(75, 32, 87, 40, 99, 32, SSD1306_WHITE);
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
        displayManager.getDisplay().fillCircle(34, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(26, 26, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(26, 33, 39, 42, 52, 33, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(74, 26, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(74, 33, 87, 42, 100, 33, SSD1306_WHITE);
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
        displayManager.getDisplay().fillCircle(34, 26, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(28, 26, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(28, 31, 39, 38, 50, 31, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(76, 26, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(76, 31, 87, 38, 98, 31, SSD1306_WHITE);
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
        displayManager.getDisplay().fillCircle(34, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(27, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(27, 32, 39, 40, 51, 32, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(75, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(75, 32, 87, 40, 99, 32, SSD1306_WHITE);
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
        displayManager.getDisplay().fillCircle(34, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(27, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(27, 32, 39, 40, 51, 32, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(75, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(75, 32, 87, 40, 99, 32, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(48, 50, 32, 8, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(15, 42, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(113, 42, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(110, 15, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(114, 15, 2, SSD1306_WHITE);
        break;
        
      case 47: case 48: case 49: case 50:
        // Back to calm love state
        displayManager.getDisplay().fillCircle(34, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(44, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(27, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(27, 32, 39, 40, 51, 32, SSD1306_WHITE);
        
        displayManager.getDisplay().fillCircle(82, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(92, 26, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(75, 26, 24, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillTriangle(75, 32, 87, 40, 99, 32, SSD1306_WHITE);
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
        displayManager.drawEyes(40, 30, 88, 30, 14);
        // Simple frown mouth
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 4, 2, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes narrowing, thick angled eyebrows appear (furrowed)
        displayManager.drawEyes(40, 31, 88, 31, 12);
        // Left eyebrow - thick angled line pointing down toward center
        for(int i = 0; i < 4; i++) {
          displayManager.getDisplay().drawLine(24, 17 + i, 52, 21 + i, SSD1306_WHITE);
        }
        // Right eyebrow - thick angled line pointing down toward center
        for(int i = 0; i < 4; i++) {
          displayManager.getDisplay().drawLine(76, 21 + i, 104, 17 + i, SSD1306_WHITE);
        }
        // Deeper frown
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 5, 2, SSD1306_WHITE);
        break;
        
      case 3: case 4: case 5:
        // Eyebrows lowering more, eyes more narrow
        displayManager.drawEyes(40, 32, 88, 32, 11);
        // Thicker angled eyebrows (angrier)
        for(int i = 0; i < 5; i++) {
          displayManager.getDisplay().drawLine(22, 16 + i, 52, 22 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 5; i++) {
          displayManager.getDisplay().drawLine(76, 22 + i, 106, 16 + i, SSD1306_WHITE);
        }
        // Small frown
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 5, 2, SSD1306_WHITE);
        break;
        
      case 6: case 7: case 8: case 9: case 10:
        // Full angry expression with maximum eyebrow angle
        displayManager.drawEyes(40, 32, 88, 32, 10);
        // Maximum thick furrowed eyebrows
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(20, 15 + i, 52, 23 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(76, 23 + i, 108, 15 + i, SSD1306_WHITE);
        }
        // Small angry frown
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 5, 2, SSD1306_WHITE);
        break;
        
      // === VIOLENT SHAKING (frames 11-38) ===
      case 11: case 13: case 15: case 17: case 19:
      case 21: case 23: case 25: case 27: case 29:
      case 31: case 33: case 35: case 37:
        // Shake LEFT
        {
          int xLeft = -3;
          displayManager.drawEyes(40 + xLeft, 32, 88 + xLeft, 32, 10);
          // Thick angled furrowed eyebrows
          for(int i = 0; i < 6; i++) {
            displayManager.getDisplay().drawLine(20 + xLeft, 15 + i, 52 + xLeft, 23 + i, SSD1306_WHITE);
          }
          for(int i = 0; i < 6; i++) {
            displayManager.getDisplay().drawLine(76 + xLeft, 23 + i, 108 + xLeft, 15 + i, SSD1306_WHITE);
          }
          // Angry frown
          displayManager.getDisplay().fillRoundRect(52 + xLeft, 52, 24, 5, 2, SSD1306_WHITE);
        }
        break;
        
      case 12: case 14: case 16: case 18: case 20:
      case 22: case 24: case 26: case 28: case 30:
      case 32: case 34: case 36: case 38:
        // Shake RIGHT
        {
          int xRight = 3;
          displayManager.drawEyes(40 + xRight, 32, 88 + xRight, 32, 10);
          // Thick angled furrowed eyebrows
          for(int i = 0; i < 6; i++) {
            displayManager.getDisplay().drawLine(20 + xRight, 15 + i, 52 + xRight, 23 + i, SSD1306_WHITE);
          }
          for(int i = 0; i < 6; i++) {
            displayManager.getDisplay().drawLine(76 + xRight, 23 + i, 108 + xRight, 15 + i, SSD1306_WHITE);
          }
          // Angry frown
          displayManager.getDisplay().fillRoundRect(52 + xRight, 52, 24, 5, 2, SSD1306_WHITE);
        }
        break;
        
      // === MAXIMUM RAGE (frames 39-45) ===
      case 39: case 40: case 41: case 42: case 43:
      case 44: case 45:
        // Extreme shaking
        {
          int xExtreme = (angryFrame % 2 == 0) ? -4 : 4;
          displayManager.drawEyes(40 + xExtreme, 32, 88 + xExtreme, 32, 9);
          // Maximum thick furrowed eyebrows
          for(int i = 0; i < 7; i++) {
            displayManager.getDisplay().drawLine(18 + xExtreme, 14 + i, 52 + xExtreme, 24 + i, SSD1306_WHITE);
          }
          for(int i = 0; i < 7; i++) {
            displayManager.getDisplay().drawLine(76 + xExtreme, 24 + i, 110 + xExtreme, 14 + i, SSD1306_WHITE);
          }
          // Small intense frown
          displayManager.getDisplay().fillRoundRect(52 + xExtreme, 52, 24, 5, 2, SSD1306_WHITE);
        }
        break;
        
      // === CALMING SLIGHTLY (frames 46-50) ===
      case 46: case 47:
        displayManager.drawEyes(40, 32, 88, 32, 10);
        // Thick furrowed eyebrows
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(20, 15 + i, 52, 23 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(76, 23 + i, 108, 15 + i, SSD1306_WHITE);
        }
        // Frown
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 5, 2, SSD1306_WHITE);
        break;
        
      case 48: case 49: case 50:
        // Still angry but less intense
        displayManager.drawEyes(40, 32, 88, 32, 11);
        // Slightly less thick eyebrows
        for(int i = 0; i < 5; i++) {
          displayManager.getDisplay().drawLine(22, 16 + i, 52, 22 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 5; i++) {
          displayManager.getDisplay().drawLine(76, 22 + i, 106, 16 + i, SSD1306_WHITE);
        }
        // Small frown
        displayManager.getDisplay().fillRoundRect(52, 52, 24, 5, 2, SSD1306_WHITE);
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
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 4, 2, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes starting to droop
        displayManager.drawEyes(40, 29, 88, 29, 17);
        displayManager.getDisplay().fillRoundRect(56, 51, 16, 4, 2, SSD1306_WHITE);
        break;
        
      case 3: case 4:
        // Eyes drooping more, mouth turning down
        displayManager.drawEyes(40, 30, 88, 30, 16);
        displayManager.getDisplay().fillRoundRect(56, 52, 16, 4, 2, SSD1306_WHITE);
        break;
        
      case 5: case 6:
        // Eyes very droopy, frown forming
        displayManager.drawEyes(40, 31, 88, 31, 15);
        displayManager.getDisplay().fillRoundRect(56, 52, 16, 5, 2, SSD1306_WHITE);
        break;
        
      case 7: case 8:
        // Maximum droop, tear forming
        displayManager.drawEyes(40, 32, 88, 32, 14);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // First tear appears
        displayManager.getDisplay().fillCircle(50, 40, 2, SSD1306_WHITE);
        break;
        
      // === CRYING (frames 9-40) ===
      case 9: case 10: case 11: case 12:
        // Tears starting to form - just dot
        displayManager.drawEyes(40, 32, 88, 32, 14);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // Single teardrop starting to form
        displayManager.getDisplay().fillCircle(50, 40, 1, SSD1306_WHITE);
        break;
        
      case 13: case 14: case 15: case 16:
        // Tear growing slightly
        displayManager.drawEyes(40, 32, 88, 32, 13);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // Tear growing
        displayManager.getDisplay().fillCircle(50, 41, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 43, 50, 45, SSD1306_WHITE);
        break;
        
      case 17: case 18: case 19: case 20:
        // Tear falling slowly
        displayManager.drawEyes(40, 32, 88, 32, 13);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // Single tear falling
        displayManager.getDisplay().fillCircle(50, 42, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 44, 50, 50, SSD1306_WHITE);
        break;
        
      case 21: case 22: case 23: case 24: case 25:
      case 26: case 27: case 28: case 29: case 30:
        // Tear continuing to fall
        displayManager.drawEyes(40, 32, 88, 32, 12);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // Single tear falling further
        displayManager.getDisplay().fillCircle(50, 42, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 44, 50, 55, SSD1306_WHITE);
        break;
        
      case 31: case 32: case 33: case 34: case 35:
      case 36: case 37: case 38: case 39: case 40:
        // Tear reaches bottom
        displayManager.drawEyes(40, 32, 88, 32, 12);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // Single tear at full length
        displayManager.getDisplay().fillCircle(50, 42, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 44, 50, 60, SSD1306_WHITE);
        break;
        
      // === SLIGHT RECOVERY (frames 41-50) ===
      case 41: case 42:
        displayManager.drawEyes(40, 32, 88, 32, 13);
        displayManager.getDisplay().fillRoundRect(54, 52, 20, 5, 2, SSD1306_WHITE);
        // Tear fading
        displayManager.getDisplay().fillCircle(50, 42, 1, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 43, 50, 58, SSD1306_WHITE);
        break;
        
      case 43: case 44:
        displayManager.drawEyes(40, 31, 88, 31, 14);
        displayManager.getDisplay().fillRoundRect(56, 52, 16, 5, 2, SSD1306_WHITE);
        // Last small tear
        displayManager.getDisplay().fillCircle(50, 42, 1, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(50, 43, 50, 52, SSD1306_WHITE);
        break;
        
      case 45: case 46: case 47: case 48: case 49: case 50:
        // Still sad but calmer - no tears
        displayManager.drawEyes(40, 30, 88, 30, 16);
        displayManager.getDisplay().fillRoundRect(56, 52, 16, 4, 2, SSD1306_WHITE);
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
        // Normal state (eyes: y=24, height=18, center y = 24+9 = 33)
        displayManager.getDisplay().fillRoundRect(33, 24, 18, 18, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 24, 18, 18, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 33, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 33, 3, SSD1306_BLACK);
        displayManager.getDisplay().drawCircle(64, 50, 4, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes start widening (eyes: y=23, height=20, center y = 23+10 = 33)
        displayManager.getDisplay().fillRoundRect(33, 23, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 23, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 33, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 33, 3, SSD1306_BLACK);
        displayManager.getDisplay().drawCircle(64, 50, 5, SSD1306_WHITE);
        break;
        
      case 2:
        // Eyes wider (eyes: y=21, height=22, center y = 21+11 = 32)
        displayManager.getDisplay().fillRoundRect(33, 21, 18, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 21, 18, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 50, 5, SSD1306_WHITE);
        break;
        
      case 3:
        // Eyes much wider (eyes: y=20, height=24, center y = 20+12 = 32)
        displayManager.getDisplay().fillRoundRect(33, 20, 18, 24, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 20, 18, 24, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 50, 6, SSD1306_WHITE);
        break;
        
      case 4:
        // Eyes at max, mouth opening (eyes: y=19, height=26, center y = 19+13 = 32)
        displayManager.getDisplay().fillRoundRect(33, 19, 18, 26, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 19, 18, 26, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 51, 7, SSD1306_WHITE);
        break;
        
      case 5:
        // Maximum surprise (eyes: y=18, height=28, center y = 18+14 = 32)
        displayManager.getDisplay().fillRoundRect(33, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      // === MAINTAINING SHOCK (frames 6-35) ===
      case 6: case 7: case 8: case 9: case 10:
        // Eyes at maximum width with centered pupils (eyes: y=18, height=28, center y = 32)
        displayManager.getDisplay().fillRoundRect(33, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      // FIRST BLINK
      case 11:
        // Eyes closing for first blink
        displayManager.getDisplay().fillRoundRect(33, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      case 12:
        // Eyes fully closed
        displayManager.getDisplay().fillRoundRect(33, 31, 18, 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 31, 18, 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      case 13:
        // Eyes opening from first blink
        displayManager.getDisplay().fillRoundRect(33, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      case 14: case 15: case 16:
        // Eyes fully open again
        displayManager.getDisplay().fillRoundRect(33, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      // SECOND BLINK (quick)
      case 17:
        // Eyes closing for second blink
        displayManager.getDisplay().fillRoundRect(33, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      case 18:
        // Eyes fully closed
        displayManager.getDisplay().fillRoundRect(33, 31, 18, 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 31, 18, 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      case 19:
        // Eyes opening from second blink
        displayManager.getDisplay().fillRoundRect(33, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 28, 18, 10, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      case 20: case 21: case 22: case 23: case 24:
      case 25: case 26: case 27: case 28: case 29:
      case 30: case 31: case 32: case 33: case 34: case 35:
        // Eyes fully open, maintaining shock
        displayManager.getDisplay().fillRoundRect(33, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 18, 18, 28, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 52, 8, SSD1306_WHITE);
        break;
        
      // === CALMING DOWN (frames 36-50) ===
      case 36: case 37:
        // Eyes: y=19, height=26, center y = 32
        displayManager.getDisplay().fillRoundRect(33, 19, 18, 26, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 19, 18, 26, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 51, 7, SSD1306_WHITE);
        break;
        
      case 38: case 39:
        // Eyes: y=20, height=24, center y = 32
        displayManager.getDisplay().fillRoundRect(33, 20, 18, 24, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 20, 18, 24, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 50, 6, SSD1306_WHITE);
        break;
        
      case 40: case 41:
        // Eyes: y=21, height=22, center y = 32
        displayManager.getDisplay().fillRoundRect(33, 21, 18, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 21, 18, 22, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 32, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 50, 5, SSD1306_WHITE);
        break;
        
      case 42: case 43: case 44: case 45: case 46:
      case 47: case 48: case 49: case 50:
        // Return to mild surprise (eyes: y=23, height=20, center y = 33)
        displayManager.getDisplay().fillRoundRect(33, 23, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(81, 23, 18, 20, 5, SSD1306_WHITE);
        displayManager.getDisplay().fillCircle(42, 33, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(90, 33, 3, SSD1306_BLACK);
        displayManager.getDisplay().drawCircle(64, 50, 5, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    surprisedFrame = (surprisedFrame + 1) % 51;
    lastSurprisedAnim = currentTime;
  }
}

// Animated music state - SANGI humming with closed eyes, swaying left and right with music notes
void AnimationManager::animateMusic() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastMusicAnim > 30) {  // Match other animations' frame rate for smooth motion
    displayManager.clearDisplay();
    
    // Helper function to draw a music note at position
    auto drawMusicNote = [&](int x, int y, int size) {
      if (size == 1) {
        // Small note
        displayManager.getDisplay().fillCircle(x, y + 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(x + 2, y + 4, x + 2, y, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(x + 2, y, x + 4, y + 1, SSD1306_WHITE);
      } else if (size == 2) {
        // Medium note
        displayManager.getDisplay().fillCircle(x, y + 5, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(x + 2, y, 2, 6, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(x + 2, y, 4, 2, SSD1306_WHITE);
      } else {
        // Large note
        displayManager.getDisplay().fillCircle(x, y + 6, 4, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(x + 3, y, 3, 7, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(x + 3, y, 5, 3, SSD1306_WHITE);
      }
    };
    
    switch(musicFrame) {
      // === EYES CLOSING (frames 0-5) ===
      case 0:
        // Normal eyes, happy mouth
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes starting to close
        displayManager.drawEyes(40, 29, 88, 29, 14);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        break;
        
      case 2:
        // Eyes closing more
        displayManager.drawEyes(40, 30, 88, 30, 10);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        break;
        
      case 3:
        // Eyes almost closed
        displayManager.drawEyes(40, 31, 88, 31, 6);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        break;
        
      case 4:
      case 5:
        // Eyes fully closed (peaceful expression)
        displayManager.drawEyes(40, 31, 88, 31, 3);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        // First music note appears bottom right
        drawMusicNote(110, 50, 1);
        break;
        
      // === SLOW SWAY: CENTER → LEFT (frames 6-17) ===
      case 6:
      case 7:
        // Very slight shift left, eyes starting to lower
        displayManager.drawEyes(39, 31, 87, 31, 3);  // Barely moved
        displayManager.getDisplay().fillRoundRect(51, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(115, 50, 1);
        break;
        
      case 8:
      case 9:
        // Gradually shifting left, eyes lowering more
        displayManager.drawEyes(38, 32, 86, 32, 3);  // Y+1 (lowering)
        displayManager.getDisplay().fillRoundRect(50, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(116, 45, 1);
        break;
        
      case 10:
      case 11:
        // Continuing left, eyes at lowest point
        displayManager.drawEyes(37, 33, 85, 33, 3);  // Y+2 (lowest)
        displayManager.getDisplay().fillRoundRect(49, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(118, 38, 1);
        break;
        
      case 12:
      case 13:
        // More to the left, eyes still low
        displayManager.drawEyes(36, 33, 84, 33, 3);  // Staying at lowest
        displayManager.getDisplay().fillRoundRect(48, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(120, 30, 2);
        drawMusicNote(12, 52, 1);
        break;
        
      case 14:
      case 15:
        // Almost at leftmost position, eyes starting to rise
        displayManager.drawEyes(35, 32, 83, 32, 3);  // Y+1 (rising)
        displayManager.getDisplay().fillRoundRect(47, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(122, 22, 1);
        drawMusicNote(10, 48, 1);
        break;
        
      case 16:
      case 17:
        // At leftmost position, eyes back to normal height
        displayManager.drawEyes(34, 31, 82, 31, 3);  // Back to normal Y
        displayManager.getDisplay().fillRoundRect(46, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(124, 14, 1);
        drawMusicNote(8, 42, 1);
        break;
        
      // === SLOW SWAY: LEFT → RIGHT (frames 18-29) ===
      case 18:
      case 19:
        // Start moving back toward center from left
        displayManager.drawEyes(35, 31, 83, 31, 3);
        displayManager.getDisplay().fillRoundRect(47, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(126, 8, 1);
        drawMusicNote(6, 36, 1);
        break;
        
      case 20:
      case 21:
        // Continuing right, eyes lowering
        displayManager.drawEyes(36, 32, 84, 32, 3);  // Y+1
        displayManager.getDisplay().fillRoundRect(48, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(6, 28, 2);
        drawMusicNote(112, 52, 1);
        break;
        
      case 22:
      case 23:
        // Moving through center, eyes at lowest
        displayManager.drawEyes(38, 33, 86, 33, 3);  // Y+2 (lowest)
        displayManager.getDisplay().fillRoundRect(50, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(4, 20, 1);
        drawMusicNote(116, 48, 1);
        break;
        
      case 24:
      case 25:
        // Past center toward right, eyes still low
        displayManager.drawEyes(40, 33, 88, 33, 3);  // At center, still low
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(6, 12, 1);
        drawMusicNote(118, 42, 1);
        break;
        
      case 26:
      case 27:
        // Continuing right, eyes rising
        displayManager.drawEyes(42, 32, 90, 32, 3);  // Y+1 (rising)
        displayManager.getDisplay().fillRoundRect(54, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(4, 6, 1);
        drawMusicNote(120, 36, 2);
        // New note bottom left
        drawMusicNote(10, 52, 1);
        break;
        
      case 28:
      case 29:
        // More to the right, eyes at normal height
        displayManager.drawEyes(44, 31, 92, 31, 3);  // Back to normal Y
        displayManager.getDisplay().fillRoundRect(56, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(122, 30, 1);
        break;
        
      // === SLOW SWAY: RIGHT → LEFT (frames 30-41) ===
      case 30:
      case 31:
        // Start moving back toward center from right
        displayManager.drawEyes(43, 31, 91, 31, 3);
        displayManager.getDisplay().fillRoundRect(55, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(124, 24, 1);
        drawMusicNote(10, 52, 1);
        break;
        
      case 32:
      case 33:
        // Continuing left, eyes lowering
        displayManager.drawEyes(42, 32, 90, 32, 3);  // Y+1
        displayManager.getDisplay().fillRoundRect(54, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(126, 16, 1);
        drawMusicNote(8, 46, 1);
        break;
        
      case 34:
      case 35:
        // Moving through center, eyes at lowest
        displayManager.drawEyes(40, 33, 88, 33, 3);  // Y+2 (lowest)
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(124, 10, 1);
        drawMusicNote(6, 38, 2);
        break;
        
      case 36:
      case 37:
        // Past center toward left, eyes still low
        displayManager.drawEyes(38, 33, 86, 33, 3);  // Still at lowest
        displayManager.getDisplay().fillRoundRect(50, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(122, 6, 1);
        drawMusicNote(6, 30, 1);
        break;
        
      case 38:
      case 39:
        // Continuing left, eyes rising
        displayManager.drawEyes(36, 32, 84, 32, 3);  // Y+1 (rising)
        displayManager.getDisplay().fillRoundRect(48, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(6, 20, 1);
        drawMusicNote(120, 20, 1);
        break;
        
      case 40:
      case 41:
        // More to the left, eyes back to normal height
        displayManager.drawEyes(35, 31, 83, 31, 3);  // Back to normal Y
        displayManager.getDisplay().fillRoundRect(47, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(6, 12, 1);
        drawMusicNote(122, 12, 1);
        break;
        
      // === SLOW SWAY: LEFT → CENTER (frames 42-50) ===
      case 42:
      case 43:
        // Starting to return to center from left
        displayManager.drawEyes(36, 31, 84, 31, 3);
        displayManager.getDisplay().fillRoundRect(48, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(6, 8, 1);
        drawMusicNote(122, 8, 1);
        break;
        
      case 44:
      case 45:
        // Gradually moving toward center, eyes lowering
        displayManager.drawEyes(37, 32, 85, 32, 3);  // Y+1
        displayManager.getDisplay().fillRoundRect(49, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(8, 14, 1);
        drawMusicNote(120, 14, 1);
        break;
        
      case 46:
      case 47:
        // Almost at center, eyes at lowest
        displayManager.drawEyes(38, 33, 86, 33, 3);  // Y+2 (lowest)
        displayManager.getDisplay().fillRoundRect(50, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(8, 20, 1);
        drawMusicNote(118, 20, 1);
        break;
        
      case 48:
      case 49:
        // Almost centered, eyes rising
        displayManager.drawEyes(39, 32, 87, 32, 3);  // Y+1 (rising)
        displayManager.getDisplay().fillRoundRect(51, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(10, 26, 1);
        drawMusicNote(116, 26, 1);
        break;
        
      case 50:
        // Back to center, eyes normal - ready to loop
        displayManager.drawEyes(40, 31, 88, 31, 3);
        displayManager.getDisplay().fillRoundRect(52, 50, 24, 6, 3, SSD1306_WHITE);
        drawMusicNote(12, 30, 1);
        drawMusicNote(114, 30, 1);
        break;
    }
    
    displayManager.updateDisplay();
    musicFrame = (musicFrame + 1) % 51;
    lastMusicAnim = currentTime;
  }
}

// Animated dead state - X eyes with tongue sticking out
void AnimationManager::animateDead() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastDeadAnim > 30) {
    displayManager.clearDisplay();
    
    switch(deadFrame) {
      // === INITIAL COLLAPSE (frames 0-8) ===
      case 0:
        // Normal eyes start to droop
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawCircle(64, 50, 4, SSD1306_WHITE);
        break;
        
      case 1: case 2:
        // Eyes drooping
        displayManager.drawEyes(40, 29, 88, 29, 16);
        displayManager.getDisplay().drawCircle(64, 50, 4, SSD1306_WHITE);
        break;
        
      case 3: case 4:
        // Eyes closing more
        displayManager.drawEyes(40, 30, 88, 30, 12);
        displayManager.getDisplay().drawCircle(64, 50, 4, SSD1306_WHITE);
        break;
        
      case 5: case 6:
        // Eyes nearly closed
        displayManager.drawEyes(40, 31, 88, 31, 8);
        displayManager.getDisplay().drawCircle(64, 51, 5, SSD1306_WHITE);
        break;
        
      case 7: case 8:
        // Eyes transitioning to X - first diagonal lines appear
        displayManager.getDisplay().fillRoundRect(38, 30, 18, 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(86, 30, 18, 4, 2, SSD1306_WHITE);
        displayManager.getDisplay().drawCircle(64, 51, 5, SSD1306_WHITE);
        break;
        
      // === X EYES FORMING (frames 9-15) ===
      case 9: case 10:
        // X eyes starting to form - shifted upwards
        // Left X eye - diagonal from top-left to bottom-right
        for(int i = 0; i < 5; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        // Right X eye - diagonal from top-left to bottom-right
        for(int i = 0; i < 5; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().drawCircle(64, 50, 5, SSD1306_WHITE);
        break;
        
      case 11: case 12: case 13: case 14: case 15:
        // Full X eyes formed - thick and bold, shifted upwards
        // Left X eye - very thick crossed lines (6 pixels thick)
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        // Right X eye - very thick crossed lines (6 pixels thick)
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        // Default mouth style (circle outline)
        displayManager.getDisplay().drawCircle(64, 50, 6, SSD1306_WHITE);
        break;
        
      // === TONGUE STARTING TO STICK OUT (frames 16-25) ===
      case 16: case 17: case 18:
        // X eyes stable - thick and shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        // Mouth open with default style, tongue tip appears
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(62, 54, 4, 2, SSD1306_WHITE);
        break;
        
      case 19: case 20: case 21:
        // Tongue extending - X eyes shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        // Tongue extending more
        displayManager.getDisplay().fillRoundRect(61, 54, 6, 4, 2, SSD1306_WHITE);
        break;
        
      case 22: case 23: case 24: case 25:
        // Tongue fully extended - X eyes shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        // Tongue sticking out fully
        displayManager.getDisplay().fillRoundRect(60, 54, 8, 6, 3, SSD1306_WHITE);
        break;
        
      // === HOLDING DEAD EXPRESSION (frames 26-40) ===
      case 26: case 27: case 28: case 29: case 30:
      case 31: case 32: case 33: case 34: case 35:
      case 36: case 37: case 38: case 39: case 40:
        // X eyes and tongue out - shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(60, 54, 8, 6, 3, SSD1306_WHITE);
        
        // Add occasional "ghost" effect (alternating frames)
        if (deadFrame % 8 < 4) {
          // Small spirals near head
          displayManager.getDisplay().drawCircle(20, 20, 3, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(108, 20, 3, SSD1306_WHITE);
        } else {
          // Different spiral positions
          displayManager.getDisplay().drawCircle(18, 24, 3, SSD1306_WHITE);
          displayManager.getDisplay().drawCircle(110, 24, 3, SSD1306_WHITE);
        }
        break;
        
      // === TONGUE RETRACTING (frames 41-50) ===
      case 41: case 42:
        // Tongue starts retracting - X eyes shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRoundRect(61, 54, 6, 4, 2, SSD1306_WHITE);
        break;
        
      case 43: case 44:
        // Tongue retracting more - X eyes shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(62, 54, 4, 2, SSD1306_WHITE);
        break;
        
      case 45: case 46: case 47: case 48: case 49: case 50:
        // Tongue fully retracted, X eyes shifted upwards
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(32, 20 + i, 50, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(32, 36 + i, 50, 20 + i, SSD1306_WHITE);
        }
        for(int i = 0; i < 6; i++) {
          displayManager.getDisplay().drawLine(78, 20 + i, 96, 36 + i, SSD1306_WHITE);
          displayManager.getDisplay().drawLine(78, 36 + i, 96, 20 + i, SSD1306_WHITE);
        }
        displayManager.getDisplay().fillRoundRect(56, 50, 16, 6, 3, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    deadFrame = (deadFrame + 1) % 51;
    lastDeadAnim = currentTime;
  }
}

// Animated notification - SANGI reacts, runs away, notification appears (2.6s display), then returns calm to idle
// 86 frames @ 50ms = 4.3s total (extended content display for readability)
void AnimationManager::animateNotification(const char* title, const char* message) {
  unsigned long currentTime = millis();
  
  // Slower animation for dramatic effect - 50ms per frame (4.3s total)
  if (currentTime - lastNotificationAnim > 50) {
    displayManager.clearDisplay();
    
    switch(notificationFrame) {
      // ===== PHASE 1: SURPRISE REACTION (frames 0-5) =====
      case 0:
        // Normal idle state
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
        
      case 1:
        // Eyes start widening (notification alert!)
        displayManager.drawEyes(40, 27, 88, 27, 22);
        displayManager.getDisplay().fillCircle(40, 27, 2, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 27, 2, SSD1306_BLACK);
        displayManager.getDisplay().drawCircle(64, 48, 6, SSD1306_WHITE);
        break;
        
      case 2:
      case 3:
        // Eyes WIDE (startled!)
        displayManager.drawEyes(40, 26, 88, 26, 26);
        displayManager.getDisplay().fillCircle(40, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(88, 26, 3, SSD1306_BLACK);
        displayManager.getDisplay().fillCircle(64, 50, 8, SSD1306_WHITE);
        break;
        
      case 4:
      case 5:
        // Eyes squinting (preparing to run!)
        displayManager.drawEyes(40, 28, 88, 28, 12);
        displayManager.getDisplay().drawLine(52, 50, 76, 50, SSD1306_WHITE);
        break;
        
      // ===== PHASE 2: RUN AWAY (frames 6-12) =====
      case 6:
        // Start moving right
        displayManager.drawEyes(50, 28, 98, 28, 14);
        displayManager.getDisplay().drawLine(62, 50, 86, 50, SSD1306_WHITE);
        break;
        
      case 7:
        // Moving further right
        displayManager.drawEyes(60, 28, 108, 28, 14);
        displayManager.getDisplay().drawLine(72, 50, 96, 50, SSD1306_WHITE);
        break;
        
      case 8:
        // Almost off-screen
        displayManager.drawEyes(75, 28, 123, 28, 14);
        displayManager.getDisplay().drawLine(87, 50, 111, 50, SSD1306_WHITE);
        break;
        
      case 9:
      case 10:
      case 11:
      case 12:
        // Completely off-screen (blank)
        break;
        
      // ===== PHASE 3: NOTIFICATION BOARD SLIDES IN (frames 13-18) =====
      case 13:
        // Notification board edge appears from left
        displayManager.getDisplay().fillRect(0, 8, 10, 50, SSD1306_WHITE);
        break;
        
      case 14:
        // Board slides in more
        displayManager.getDisplay().drawRect(0, 8, 40, 50, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(1, 9, 38, 48, SSD1306_WHITE);
        break;
        
      case 15:
        // Board halfway in
        displayManager.getDisplay().drawRect(0, 8, 80, 50, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(1, 9, 78, 48, SSD1306_WHITE);
        break;
        
      case 16:
        // Board almost fully in
        displayManager.getDisplay().drawRect(2, 8, 120, 50, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(3, 9, 118, 48, SSD1306_WHITE);
        break;
        
      case 17:
      case 18:
        // Board fully in position
        displayManager.getDisplay().drawRect(4, 8, 120, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(5, 9, 118, 50, SSD1306_WHITE);
        break;
        
      // ===== PHASE 4: DISPLAY NOTIFICATION (frames 19-70) - Extended for readability =====
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
      case 32:
      case 33:
      case 34:
      case 35:
      case 36:
      case 37:
      case 38:
      case 39:
      case 40:
      case 41:
      case 42:
      case 43:
      case 44:
      case 45:
      case 46:
      case 47:
      case 48:
      case 49:
      case 50:
      case 51:
      case 52:
      case 53:
      case 54:
      case 55:
      case 56:
      case 57:
      case 58:
      case 59:
      case 60:
      case 61:
      case 62:
      case 63:
      case 64:
      case 65:
      case 66:
      case 67:
      case 68:
      case 69:
      case 70:
        // Show notification content (full screen board)
        displayManager.getDisplay().drawRect(4, 8, 120, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(5, 9, 118, 50, SSD1306_WHITE);
        
        // Display text content with larger font
        displayManager.getDisplay().setTextSize(2);  // Larger font for better readability
        displayManager.getDisplay().setTextColor(SSD1306_WHITE);
        
        if (title && strlen(title) > 0) {
          displayManager.getDisplay().setCursor(10, 15);
          displayManager.getDisplay().print(title);
        }
        
        displayManager.getDisplay().setTextSize(1);  // Smaller font for message
        if (message && strlen(message) > 0) {
          displayManager.getDisplay().setCursor(10, 38);
          displayManager.getDisplay().print(message);
        }
        break;
        
      // ===== PHASE 5: BOARD SLIDES OUT (frames 71-76) =====
      case 71:
        // Board starts sliding out to right
        displayManager.getDisplay().drawRect(8, 8, 116, 52, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(9, 9, 114, 50, SSD1306_WHITE);
        break;
        
      case 72:
        // Board sliding out more
        displayManager.getDisplay().drawRect(20, 8, 100, 50, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(21, 9, 98, 48, SSD1306_WHITE);
        break;
        
      case 73:
        // Board halfway out
        displayManager.getDisplay().drawRect(50, 8, 70, 50, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(51, 9, 68, 48, SSD1306_WHITE);
        break;
        
      case 74:
        // Board almost gone
        displayManager.getDisplay().drawRect(90, 8, 34, 50, SSD1306_WHITE);
        displayManager.getDisplay().drawRect(91, 9, 32, 48, SSD1306_WHITE);
        break;
        
      case 75:
      case 76:
        // Board completely off-screen
        break;
        
      // ===== PHASE 6: SANGI RETURNS CALM (frames 77-85) =====
      case 77:
        // Eyes appear from left
        displayManager.drawEyes(10, 28, 58, 28, 14);
        displayManager.getDisplay().drawCircle(34, 48, 4, SSD1306_WHITE);
        break;
        
      case 78:
        // Moving to center
        displayManager.drawEyes(20, 28, 68, 28, 16);
        displayManager.getDisplay().drawCircle(44, 48, 4, SSD1306_WHITE);
        break;
        
      case 79:
      case 80:
        // Almost centered
        displayManager.drawEyes(30, 28, 78, 28, 17);
        displayManager.getDisplay().drawCircle(54, 48, 5, SSD1306_WHITE);
        break;
        
      case 81:
      case 82:
      case 83:
      case 84:
      case 85:
        // Fully centered - return to idle state
        displayManager.drawEyes(40, 28, 88, 28, 18);
        displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
        break;
    }
    
    displayManager.updateDisplay();
    
    // Play animation as single sequence, hold on final frame (idle state)
    if (notificationFrame < 85) {
      notificationFrame++;
    }
    // If frame is 85, hold there (don't loop back to 0)
    
    lastNotificationAnim = currentTime;
  }
}

// Matrix-inspired coding animation - focused face with glasses and falling binary digits
void AnimationManager::animateCoding() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastCodingAnim > 100) {  // 100ms per frame for smooth binary fall
    displayManager.clearDisplay();
    
    // Face position (centered, matching standard emotions)
    int leftEyeX = 40, leftEyeY = 28;
    int rightEyeX = 88, rightEyeY = 28;
    int eyeHeight = 14;  // Squinted for focused look
    
    // FIRST: Draw the face with glasses based on current frame
    switch(codingFrame % 25) {  // 25-frame loop (faster cycle)
      // === NORMAL FOCUSED STATE (frames 0-10) ===
      case 0: case 1: case 2: case 3: case 4:
      case 5: case 6: case 7: case 8: case 9: case 10:
        // Draw squinted eyes (focused)
        displayManager.drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeHeight);
        
        // Draw THICKER and LARGER glasses - 3 layers for thickness
        // Left lens - LARGER: 28x24 (was 24x20)
        displayManager.getDisplay().drawRoundRect(leftEyeX - 14, leftEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 13, leftEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 12, leftEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        
        // Right lens - LARGER: 28x24 (was 24x20)
        displayManager.getDisplay().drawRoundRect(rightEyeX - 14, rightEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 13, rightEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 12, rightEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        
        // Thicker bridge connecting lenses
        displayManager.getDisplay().fillRect(52, leftEyeY - 2, 24, 3, SSD1306_WHITE);
        
        // Standard mouth (28x8 to match other emotions)
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        break;
        
      // === QUICK BLINK 1 (frame 11 only) - VERY FAST ===
      case 11:
        // Eyes closed - single frame blink like surprised
        displayManager.drawEyes(leftEyeX, leftEyeY + 5, rightEyeX, rightEyeY + 5, 3);
        
        // Thicker and larger glasses
        displayManager.getDisplay().drawRoundRect(leftEyeX - 14, leftEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 13, leftEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 12, leftEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 14, rightEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 13, rightEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 12, rightEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(52, leftEyeY - 2, 24, 3, SSD1306_WHITE);
        
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        break;
        
      // === NORMAL FOCUSED STATE (frames 12-16) ===
      case 12: case 13: case 14: case 15: case 16:
        displayManager.drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeHeight);
        
        // Thicker and larger glasses
        displayManager.getDisplay().drawRoundRect(leftEyeX - 14, leftEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 13, leftEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 12, leftEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 14, rightEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 13, rightEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 12, rightEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(52, leftEyeY - 2, 24, 3, SSD1306_WHITE);
        
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        break;
        
      // === QUICK BLINK 2 (frame 17 only) - VERY FAST ===
      case 17:
        // Eyes closed - single frame blink
        displayManager.drawEyes(leftEyeX, leftEyeY + 5, rightEyeX, rightEyeY + 5, 3);
        
        // Thicker and larger glasses
        displayManager.getDisplay().drawRoundRect(leftEyeX - 14, leftEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 13, leftEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 12, leftEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 14, rightEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 13, rightEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 12, rightEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(52, leftEyeY - 2, 24, 3, SSD1306_WHITE);
        
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        break;
        
      case 18:
      default:
        // Back to focused for remaining frames
        displayManager.drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeHeight);
        
        // Thicker and larger glasses
        displayManager.getDisplay().drawRoundRect(leftEyeX - 14, leftEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 13, leftEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(leftEyeX - 12, leftEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 14, rightEyeY - 12, 28, 24, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 13, rightEyeY - 11, 26, 22, 4, SSD1306_WHITE);
        displayManager.getDisplay().drawRoundRect(rightEyeX - 12, rightEyeY - 10, 24, 20, 3, SSD1306_WHITE);
        displayManager.getDisplay().fillRect(52, leftEyeY - 2, 24, 3, SSD1306_WHITE);
        
        displayManager.getDisplay().fillRoundRect(50, 48, 28, 8, 4, SSD1306_WHITE);
        break;
    }
    
    // SECOND: Draw falling binary digits AFTER the face (so they appear on top)
    // 6 columns of falling 0s and 1s
    static int binaryY[6] = {0, 10, 20, 5, 15, 25};  // Different starting Y positions for each column
    
    // Column X positions (avoiding center face area)
    int colX[6] = {5, 25, 45, 83, 103, 118};
    
    // Draw falling binary in columns
    displayManager.getDisplay().setTextSize(1);
    displayManager.getDisplay().setTextColor(SSD1306_WHITE);
    
    for (int col = 0; col < 6; col++) {
      // Draw 3 digits per column for trail effect
      for (int trail = 0; trail < 3; trail++) {
        int y = (binaryY[col] + trail * 15) % 70;
        if (y < 64) {
          displayManager.getDisplay().setCursor(colX[col], y);
          // Alternate 0 and 1 based on column and trail
          if ((col + trail + codingFrame) % 2 == 0) {
            displayManager.getDisplay().print("0");
          } else {
            displayManager.getDisplay().print("1");
          }
        }
      }
      // Move each column down
      binaryY[col] = (binaryY[col] + 2) % 70;
    }
    
    displayManager.updateDisplay();
    
    codingFrame++;
    lastCodingAnim = currentTime;
  }
}

// Animated GitHub contribution graph - displays heat map like GitHub
void AnimationManager::animateGitHubStats() {
  unsigned long currentTime = millis();
  


  // Animation timing: 50ms per frame for smoothness
  if (currentTime - lastGitHubStatsAnim > 50) {
    displayManager.clearDisplay();
    extern NetworkManager networkManager;
    GitHubContributionData* githubData = networkManager.getGitHubData();

  // PHASES (extended):
  // 0-9: Sangi transitions from neutral to scared (wide eyes, squint, mouth)
  // 10-19: Sangi runs off-screen (moves right)
  // 20-29: 'github stats' text slides in from left
  // 30-49: Text holds center
  // 50-59: Text slides out to right
  // 60-69: Stats boxes slide in from left
  // 70-109: Stats boxes hold
  // 110-119: Stats boxes slide out to right
  // 120-139: Sangi returns using angry emotion


    // --- PHASE 1: Sangi transitions to scared ---
    if (githubStatsFrame <= 9) {
      int frame = githubStatsFrame;
      int eyeHeight = 18 - frame;
      int mouthY = 48 + frame/2;
      int mouthR = 5 + frame/2;
      displayManager.drawEyes(40, 28, 88, 28, eyeHeight);
      if (frame < 4) {
        displayManager.getDisplay().drawCircle(64, mouthY, mouthR, SSD1306_WHITE);
      } else if (frame < 7) {
        displayManager.getDisplay().drawLine(54, mouthY, 74, mouthY, SSD1306_WHITE);
      } else {
        displayManager.getDisplay().fillRoundRect(54, mouthY, 20, 5, 2, SSD1306_WHITE);
      }
      displayManager.getDisplay().drawLine(30, 18, 60, 18, SSD1306_WHITE);
      displayManager.getDisplay().drawLine(68, 18, 98, 18, SSD1306_WHITE);
    }

    // --- PHASE 2: Sangi runs off-screen ---
    else if (githubStatsFrame <= 19) {
      int offset = (githubStatsFrame - 10) * 7; // Move 0 to 70px right
      displayManager.drawEyes(40 + offset, 28, 88 + offset, 28, 10);
      displayManager.getDisplay().fillRoundRect(54 + offset, 52, 20, 5, 2, SSD1306_WHITE);
      displayManager.getDisplay().drawLine(30 + offset, 18, 60 + offset, 18, SSD1306_WHITE);
      displayManager.getDisplay().drawLine(68 + offset, 18, 98 + offset, 18, SSD1306_WHITE);
    }

    // --- PHASE 3: 'github stats' text slides in ---
    else if (githubStatsFrame <= 29) {
      int slide = 128 - ((githubStatsFrame - 20) * 6);
      if (slide < 0) slide = 0;
      displayManager.getDisplay().setTextSize(2);
      displayManager.getDisplay().setTextColor(SSD1306_WHITE);
      displayManager.getDisplay().setCursor(slide, 10);
      displayManager.getDisplay().print("github stats");
    }

    // --- PHASE 4: Text holds center ---
    else if (githubStatsFrame <= 49) {
      displayManager.getDisplay().setTextSize(2);
      displayManager.getDisplay().setTextColor(SSD1306_WHITE);
      displayManager.getDisplay().setCursor(10, 10);
      displayManager.getDisplay().print("github stats");
    }

    // --- PHASE 5: Text slides out ---
    else if (githubStatsFrame <= 59) {
      int slide = (githubStatsFrame - 50) * 6;
      displayManager.getDisplay().setTextSize(2);
      displayManager.getDisplay().setTextColor(SSD1306_WHITE);
      displayManager.getDisplay().setCursor(10 + slide, 10);
      displayManager.getDisplay().print("github stats");
    }

    // --- PHASE 6: Stats boxes slide in ---
    else if (githubStatsFrame <= 69) {
      if (githubData == nullptr || !networkManager.hasGitHubData()) {
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setTextColor(SSD1306_WHITE);
        displayManager.getDisplay().setCursor(8, 20);
        displayManager.getDisplay().println("No GitHub data");
        displayManager.getDisplay().setCursor(15, 35);
        displayManager.getDisplay().println("Waiting...");
        displayManager.updateDisplay();
        lastGitHubStatsAnim = currentTime;
        githubStatsFrame++;
        return;
      }
      int slide = 128 - ((githubStatsFrame - 60) * 13);
      if (slide < 0) slide = 0;
      int totalDays = 21;
      int daysPerRow = 7;
      int numRows = 3;
      int cellWidth = 17;
      int cellHeight = 20;
      int cellGapX = 1;
      int cellGapY = 2;
      int gridStartX = 1;
      int gridStartY = 0;
      int startWeek = 49;
      for (int row = 0; row < numRows; row++) {
        int dataWeek = startWeek + row;
        if (dataWeek >= 52) dataWeek = 51;
        for (int col = 0; col < daysPerRow; col++) {
          int x = gridStartX + (col * (cellWidth + cellGapX)) - slide;
          int y = gridStartY + (row * (cellHeight + cellGapY));
          uint8_t level = githubData->contributions[dataWeek][col];
          if (level > 0) {
            displayManager.getDisplay().fillRect(x, y, cellWidth, cellHeight, SSD1306_WHITE);
          } else {
            displayManager.getDisplay().drawRect(x, y, cellWidth, cellHeight, SSD1306_WHITE);
          }
        }
      }
    }

    // --- PHASE 7: Stats boxes hold ---
    else if (githubStatsFrame <= 109) {
      if (githubData == nullptr || !networkManager.hasGitHubData()) {
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setTextColor(SSD1306_WHITE);
        displayManager.getDisplay().setCursor(8, 20);
        displayManager.getDisplay().println("No GitHub data");
        displayManager.getDisplay().setCursor(15, 35);
        displayManager.getDisplay().println("Waiting...");
        displayManager.updateDisplay();
        lastGitHubStatsAnim = currentTime;
        githubStatsFrame++;
        return;
      }
      int totalDays = 21;
      int daysPerRow = 7;
      int numRows = 3;
      int cellWidth = 17;
      int cellHeight = 20;
      int cellGapX = 1;
      int cellGapY = 2;
      int gridStartX = 1;
      int gridStartY = 0;
      int startWeek = 49;
      for (int row = 0; row < numRows; row++) {
        int dataWeek = startWeek + row;
        if (dataWeek >= 52) dataWeek = 51;
        for (int col = 0; col < daysPerRow; col++) {
          int x = gridStartX + (col * (cellWidth + cellGapX));
          int y = gridStartY + (row * (cellHeight + cellGapY));
          uint8_t level = githubData->contributions[dataWeek][col];
          if (level > 0) {
            displayManager.getDisplay().fillRect(x, y, cellWidth, cellHeight, SSD1306_WHITE);
          } else {
            displayManager.getDisplay().drawRect(x, y, cellWidth, cellHeight, SSD1306_WHITE);
          }
        }
      }
    }

    // --- PHASE 8: Stats boxes slide out ---
    else if (githubStatsFrame <= 119) {
      if (githubData == nullptr || !networkManager.hasGitHubData()) {
        displayManager.getDisplay().setTextSize(1);
        displayManager.getDisplay().setTextColor(SSD1306_WHITE);
        displayManager.getDisplay().setCursor(8, 20);
        displayManager.getDisplay().println("No GitHub data");
        displayManager.getDisplay().setCursor(15, 35);
        displayManager.getDisplay().println("Waiting...");
        displayManager.updateDisplay();
        lastGitHubStatsAnim = currentTime;
        githubStatsFrame++;
        return;
      }
      int slide = (githubStatsFrame - 110) * 13;
      int totalDays = 21;
      int daysPerRow = 7;
      int numRows = 3;
      int cellWidth = 17;
      int cellHeight = 20;
      int cellGapX = 1;
      int cellGapY = 2;
      int gridStartX = 1;
      int gridStartY = 0;
      int startWeek = 49;
      for (int row = 0; row < numRows; row++) {
        int dataWeek = startWeek + row;
        if (dataWeek >= 52) dataWeek = 51;
        for (int col = 0; col < daysPerRow; col++) {
          int x = gridStartX + (col * (cellWidth + cellGapX)) + slide;
          int y = gridStartY + (row * (cellHeight + cellGapY));
          uint8_t level = githubData->contributions[dataWeek][col];
          if (level > 0) {
            displayManager.getDisplay().fillRect(x, y, cellWidth, cellHeight, SSD1306_WHITE);
          } else {
            displayManager.getDisplay().drawRect(x, y, cellWidth, cellHeight, SSD1306_WHITE);
          }
        }
      }
    }

    // --- PHASE 9: Sangi returns using angry emotion ---
    else if (githubStatsFrame <= 139) {
      int angryPhase = githubStatsFrame - 120;
      int eyeY = 32 - angryPhase/2;
      int browY = 16 + angryPhase/4;
      int mouthY = 52 + angryPhase/4;
      displayManager.drawEyes(40, eyeY, 88, eyeY, 11 + angryPhase/4);
      for(int i = 0; i < 5; i++) {
        displayManager.getDisplay().drawLine(22, browY + i, 52, browY + 6 + i, SSD1306_WHITE);
        displayManager.getDisplay().drawLine(76, browY + 6 + i, 106, browY + i, SSD1306_WHITE);
      }
      displayManager.getDisplay().fillRoundRect(52, mouthY, 24, 5, 2, SSD1306_WHITE);
    }

    // --- PHASE 10: Hold idle Sangi ---
    else {
      displayManager.drawEyes(40, 28, 88, 28, 18);
      displayManager.getDisplay().drawCircle(64, 48, 5, SSD1306_WHITE);
    }

    displayManager.updateDisplay();
    githubStatsFrame++;
    lastGitHubStatsAnim = currentTime;
  }
}

