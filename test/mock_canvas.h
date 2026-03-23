#ifndef MOCK_CANVAS_H
#define MOCK_CANVAS_H

#include "canvas.h"
#include <stdio.h>
#include <string.h>

// Records drawing calls for assertion in tests.
struct DrawCall {
  enum Type {
    FILL_RRECT, FILL_CIRCLE, DRAW_CIRCLE, DRAW_LINE, FILL_RECT,
    FILL_TRIANGLE, TEXT, CLEAR, FLUSH, OTHER
  };
  Type type;
  int16_t x, y, w, h, r;
  uint16_t color;
  char text[32];
};

class MockCanvas : public ICanvas {
public:
  static const int MAX_CALLS = 200;

  MockCanvas() : callCount_(0), verbose_(false) { reset(); }

  void setVerbose(bool v) { verbose_ = v; }
  void reset() { callCount_ = 0; memset(calls_, 0, sizeof(calls_)); }

  int callCount() const { return callCount_; }
  const DrawCall& call(int i) const { return calls_[i]; }

  // Find first call of a given type
  int findCall(DrawCall::Type type, int startFrom = 0) const {
    for (int i = startFrom; i < callCount_; i++) {
      if (calls_[i].type == type) return i;
    }
    return -1;
  }

  // --- ICanvas implementation ---
  void clear() override { record(DrawCall::CLEAR); }
  void flush() override { record(DrawCall::FLUSH); }

  void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     int16_t r, uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::FILL_RRECT;
      dc.x = x; dc.y = y; dc.w = w; dc.h = h; dc.r = r; dc.color = color;
      calls_[callCount_++] = dc;
    }
    if (verbose_) printf("fillRoundRect(%d,%d,%d,%d,r=%d)\n", x, y, w, h, r);
  }

  void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::FILL_CIRCLE;
      dc.x = x; dc.y = y; dc.r = r; dc.color = color;
      calls_[callCount_++] = dc;
    }
    if (verbose_) printf("fillCircle(%d,%d,r=%d)\n", x, y, r);
  }

  void drawRoundRect(int16_t, int16_t, int16_t, int16_t,
                     int16_t, uint16_t) override { record(DrawCall::OTHER); }

  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::FILL_RECT;
      dc.x = x; dc.y = y; dc.w = w; dc.h = h; dc.color = color;
      calls_[callCount_++] = dc;
    }
    if (verbose_) printf("fillRect(%d,%d,%d,%d)\n", x, y, w, h);
  }

  void drawRect(int16_t, int16_t, int16_t, int16_t,
                uint16_t) override { record(DrawCall::OTHER); }

  void drawCircle(int16_t x, int16_t y, int16_t r,
                  uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::DRAW_CIRCLE;
      dc.x = x; dc.y = y; dc.r = r; dc.color = color;
      calls_[callCount_++] = dc;
    }
    if (verbose_) printf("drawCircle(%d,%d,r=%d)\n", x, y, r);
  }

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::DRAW_LINE;
      dc.x = x0; dc.y = y0; dc.w = x1; dc.h = y1; dc.color = color;
      calls_[callCount_++] = dc;
    }
    if (verbose_) printf("drawLine(%d,%d,%d,%d)\n", x0, y0, x1, y1);
  }

  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                    int16_t x2, int16_t y2, uint16_t color) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::FILL_TRIANGLE;
      dc.x = x0; dc.y = y0; dc.w = x1; dc.h = y1; dc.r = x2; dc.color = color;
      calls_[callCount_++] = dc;
    }
    if (verbose_) printf("fillTriangle(%d,%d,%d,%d,%d,%d)\n", x0, y0, x1, y1, x2, y2);
  }

  void setTextSize(uint8_t) override {}
  void setCursor(int16_t, int16_t) override {}
  void setTextColor(uint16_t) override {}
  void print(const char* t) override {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = DrawCall::TEXT;
      if (t) strncpy(dc.text, t, 31);
      calls_[callCount_++] = dc;
    }
  }
  void println(const char* t) override { print(t); }

private:
  DrawCall calls_[MAX_CALLS];
  int callCount_;
  bool verbose_;

  void record(DrawCall::Type type) {
    if (callCount_ < MAX_CALLS) {
      DrawCall dc = {};
      dc.type = type;
      calls_[callCount_++] = dc;
    }
  }
};

#endif // MOCK_CANVAS_H
