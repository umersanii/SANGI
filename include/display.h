#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "emotion.h"

// ===== DISPLAY MANAGER =====
class DisplayManager {
public:
  DisplayManager();
  
  bool init();
  void scanI2C();
  void showBootScreen();
  
  // Basic drawing functions
  void drawEyes(int leftX, int leftY, int rightX, int rightY, int eyeHeight);
  void clearDisplay();
  void updateDisplay();
  
  // Static face drawing functions
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
  
  // Emotion-based drawing
  void drawEmotionFace(EmotionState emotion);
  
  // Transition animation
  void performTransition();
  
  Adafruit_SSD1306& getDisplay() { return display; }
  
private:
  Adafruit_SSD1306 display;
};

extern DisplayManager displayManager;

#endif // DISPLAY_H
