#ifndef DISPLAY_H
#define DISPLAY_H

// DisplayManager — OLED rendering via Adafruit SSD1306.
// CHANGED in Phase 1:
//   - Implements ICanvas interface (virtual drawing primitives)
//   - performTransition now takes explicit params instead of reading
//     emotionManager globals, returning a result enum.
//   - drawEyes removed (inherited from ICanvas base class)
//   - clearDisplay/updateDisplay kept as inline aliases for compatibility

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "canvas.h"
#include "emotion.h"

// Returned by performTransitionFrame to tell main.cpp what to do
enum TransitionResult {
  TR_DREW_FRAME,  // Frame rendered, caller should advanceTransition()
  TR_COMPLETE      // Final frame rendered, caller should completeTransition()
};

// ===== DISPLAY MANAGER =====
class DisplayManager : public ICanvas {
public:
  DisplayManager();

  bool init();
  void scanI2C();
  void showBootScreen();
  void drawBatSignal();

  // --- ICanvas pure-virtual implementations ---
  void clear() override;
  void flush() override;
  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color) override;
  void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,
                     uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t color) override;
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t color) override;
  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;
  void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                uint16_t color) override;
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t color) override;
  void setTextSize(uint8_t size) override;
  void setCursor(int16_t x, int16_t y) override;
  void setTextColor(uint16_t color) override;
  void print(const char* text) override;
  void println(const char* text) override;

  // --- Static face drawing (will migrate to EmotionRegistry in Phase 2) ---
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
  void drawFace_Notification(const char* title = "", const char* message = "");

  // Emotion-based dispatch
  void drawEmotionFace(EmotionState emotion);

  // Transition animation — decoupled from EmotionManager.
  // main.cpp reads emotion state and passes it here; return value tells
  // main.cpp whether to advance or complete the transition.
  TransitionResult performTransitionFrame(int frame, EmotionState current,
                                          EmotionState target);

  // Backward-compat aliases (used by animations.cpp until Phase 2 migration)
  void clearDisplay() { clear(); }
  void updateDisplay() { flush(); }

  // Raw display access (used by animations.cpp until Phase 2 migration)
  Adafruit_SSD1306& getDisplay() { return display; }

private:
  Adafruit_SSD1306 display;

  // Internal transition helpers
  TransitionResult sleepyTransitionFrame(int frame, EmotionState target);
  TransitionResult notificationTransitionFrame(int frame,
                                               EmotionState current);
};

extern DisplayManager displayManager;

#endif // DISPLAY_H
