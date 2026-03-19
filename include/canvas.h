#ifndef CANVAS_H
#define CANVAS_H

// Abstract canvas interface for display operations.
// Allows animation logic to be unit-tested on desktop without OLED hardware.
// DisplayManager implements this; MockCanvas (in test/) can substitute.

#include <stdint.h>

// Color constants (match SSD1306 values: BLACK=0, WHITE=1)
static const uint16_t COLOR_BLACK = 0;
static const uint16_t COLOR_WHITE = 1;

class ICanvas {
public:
  virtual ~ICanvas() = default;

  // --- Pure virtual primitives (hardware-specific) ---
  virtual void clear() = 0;
  virtual void flush() = 0;

  virtual void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                             int16_t r, uint16_t color) = 0;
  virtual void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                             int16_t r, uint16_t color) = 0;
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color) = 0;
  virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,
                        uint16_t color) = 0;
  virtual void fillCircle(int16_t x, int16_t y, int16_t r,
                          uint16_t color) = 0;
  virtual void drawCircle(int16_t x, int16_t y, int16_t r,
                          uint16_t color) = 0;
  virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                        uint16_t color) = 0;
  virtual void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            int16_t x2, int16_t y2, uint16_t color) = 0;

  virtual void setTextSize(uint8_t size) = 0;
  virtual void setCursor(int16_t x, int16_t y) = 0;
  virtual void setTextColor(uint16_t color) = 0;
  virtual void print(const char* text) = 0;
  virtual void println(const char* text) = 0;

  // --- Concrete helpers (implemented via primitives above) ---

  // Standard rounded-rect eye pair used across nearly all emotions
  void drawEyes(int leftX, int leftY, int rightX, int rightY, int eyeHeight) {
    fillRoundRect(leftX - 10, leftY - eyeHeight / 2, 20, eyeHeight, 5,
                  COLOR_WHITE);
    fillRoundRect(rightX - 10, rightY - eyeHeight / 2, 20, eyeHeight, 5,
                  COLOR_WHITE);
  }

  // Eyes with circular pupils (excited, surprised)
  void drawEyesWithPupils(int leftX, int leftY, int rightX, int rightY,
                          int eyeHeight, int pupilR) {
    drawEyes(leftX, leftY, rightX, rightY, eyeHeight);
    fillCircle(leftX, leftY, pupilR, COLOR_BLACK);
    fillCircle(rightX, rightY, pupilR, COLOR_BLACK);
  }

  // Simple filled rounded-rect mouth
  void drawMouth(int x, int y, int w, int h, int r = 2) {
    fillRoundRect(x, y, w, h, r, COLOR_WHITE);
  }

  // Symmetrical blush circles
  void drawBlush(int leftX, int leftY, int rightX, int rightY, int r) {
    fillCircle(leftX, leftY, r, COLOR_WHITE);
    fillCircle(rightX, rightY, r, COLOR_WHITE);
  }

  // Symmetrical sparkle dots
  void drawSparkles(int leftX, int leftY, int rightX, int rightY,
                    int r = 2) {
    fillCircle(leftX, leftY, r, COLOR_WHITE);
    fillCircle(rightX, rightY, r, COLOR_WHITE);
  }
};

#endif // CANVAS_H
