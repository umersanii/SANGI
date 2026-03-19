#include "emotion_draws.h"
#include "canvas.h"
#include <stdio.h>

// ===== LOCAL HELPERS =====

// Draw X-shaped eyes (dead emotion)
static void drawXEyes(ICanvas& c, int thickness) {
  for (int i = 0; i < thickness; i++) {
    c.drawLine(32, 20 + i, 50, 36 + i, COLOR_WHITE);
    c.drawLine(32, 36 + i, 50, 20 + i, COLOR_WHITE);
    c.drawLine(78, 20 + i, 96, 36 + i, COLOR_WHITE);
    c.drawLine(78, 36 + i, 96, 20 + i, COLOR_WHITE);
  }
}

// Draw thick angled furrowed eyebrows (angry emotion)
static void drawAngryBrows(ICanvas& c, int xOff, int thickness,
                           int outerY, int innerY) {
  for (int i = 0; i < thickness; i++) {
    c.drawLine(20 + xOff, outerY + i, 52 + xOff, innerY + i, COLOR_WHITE);
    c.drawLine(76 + xOff, innerY + i, 108 + xOff, outerY + i, COLOR_WHITE);
  }
}

// Draw thick glasses (coding emotion)
static void drawGlasses(ICanvas& c, int leftX, int leftY, int rightX,
                         int rightY) {
  // Left lens — 3 layers for thickness
  c.drawRoundRect(leftX - 14, leftY - 12, 28, 24, 4, COLOR_WHITE);
  c.drawRoundRect(leftX - 13, leftY - 11, 26, 22, 4, COLOR_WHITE);
  c.drawRoundRect(leftX - 12, leftY - 10, 24, 20, 3, COLOR_WHITE);
  // Right lens
  c.drawRoundRect(rightX - 14, rightY - 12, 28, 24, 4, COLOR_WHITE);
  c.drawRoundRect(rightX - 13, rightY - 11, 26, 22, 4, COLOR_WHITE);
  c.drawRoundRect(rightX - 12, rightY - 10, 24, 20, 3, COLOR_WHITE);
  // Bridge
  c.fillRect(52, leftY - 2, 24, 3, COLOR_WHITE);
}

// Draw a music note at position
static void drawMusicNote(ICanvas& c, int x, int y, int size) {
  if (size == 1) {
    c.fillCircle(x, y + 4, 2, COLOR_WHITE);
    c.drawLine(x + 2, y + 4, x + 2, y, COLOR_WHITE);
    c.drawLine(x + 2, y, x + 4, y + 1, COLOR_WHITE);
  } else if (size == 2) {
    c.fillCircle(x, y + 5, 3, COLOR_WHITE);
    c.fillRect(x + 2, y, 2, 6, COLOR_WHITE);
    c.fillRect(x + 2, y, 4, 2, COLOR_WHITE);
  } else {
    c.fillCircle(x, y + 6, 4, COLOR_WHITE);
    c.fillRect(x + 3, y, 3, 7, COLOR_WHITE);
    c.fillRect(x + 3, y, 5, 3, COLOR_WHITE);
  }
}

// Draw a heart shape for love eyes
static void drawHeartEye(ICanvas& c, int cx, int cy, int r) {
  c.fillCircle(cx - 5, cy, r, COLOR_WHITE);
  c.fillCircle(cx + 5, cy, r, COLOR_WHITE);
  int halfSpan = (r > 6) ? r + 5 : r + 4;
  c.fillRect(cx - halfSpan + 3, cy, halfSpan * 2 - 6, r - 1, COLOR_WHITE);
  c.fillTriangle(cx - halfSpan + 3, cy + r - 1, cx, cy + r + 6,
                 cx + halfSpan - 3, cy + r - 1, COLOR_WHITE);
}

// ===== STATIC EMOTIONS =====

void drawIdle(ICanvas& canvas, int frame, const void* ctx) {
  canvas.drawEyes(40, 28, 88, 28, 20);
}

void drawBlink(ICanvas& canvas, int frame, const void* ctx) {
  canvas.drawEyes(40, 28, 88, 28, 4);
}

// ===== SLEEPY =====

void drawSleepy(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    // === CLOSING SEQUENCE (frames 0-8) ===
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 20);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 29, 88, 29, 16);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 2:
      canvas.drawEyes(40, 29, 88, 29, 12);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 3:
      canvas.drawEyes(40, 30, 88, 30, 10);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 4:
      canvas.drawEyes(40, 30, 88, 30, 8);
      canvas.drawCircle(64, 48, 6, COLOR_WHITE);
      break;
    case 5:
      canvas.drawEyes(40, 31, 88, 31, 6);
      canvas.drawCircle(64, 49, 7, COLOR_WHITE);
      break;
    case 6:
      canvas.drawEyes(40, 31, 88, 31, 4);
      canvas.fillCircle(64, 49, 7, COLOR_WHITE);
      break;
    case 7:
      canvas.drawEyes(40, 31, 88, 31, 3);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      break;
    case 8:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(85, 35);
      canvas.print("z");
      break;

    // === STAY CLOSED (frames 9-42) ===
    case 9: case 10: case 11: case 12: case 13:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(88, 32);
      canvas.print("z");
      canvas.setCursor(95, 30);
      canvas.print("z");
      break;
    case 14: case 15: case 16: case 17: case 18:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(92, 28);
      canvas.print("z");
      canvas.setCursor(100, 25);
      canvas.print("z");
      canvas.setCursor(108, 22);
      canvas.print("z");
      break;
    case 19: case 20: case 21: case 22: case 23:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(95, 24);
      canvas.print("z");
      canvas.setCursor(104, 20);
      canvas.print("z");
      canvas.setCursor(112, 16);
      canvas.print("z");
      canvas.setCursor(118, 12);
      canvas.print("z");
      break;
    case 24: case 25: case 26: case 27: case 28:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(98, 20);
      canvas.print("z");
      canvas.setCursor(106, 16);
      canvas.print("z");
      canvas.setCursor(114, 12);
      canvas.print("z");
      canvas.setCursor(120, 8);
      canvas.print("z");
      break;
    case 29: case 30: case 31: case 32: case 33:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(100, 18);
      canvas.print("z");
      canvas.setCursor(108, 14);
      canvas.print("z");
      canvas.setCursor(115, 10);
      canvas.print("z");
      canvas.setCursor(122, 6);
      canvas.print("z");
      break;
    case 34: case 35: case 36: case 37: case 38:
    case 39: case 40: case 41: case 42:
      canvas.drawEyes(40, 31, 88, 31, 2);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(102, 16);
      canvas.print("z");
      canvas.setCursor(110, 12);
      canvas.print("z");
      canvas.setCursor(116, 8);
      canvas.print("z");
      canvas.setCursor(122, 4);
      canvas.print("z");
      canvas.setCursor(126, 2);
      canvas.print("z");
      break;

    // === OPENING SEQUENCE (frames 43-50) ===
    case 43:
      canvas.drawEyes(40, 31, 88, 31, 3);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(110, 8);
      canvas.print("Z");
      break;
    case 44:
      canvas.drawEyes(40, 31, 88, 31, 4);
      canvas.fillCircle(64, 49, 7, COLOR_WHITE);
      break;
    case 45:
      canvas.drawEyes(40, 31, 88, 31, 6);
      canvas.drawCircle(64, 49, 7, COLOR_WHITE);
      break;
    case 46:
      canvas.drawEyes(40, 30, 88, 30, 8);
      canvas.drawCircle(64, 48, 6, COLOR_WHITE);
      break;
    case 47:
      canvas.drawEyes(40, 30, 88, 30, 10);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 48:
      canvas.drawEyes(40, 29, 88, 29, 14);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 49:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 50:
      canvas.drawEyes(40, 28, 88, 28, 20);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
  }
}

// ===== THINKING =====

void drawThinking(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.fillRoundRect(33, 22, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 22, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(118, 12, 2, 10, COLOR_WHITE);
      canvas.fillRect(118, 24, 2, 2, COLOR_WHITE);
      break;
    case 1: case 2:
      canvas.fillRoundRect(31, 20, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(79, 20, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(50, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(115, 10, 3, 14, COLOR_WHITE);
      canvas.fillRect(115, 26, 3, 3, COLOR_WHITE);
      break;
    case 3: case 4: case 5:
      canvas.fillRoundRect(30, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(110, 6, 4, 20, COLOR_WHITE);
      canvas.fillRect(110, 28, 4, 4, COLOR_WHITE);
      break;
    case 6: case 7: case 8:
      canvas.fillRoundRect(30, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(110, 6, 4, 20, COLOR_WHITE);
      canvas.fillRect(110, 28, 4, 4, COLOR_WHITE);
      break;
    case 9: case 10: case 11: case 12: case 13:
    case 14: case 15: case 16: case 17: case 18:
      canvas.fillRoundRect(30, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(110, 6, 4, 20, COLOR_WHITE);
      canvas.fillRect(110, 28, 4, 4, COLOR_WHITE);
      break;
    case 19: case 20: case 21: case 22: case 23:
    case 24: case 25: case 26: case 27: case 28:
      canvas.fillRoundRect(28, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(76, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(110, 6, 4, 20, COLOR_WHITE);
      canvas.fillRect(110, 28, 4, 4, COLOR_WHITE);
      break;
    case 29: case 30: case 31: case 32: case 33:
    case 34: case 35:
      canvas.fillRoundRect(26, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(74, 18, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(110, 6, 4, 20, COLOR_WHITE);
      canvas.fillRect(110, 28, 4, 4, COLOR_WHITE);
      break;
    case 36: case 37: case 38:
      canvas.fillRoundRect(30, 19, 18, 22, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 19, 18, 22, 5, COLOR_WHITE);
      canvas.fillRoundRect(49, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(112, 8, 4, 20, COLOR_WHITE);
      canvas.fillRect(112, 30, 4, 4, COLOR_WHITE);
      break;
    case 39: case 40: case 41: case 42:
      canvas.fillRoundRect(31, 20, 18, 24, 5, COLOR_WHITE);
      canvas.fillRoundRect(79, 20, 18, 24, 5, COLOR_WHITE);
      canvas.fillRoundRect(50, 50, 24, 5, 2, COLOR_WHITE);
      canvas.fillRect(112, 6, 4, 20, COLOR_WHITE);
      canvas.fillRect(112, 28, 4, 4, COLOR_WHITE);
      break;
    case 43:
      canvas.fillRoundRect(32, 21, 18, 21, 5, COLOR_WHITE);
      canvas.fillRoundRect(80, 21, 18, 21, 5, COLOR_WHITE);
      canvas.fillRoundRect(51, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(115, 12, 3, 14, COLOR_WHITE);
      canvas.fillRect(115, 26, 3, 3, COLOR_WHITE);
      break;
    case 44:
      canvas.fillRoundRect(33, 21, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 21, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 5, 2, COLOR_WHITE);
      canvas.fillRect(115, 12, 3, 14, COLOR_WHITE);
      canvas.fillRect(115, 26, 3, 3, COLOR_WHITE);
      break;
    case 45: case 46: case 47: case 48: case 49: case 50:
      canvas.fillRoundRect(33, 22, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 22, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      canvas.fillRect(118, 14, 2, 10, COLOR_WHITE);
      canvas.fillRect(118, 26, 2, 2, COLOR_WHITE);
      break;
  }
}

// ===== EXCITED =====

void drawExcited(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 20);
      canvas.fillCircle(40, 28, 2, COLOR_BLACK);
      canvas.fillCircle(88, 28, 2, COLOR_BLACK);
      canvas.fillRoundRect(52, 50, 24, 8, 4, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 27, 88, 27, 22);
      canvas.fillCircle(40, 27, 2, COLOR_BLACK);
      canvas.fillCircle(88, 27, 2, COLOR_BLACK);
      canvas.fillRoundRect(50, 50, 28, 9, 4, COLOR_WHITE);
      canvas.fillCircle(18, 15, 2, COLOR_WHITE);
      canvas.fillCircle(110, 15, 2, COLOR_WHITE);
      break;
    case 2:
      canvas.drawEyes(40, 26, 88, 26, 24);
      canvas.fillCircle(40, 26, 3, COLOR_BLACK);
      canvas.fillCircle(88, 26, 3, COLOR_BLACK);
      canvas.fillRoundRect(48, 50, 32, 10, 5, COLOR_WHITE);
      canvas.fillCircle(15, 12, 2, COLOR_WHITE);
      canvas.fillCircle(113, 12, 2, COLOR_WHITE);
      canvas.fillCircle(20, 20, 2, COLOR_WHITE);
      canvas.fillCircle(108, 20, 2, COLOR_WHITE);
      break;
    case 3:
      canvas.drawEyes(40, 26, 88, 26, 26);
      canvas.fillCircle(40, 26, 3, COLOR_BLACK);
      canvas.fillCircle(88, 26, 3, COLOR_BLACK);
      canvas.fillRoundRect(45, 50, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(12, 10, 2, COLOR_WHITE);
      canvas.fillCircle(116, 10, 2, COLOR_WHITE);
      canvas.fillCircle(15, 25, 2, COLOR_WHITE);
      canvas.fillCircle(113, 25, 2, COLOR_WHITE);
      canvas.fillCircle(10, 35, 2, COLOR_WHITE);
      canvas.fillCircle(118, 35, 2, COLOR_WHITE);
      break;
    case 4: case 5: case 6: case 7: case 8:
      canvas.drawEyes(40, 26, 88, 26, 26);
      canvas.fillCircle(40, 26, 3, COLOR_BLACK);
      canvas.fillCircle(88, 26, 3, COLOR_BLACK);
      canvas.fillRoundRect(45, 50, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(12, 12, 2, COLOR_WHITE);
      canvas.fillCircle(116, 12, 2, COLOR_WHITE);
      canvas.fillCircle(15, 28, 2, COLOR_WHITE);
      canvas.fillCircle(113, 28, 2, COLOR_WHITE);
      canvas.fillCircle(8, 38, 2, COLOR_WHITE);
      canvas.fillCircle(120, 38, 2, COLOR_WHITE);
      break;

    // === BOUNCING (frames 9-38) ===
    case 9: case 11: case 13: case 15: case 17:
    case 19: case 21: case 23: case 25: case 27:
    case 29: case 31: case 33: case 35: case 37:
      // Bounce UP
      canvas.drawEyes(40, 24, 88, 24, 26);
      canvas.fillCircle(40, 24, 3, COLOR_BLACK);
      canvas.fillCircle(88, 24, 3, COLOR_BLACK);
      canvas.fillRoundRect(45, 48, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(15, 10, 2, COLOR_WHITE);
      canvas.fillCircle(113, 10, 2, COLOR_WHITE);
      canvas.fillCircle(12, 30, 2, COLOR_WHITE);
      canvas.fillCircle(116, 30, 2, COLOR_WHITE);
      break;
    case 10: case 12: case 14: case 16: case 18:
    case 20: case 22: case 24: case 26: case 28:
    case 30: case 32: case 34: case 36: case 38:
      // Bounce DOWN
      canvas.drawEyes(40, 28, 88, 28, 26);
      canvas.fillCircle(40, 28, 3, COLOR_BLACK);
      canvas.fillCircle(88, 28, 3, COLOR_BLACK);
      canvas.fillRoundRect(45, 52, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(18, 15, 2, COLOR_WHITE);
      canvas.fillCircle(110, 15, 2, COLOR_WHITE);
      canvas.fillCircle(10, 38, 2, COLOR_WHITE);
      canvas.fillCircle(118, 38, 2, COLOR_WHITE);
      break;

    // === SETTLING (frames 39-50) ===
    case 39:
      canvas.drawEyes(40, 27, 88, 27, 25);
      canvas.fillCircle(40, 27, 3, COLOR_BLACK);
      canvas.fillCircle(88, 27, 3, COLOR_BLACK);
      canvas.fillRoundRect(46, 50, 36, 10, 5, COLOR_WHITE);
      canvas.fillCircle(15, 20, 2, COLOR_WHITE);
      canvas.fillCircle(113, 20, 2, COLOR_WHITE);
      break;
    case 40:
      canvas.drawEyes(40, 27, 88, 27, 24);
      canvas.fillCircle(40, 27, 3, COLOR_BLACK);
      canvas.fillCircle(88, 27, 3, COLOR_BLACK);
      canvas.fillRoundRect(47, 50, 34, 10, 5, COLOR_WHITE);
      canvas.fillCircle(18, 18, 2, COLOR_WHITE);
      canvas.fillCircle(110, 18, 2, COLOR_WHITE);
      break;
    case 41:
      canvas.drawEyes(40, 27, 88, 27, 23);
      canvas.fillCircle(40, 27, 3, COLOR_BLACK);
      canvas.fillCircle(88, 27, 3, COLOR_BLACK);
      canvas.fillRoundRect(48, 50, 32, 9, 5, COLOR_WHITE);
      break;
    case 42:
      canvas.drawEyes(40, 28, 88, 28, 22);
      canvas.fillCircle(40, 28, 2, COLOR_BLACK);
      canvas.fillCircle(88, 28, 2, COLOR_BLACK);
      canvas.fillRoundRect(50, 50, 28, 9, 4, COLOR_WHITE);
      break;
    case 43: case 44: case 45: case 46: case 47:
    case 48: case 49: case 50:
      canvas.drawEyes(40, 28, 88, 28, 20);
      canvas.fillCircle(40, 28, 2, COLOR_BLACK);
      canvas.fillCircle(88, 28, 2, COLOR_BLACK);
      canvas.fillRoundRect(52, 50, 24, 8, 4, COLOR_WHITE);
      if (frame % 3 == 0) {
        canvas.fillCircle(15, 20, 2, COLOR_WHITE);
        canvas.fillCircle(113, 20, 2, COLOR_WHITE);
      }
      break;
  }
}

// ===== CONFUSED =====

void drawConfused(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawLine(52, 50, 76, 50, COLOR_WHITE);
      break;
    case 1: case 2:
      canvas.fillRoundRect(30, 26, 20, 16, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 26, 20, 18, 5, COLOR_WHITE);
      canvas.drawLine(52, 50, 76, 50, COLOR_WHITE);
      break;
    case 3: case 4:
      canvas.fillRoundRect(30, 24, 20, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 27, 20, 14, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      break;
    case 5: case 6: case 7: case 8:
      canvas.fillRoundRect(30, 24, 20, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 26, 20, 12, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(108, 30);
      canvas.print("?");
      break;
    case 9: case 10: case 11: case 12: case 13:
    case 14: case 15: case 16:
      canvas.fillRoundRect(30, 26, 20, 12, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 24, 20, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(108, 26);
      canvas.print("?");
      canvas.setCursor(115, 30);
      canvas.print("?");
      break;
    case 17: case 18: case 19: case 20: case 21:
    case 22: case 23: case 24: case 25:
      canvas.fillRoundRect(30, 24, 20, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 26, 20, 12, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(106, 22);
      canvas.print("?");
      canvas.setCursor(113, 26);
      canvas.print("?");
      canvas.setTextSize(2);
      canvas.setCursor(118, 18);
      canvas.print("?");
      break;
    case 26: case 27: case 28: case 29: case 30:
    case 31: case 32: case 33: case 34: case 35:
    case 36: case 37: case 38:
      if (frame % 4 < 2) {
        canvas.fillRoundRect(30, 24, 20, 20, 5, COLOR_WHITE);
        canvas.fillRoundRect(78, 27, 20, 14, 5, COLOR_WHITE);
      } else {
        canvas.fillRoundRect(30, 27, 20, 14, 5, COLOR_WHITE);
        canvas.fillRoundRect(78, 24, 20, 20, 5, COLOR_WHITE);
      }
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(105, 18);
      canvas.print("?");
      canvas.setCursor(112, 22);
      canvas.print("?");
      canvas.setTextSize(2);
      canvas.setCursor(118, 14);
      canvas.print("?");
      canvas.setTextSize(1);
      canvas.setCursor(123, 10);
      canvas.print("?");
      break;
    case 39: case 40:
      canvas.fillRoundRect(30, 24, 20, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 26, 20, 12, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      canvas.setTextSize(1);
      canvas.setCursor(110, 20);
      canvas.print("?");
      break;
    case 41: case 42:
      canvas.fillRoundRect(32, 25, 20, 18, 5, COLOR_WHITE);
      canvas.fillRoundRect(78, 26, 20, 16, 5, COLOR_WHITE);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      break;
    case 43: case 44: case 45: case 46: case 47:
    case 48: case 49: case 50:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.fillRoundRect(52, 48, 24, 5, 2, COLOR_WHITE);
      break;
  }
}

// ===== HAPPY =====

void drawHappy(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 28, 88, 28, 16);
      canvas.fillRoundRect(54, 49, 20, 7, 3, COLOR_WHITE);
      break;
    case 2:
      canvas.drawEyes(40, 28, 88, 28, 14);
      canvas.fillRoundRect(52, 48, 24, 8, 4, COLOR_WHITE);
      break;
    case 3:
      canvas.drawEyes(40, 29, 88, 29, 12);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      break;
    case 4:
      canvas.drawEyes(40, 29, 88, 29, 11);
      canvas.fillRoundRect(48, 48, 32, 8, 4, COLOR_WHITE);
      break;
    case 5:
      canvas.drawEyes(40, 29, 88, 29, 10);
      canvas.fillRoundRect(46, 48, 36, 9, 4, COLOR_WHITE);
      canvas.fillCircle(20, 40, 3, COLOR_WHITE);
      canvas.fillCircle(108, 40, 3, COLOR_WHITE);
      break;
    case 6:
      canvas.drawEyes(40, 30, 88, 30, 8);
      canvas.fillRoundRect(45, 47, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(19, 40, 4, COLOR_WHITE);
      canvas.fillCircle(109, 40, 4, COLOR_WHITE);
      break;
    case 7: case 8: case 9: case 10:
      canvas.drawEyes(40, 30, 88, 30, 8);
      canvas.fillRoundRect(45, 47, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(18, 40, 4, COLOR_WHITE);
      canvas.fillCircle(110, 40, 4, COLOR_WHITE);
      if (frame % 2 == 0) {
        canvas.fillCircle(15, 20, 2, COLOR_WHITE);
        canvas.fillCircle(113, 20, 2, COLOR_WHITE);
      }
      break;
    case 11: case 12: case 13: case 14: case 15:
    case 16: case 17: case 18: case 19: case 20:
    case 21: case 22: case 23: case 24: case 25:
    case 26: case 27: case 28: case 29: case 30:
      canvas.drawEyes(40, 30, 88, 30, 8);
      canvas.fillRoundRect(45, 47, 38, 10, 5, COLOR_WHITE);
      canvas.fillCircle(18, 40, 4, COLOR_WHITE);
      canvas.fillCircle(110, 40, 4, COLOR_WHITE);
      if (frame % 4 == 0) {
        canvas.fillCircle(12, 18, 2, COLOR_WHITE);
        canvas.fillCircle(116, 18, 2, COLOR_WHITE);
      } else if (frame % 4 == 2) {
        canvas.fillCircle(15, 25, 2, COLOR_WHITE);
        canvas.fillCircle(113, 25, 2, COLOR_WHITE);
      }
      break;
    case 31: case 32: case 33: case 34: case 35:
    case 36: case 37: case 38: case 39: case 40: {
      canvas.drawEyes(40, 30, 88, 30, 8);
      canvas.fillRoundRect(45, 47, 38, 10, 5, COLOR_WHITE);
      int blushSize = (frame % 6 < 3) ? 4 : 5;
      canvas.fillCircle(18, 40, blushSize, COLOR_WHITE);
      canvas.fillCircle(110, 40, blushSize, COLOR_WHITE);
      break;
    }
    case 41:
      canvas.drawEyes(40, 30, 88, 30, 9);
      canvas.fillRoundRect(46, 47, 36, 9, 5, COLOR_WHITE);
      canvas.fillCircle(19, 40, 4, COLOR_WHITE);
      canvas.fillCircle(109, 40, 4, COLOR_WHITE);
      break;
    case 42:
      canvas.drawEyes(40, 29, 88, 29, 10);
      canvas.fillRoundRect(48, 48, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(20, 40, 3, COLOR_WHITE);
      canvas.fillCircle(108, 40, 3, COLOR_WHITE);
      break;
    case 43:
      canvas.drawEyes(40, 29, 88, 29, 12);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      canvas.fillCircle(20, 40, 3, COLOR_WHITE);
      canvas.fillCircle(108, 40, 3, COLOR_WHITE);
      break;
    case 44:
      canvas.drawEyes(40, 28, 88, 28, 14);
      canvas.fillRoundRect(52, 48, 24, 8, 4, COLOR_WHITE);
      break;
    case 45:
      canvas.drawEyes(40, 28, 88, 28, 16);
      canvas.fillRoundRect(54, 49, 20, 7, 3, COLOR_WHITE);
      break;
    case 46: case 47: case 48: case 49: case 50:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      break;
  }
}

// ===== LOVE =====

void drawLove(ICanvas& canvas, int frame, const void* ctx) {
  // Helper lambdas for heart eyes at different sizes
  auto heartEyes = [&](int r, int rectH) {
    canvas.fillCircle(34, 26, r, COLOR_WHITE);
    canvas.fillCircle(44, 26, r, COLOR_WHITE);
    int span = r + 1;
    canvas.fillRect(39 - span, 26, span * 2, rectH, COLOR_WHITE);
    canvas.fillTriangle(39 - span, 26 + rectH, 39, 26 + rectH + 8,
                        39 + span, 26 + rectH, COLOR_WHITE);

    canvas.fillCircle(82, 26, r, COLOR_WHITE);
    canvas.fillCircle(92, 26, r, COLOR_WHITE);
    canvas.fillRect(87 - span, 26, span * 2, rectH, COLOR_WHITE);
    canvas.fillTriangle(87 - span, 26 + rectH, 87, 26 + rectH + 8,
                        87 + span, 26 + rectH, COLOR_WHITE);
  };

  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 16);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      break;
    case 1: case 2:
      canvas.fillCircle(34, 28, 5, COLOR_WHITE);
      canvas.fillCircle(44, 28, 5, COLOR_WHITE);
      canvas.fillCircle(82, 28, 5, COLOR_WHITE);
      canvas.fillCircle(92, 28, 5, COLOR_WHITE);
      canvas.fillRoundRect(50, 50, 28, 7, 3, COLOR_WHITE);
      break;
    case 3: case 4:
      canvas.fillCircle(34, 27, 6, COLOR_WHITE);
      canvas.fillCircle(44, 27, 6, COLOR_WHITE);
      canvas.fillRect(28, 27, 22, 5, COLOR_WHITE);
      canvas.fillTriangle(28, 32, 39, 38, 50, 32, COLOR_WHITE);
      canvas.fillCircle(82, 27, 6, COLOR_WHITE);
      canvas.fillCircle(92, 27, 6, COLOR_WHITE);
      canvas.fillRect(76, 27, 22, 5, COLOR_WHITE);
      canvas.fillTriangle(76, 32, 87, 38, 98, 32, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      break;
    case 5: case 6: case 7:
      canvas.fillCircle(34, 26, 7, COLOR_WHITE);
      canvas.fillCircle(44, 26, 7, COLOR_WHITE);
      canvas.fillRect(27, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(27, 32, 39, 40, 51, 32, COLOR_WHITE);
      canvas.fillCircle(82, 26, 7, COLOR_WHITE);
      canvas.fillCircle(92, 26, 7, COLOR_WHITE);
      canvas.fillRect(75, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(75, 32, 87, 40, 99, 32, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(15, 42, 3, COLOR_WHITE);
      canvas.fillCircle(113, 42, 3, COLOR_WHITE);
      break;
    case 8: case 9: case 10:
      canvas.fillCircle(34, 26, 7, COLOR_WHITE);
      canvas.fillCircle(44, 26, 7, COLOR_WHITE);
      canvas.fillRect(27, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(27, 32, 39, 40, 51, 32, COLOR_WHITE);
      canvas.fillCircle(82, 26, 7, COLOR_WHITE);
      canvas.fillCircle(92, 26, 7, COLOR_WHITE);
      canvas.fillRect(75, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(75, 32, 87, 40, 99, 32, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(14, 42, 4, COLOR_WHITE);
      canvas.fillCircle(114, 42, 4, COLOR_WHITE);
      canvas.drawCircle(108, 18, 2, COLOR_WHITE);
      canvas.drawCircle(112, 18, 2, COLOR_WHITE);
      break;

    // === PULSING (frames 11-35) ===
    case 11: case 13: case 15: case 17: case 19:
    case 21: case 23: case 25: case 27: case 29:
    case 31: case 33: case 35:
      // Pulse larger
      canvas.fillCircle(34, 26, 8, COLOR_WHITE);
      canvas.fillCircle(44, 26, 8, COLOR_WHITE);
      canvas.fillRect(26, 26, 26, 7, COLOR_WHITE);
      canvas.fillTriangle(26, 33, 39, 42, 52, 33, COLOR_WHITE);
      canvas.fillCircle(82, 26, 8, COLOR_WHITE);
      canvas.fillCircle(92, 26, 8, COLOR_WHITE);
      canvas.fillRect(74, 26, 26, 7, COLOR_WHITE);
      canvas.fillTriangle(74, 33, 87, 42, 100, 33, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(14, 42, 5, COLOR_WHITE);
      canvas.fillCircle(114, 42, 5, COLOR_WHITE);
      canvas.drawCircle(106, 15, 2, COLOR_WHITE);
      canvas.drawCircle(110, 15, 2, COLOR_WHITE);
      canvas.drawCircle(115, 10, 2, COLOR_WHITE);
      canvas.drawCircle(119, 10, 2, COLOR_WHITE);
      break;
    case 12: case 14: case 16: case 18: case 20:
    case 22: case 24: case 26: case 28: case 30:
    case 32: case 34:
      // Pulse smaller
      canvas.fillCircle(34, 26, 6, COLOR_WHITE);
      canvas.fillCircle(44, 26, 6, COLOR_WHITE);
      canvas.fillRect(28, 26, 22, 5, COLOR_WHITE);
      canvas.fillTriangle(28, 31, 39, 38, 50, 31, COLOR_WHITE);
      canvas.fillCircle(82, 26, 6, COLOR_WHITE);
      canvas.fillCircle(92, 26, 6, COLOR_WHITE);
      canvas.fillRect(76, 26, 22, 5, COLOR_WHITE);
      canvas.fillTriangle(76, 31, 87, 38, 98, 31, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(14, 42, 4, COLOR_WHITE);
      canvas.fillCircle(114, 42, 4, COLOR_WHITE);
      canvas.drawCircle(108, 12, 2, COLOR_WHITE);
      canvas.drawCircle(112, 12, 2, COLOR_WHITE);
      canvas.drawCircle(18, 18, 2, COLOR_WHITE);
      canvas.drawCircle(22, 18, 2, COLOR_WHITE);
      break;

    // === MAXIMUM LOVE (frames 36-44) ===
    case 36: case 37: case 38: case 39: case 40:
    case 41: case 42: case 43: case 44:
      canvas.fillCircle(34, 26, 7, COLOR_WHITE);
      canvas.fillCircle(44, 26, 7, COLOR_WHITE);
      canvas.fillRect(27, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(27, 32, 39, 40, 51, 32, COLOR_WHITE);
      canvas.fillCircle(82, 26, 7, COLOR_WHITE);
      canvas.fillCircle(92, 26, 7, COLOR_WHITE);
      canvas.fillRect(75, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(75, 32, 87, 40, 99, 32, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(14, 42, 5, COLOR_WHITE);
      canvas.fillCircle(114, 42, 5, COLOR_WHITE);
      if (frame % 2 == 0) {
        canvas.drawCircle(105, 10, 2, COLOR_WHITE);
        canvas.drawCircle(109, 10, 2, COLOR_WHITE);
        canvas.drawCircle(115, 5, 2, COLOR_WHITE);
        canvas.drawCircle(119, 5, 2, COLOR_WHITE);
      } else {
        canvas.drawCircle(18, 15, 2, COLOR_WHITE);
        canvas.drawCircle(22, 15, 2, COLOR_WHITE);
        canvas.drawCircle(12, 8, 2, COLOR_WHITE);
        canvas.drawCircle(16, 8, 2, COLOR_WHITE);
      }
      break;

    // === CALMING (frames 45-50) ===
    case 45: case 46:
      canvas.fillCircle(34, 26, 7, COLOR_WHITE);
      canvas.fillCircle(44, 26, 7, COLOR_WHITE);
      canvas.fillRect(27, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(27, 32, 39, 40, 51, 32, COLOR_WHITE);
      canvas.fillCircle(82, 26, 7, COLOR_WHITE);
      canvas.fillCircle(92, 26, 7, COLOR_WHITE);
      canvas.fillRect(75, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(75, 32, 87, 40, 99, 32, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(15, 42, 4, COLOR_WHITE);
      canvas.fillCircle(113, 42, 4, COLOR_WHITE);
      canvas.drawCircle(110, 15, 2, COLOR_WHITE);
      canvas.drawCircle(114, 15, 2, COLOR_WHITE);
      break;
    case 47: case 48: case 49: case 50:
      canvas.fillCircle(34, 26, 7, COLOR_WHITE);
      canvas.fillCircle(44, 26, 7, COLOR_WHITE);
      canvas.fillRect(27, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(27, 32, 39, 40, 51, 32, COLOR_WHITE);
      canvas.fillCircle(82, 26, 7, COLOR_WHITE);
      canvas.fillCircle(92, 26, 7, COLOR_WHITE);
      canvas.fillRect(75, 26, 24, 6, COLOR_WHITE);
      canvas.fillTriangle(75, 32, 87, 40, 99, 32, COLOR_WHITE);
      canvas.fillRoundRect(48, 50, 32, 8, 4, COLOR_WHITE);
      canvas.fillCircle(15, 42, 3, COLOR_WHITE);
      canvas.fillCircle(113, 42, 3, COLOR_WHITE);
      break;
  }
}

// ===== ANGRY =====

void drawAngry(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 30, 88, 30, 14);
      canvas.fillRoundRect(52, 52, 24, 4, 2, COLOR_WHITE);
      break;
    case 1: case 2:
      canvas.drawEyes(40, 31, 88, 31, 12);
      drawAngryBrows(canvas, 0, 4, 17, 21);
      canvas.fillRoundRect(52, 52, 24, 5, 2, COLOR_WHITE);
      break;
    case 3: case 4: case 5:
      canvas.drawEyes(40, 32, 88, 32, 11);
      drawAngryBrows(canvas, 0, 5, 16, 22);
      canvas.fillRoundRect(52, 52, 24, 5, 2, COLOR_WHITE);
      break;
    case 6: case 7: case 8: case 9: case 10:
      canvas.drawEyes(40, 32, 88, 32, 10);
      drawAngryBrows(canvas, 0, 6, 15, 23);
      canvas.fillRoundRect(52, 52, 24, 5, 2, COLOR_WHITE);
      break;

    // === SHAKING (frames 11-38) ===
    case 11: case 13: case 15: case 17: case 19:
    case 21: case 23: case 25: case 27: case 29:
    case 31: case 33: case 35: case 37:
      // Shake LEFT
      canvas.drawEyes(37, 32, 85, 32, 10);
      drawAngryBrows(canvas, -3, 6, 15, 23);
      canvas.fillRoundRect(49, 52, 24, 5, 2, COLOR_WHITE);
      break;
    case 12: case 14: case 16: case 18: case 20:
    case 22: case 24: case 26: case 28: case 30:
    case 32: case 34: case 36: case 38:
      // Shake RIGHT
      canvas.drawEyes(43, 32, 91, 32, 10);
      drawAngryBrows(canvas, 3, 6, 15, 23);
      canvas.fillRoundRect(55, 52, 24, 5, 2, COLOR_WHITE);
      break;

    // === MAXIMUM RAGE (frames 39-45) ===
    case 39: case 40: case 41: case 42: case 43:
    case 44: case 45: {
      int xExtreme = (frame % 2 == 0) ? -4 : 4;
      canvas.drawEyes(40 + xExtreme, 32, 88 + xExtreme, 32, 9);
      drawAngryBrows(canvas, xExtreme, 7, 14, 24);
      canvas.fillRoundRect(52 + xExtreme, 52, 24, 5, 2, COLOR_WHITE);
      break;
    }

    // === CALMING (frames 46-50) ===
    case 46: case 47:
      canvas.drawEyes(40, 32, 88, 32, 10);
      drawAngryBrows(canvas, 0, 6, 15, 23);
      canvas.fillRoundRect(52, 52, 24, 5, 2, COLOR_WHITE);
      break;
    case 48: case 49: case 50:
      canvas.drawEyes(40, 32, 88, 32, 11);
      drawAngryBrows(canvas, 0, 5, 16, 22);
      canvas.fillRoundRect(52, 52, 24, 5, 2, COLOR_WHITE);
      break;
  }
}

// ===== SAD =====

void drawSad(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.fillRoundRect(56, 50, 16, 4, 2, COLOR_WHITE);
      break;
    case 1: case 2:
      canvas.drawEyes(40, 29, 88, 29, 17);
      canvas.fillRoundRect(56, 51, 16, 4, 2, COLOR_WHITE);
      break;
    case 3: case 4:
      canvas.drawEyes(40, 30, 88, 30, 16);
      canvas.fillRoundRect(56, 52, 16, 4, 2, COLOR_WHITE);
      break;
    case 5: case 6:
      canvas.drawEyes(40, 31, 88, 31, 15);
      canvas.fillRoundRect(56, 52, 16, 5, 2, COLOR_WHITE);
      break;
    case 7: case 8:
      canvas.drawEyes(40, 32, 88, 32, 14);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 40, 2, COLOR_WHITE);
      break;
    case 9: case 10: case 11: case 12:
      canvas.drawEyes(40, 32, 88, 32, 14);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 40, 1, COLOR_WHITE);
      break;
    case 13: case 14: case 15: case 16:
      canvas.drawEyes(40, 32, 88, 32, 13);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 41, 2, COLOR_WHITE);
      canvas.drawLine(50, 43, 50, 45, COLOR_WHITE);
      break;
    case 17: case 18: case 19: case 20:
      canvas.drawEyes(40, 32, 88, 32, 13);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 42, 2, COLOR_WHITE);
      canvas.drawLine(50, 44, 50, 50, COLOR_WHITE);
      break;
    case 21: case 22: case 23: case 24: case 25:
    case 26: case 27: case 28: case 29: case 30:
      canvas.drawEyes(40, 32, 88, 32, 12);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 42, 2, COLOR_WHITE);
      canvas.drawLine(50, 44, 50, 55, COLOR_WHITE);
      break;
    case 31: case 32: case 33: case 34: case 35:
    case 36: case 37: case 38: case 39: case 40:
      canvas.drawEyes(40, 32, 88, 32, 12);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 42, 2, COLOR_WHITE);
      canvas.drawLine(50, 44, 50, 60, COLOR_WHITE);
      break;
    case 41: case 42:
      canvas.drawEyes(40, 32, 88, 32, 13);
      canvas.fillRoundRect(54, 52, 20, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 42, 1, COLOR_WHITE);
      canvas.drawLine(50, 43, 50, 58, COLOR_WHITE);
      break;
    case 43: case 44:
      canvas.drawEyes(40, 31, 88, 31, 14);
      canvas.fillRoundRect(56, 52, 16, 5, 2, COLOR_WHITE);
      canvas.fillCircle(50, 42, 1, COLOR_WHITE);
      canvas.drawLine(50, 43, 50, 52, COLOR_WHITE);
      break;
    case 45: case 46: case 47: case 48: case 49: case 50:
      canvas.drawEyes(40, 30, 88, 30, 16);
      canvas.fillRoundRect(56, 52, 16, 4, 2, COLOR_WHITE);
      break;
  }
}

// ===== SURPRISED =====

void drawSurprised(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.fillRoundRect(33, 24, 18, 18, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 24, 18, 18, 5, COLOR_WHITE);
      canvas.fillCircle(42, 33, 3, COLOR_BLACK);
      canvas.fillCircle(90, 33, 3, COLOR_BLACK);
      canvas.drawCircle(64, 50, 4, COLOR_WHITE);
      break;
    case 1:
      canvas.fillRoundRect(33, 23, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 23, 18, 20, 5, COLOR_WHITE);
      canvas.fillCircle(42, 33, 3, COLOR_BLACK);
      canvas.fillCircle(90, 33, 3, COLOR_BLACK);
      canvas.drawCircle(64, 50, 5, COLOR_WHITE);
      break;
    case 2:
      canvas.fillRoundRect(33, 21, 18, 22, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 21, 18, 22, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 50, 5, COLOR_WHITE);
      break;
    case 3:
      canvas.fillRoundRect(33, 20, 18, 24, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 20, 18, 24, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 50, 6, COLOR_WHITE);
      break;
    case 4:
      canvas.fillRoundRect(33, 19, 18, 26, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 19, 18, 26, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 51, 7, COLOR_WHITE);
      break;
    case 5:
      canvas.fillRoundRect(33, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 6: case 7: case 8: case 9: case 10:
      canvas.fillRoundRect(33, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 11:
      canvas.fillRoundRect(33, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 2, COLOR_BLACK);
      canvas.fillCircle(90, 32, 2, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 12:
      canvas.fillRoundRect(33, 31, 18, 4, 2, COLOR_WHITE);
      canvas.fillRoundRect(81, 31, 18, 4, 2, COLOR_WHITE);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 13:
      canvas.fillRoundRect(33, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 2, COLOR_BLACK);
      canvas.fillCircle(90, 32, 2, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 14: case 15: case 16:
      canvas.fillRoundRect(33, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 17:
      canvas.fillRoundRect(33, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 2, COLOR_BLACK);
      canvas.fillCircle(90, 32, 2, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 18:
      canvas.fillRoundRect(33, 31, 18, 4, 2, COLOR_WHITE);
      canvas.fillRoundRect(81, 31, 18, 4, 2, COLOR_WHITE);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 19:
      canvas.fillRoundRect(33, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 28, 18, 10, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 2, COLOR_BLACK);
      canvas.fillCircle(90, 32, 2, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 20: case 21: case 22: case 23: case 24:
    case 25: case 26: case 27: case 28: case 29:
    case 30: case 31: case 32: case 33: case 34: case 35:
      canvas.fillRoundRect(33, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 18, 18, 28, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 52, 8, COLOR_WHITE);
      break;
    case 36: case 37:
      canvas.fillRoundRect(33, 19, 18, 26, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 19, 18, 26, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 51, 7, COLOR_WHITE);
      break;
    case 38: case 39:
      canvas.fillRoundRect(33, 20, 18, 24, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 20, 18, 24, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 50, 6, COLOR_WHITE);
      break;
    case 40: case 41:
      canvas.fillRoundRect(33, 21, 18, 22, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 21, 18, 22, 5, COLOR_WHITE);
      canvas.fillCircle(42, 32, 3, COLOR_BLACK);
      canvas.fillCircle(90, 32, 3, COLOR_BLACK);
      canvas.fillCircle(64, 50, 5, COLOR_WHITE);
      break;
    case 42: case 43: case 44: case 45: case 46:
    case 47: case 48: case 49: case 50:
      canvas.fillRoundRect(33, 23, 18, 20, 5, COLOR_WHITE);
      canvas.fillRoundRect(81, 23, 18, 20, 5, COLOR_WHITE);
      canvas.fillCircle(42, 33, 3, COLOR_BLACK);
      canvas.fillCircle(90, 33, 3, COLOR_BLACK);
      canvas.drawCircle(64, 50, 5, COLOR_WHITE);
      break;
  }
}

// ===== MUSIC =====

void drawMusic(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 29, 88, 29, 14);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      break;
    case 2:
      canvas.drawEyes(40, 30, 88, 30, 10);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      break;
    case 3:
      canvas.drawEyes(40, 31, 88, 31, 6);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      break;
    case 4: case 5:
      canvas.drawEyes(40, 31, 88, 31, 3);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 110, 50, 1);
      break;
    case 6: case 7:
      canvas.drawEyes(39, 31, 87, 31, 3);
      canvas.fillRoundRect(51, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 115, 50, 1);
      break;
    case 8: case 9:
      canvas.drawEyes(38, 32, 86, 32, 3);
      canvas.fillRoundRect(50, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 116, 45, 1);
      break;
    case 10: case 11:
      canvas.drawEyes(37, 33, 85, 33, 3);
      canvas.fillRoundRect(49, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 118, 38, 1);
      break;
    case 12: case 13:
      canvas.drawEyes(36, 33, 84, 33, 3);
      canvas.fillRoundRect(48, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 120, 30, 2);
      drawMusicNote(canvas, 12, 52, 1);
      break;
    case 14: case 15:
      canvas.drawEyes(35, 32, 83, 32, 3);
      canvas.fillRoundRect(47, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 122, 22, 1);
      drawMusicNote(canvas, 10, 48, 1);
      break;
    case 16: case 17:
      canvas.drawEyes(34, 31, 82, 31, 3);
      canvas.fillRoundRect(46, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 124, 14, 1);
      drawMusicNote(canvas, 8, 42, 1);
      break;
    case 18: case 19:
      canvas.drawEyes(35, 31, 83, 31, 3);
      canvas.fillRoundRect(47, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 126, 8, 1);
      drawMusicNote(canvas, 6, 36, 1);
      break;
    case 20: case 21:
      canvas.drawEyes(36, 32, 84, 32, 3);
      canvas.fillRoundRect(48, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 6, 28, 2);
      drawMusicNote(canvas, 112, 52, 1);
      break;
    case 22: case 23:
      canvas.drawEyes(38, 33, 86, 33, 3);
      canvas.fillRoundRect(50, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 4, 20, 1);
      drawMusicNote(canvas, 116, 48, 1);
      break;
    case 24: case 25:
      canvas.drawEyes(40, 33, 88, 33, 3);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 6, 12, 1);
      drawMusicNote(canvas, 118, 42, 1);
      break;
    case 26: case 27:
      canvas.drawEyes(42, 32, 90, 32, 3);
      canvas.fillRoundRect(54, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 4, 6, 1);
      drawMusicNote(canvas, 120, 36, 2);
      drawMusicNote(canvas, 10, 52, 1);
      break;
    case 28: case 29:
      canvas.drawEyes(44, 31, 92, 31, 3);
      canvas.fillRoundRect(56, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 122, 30, 1);
      break;
    case 30: case 31:
      canvas.drawEyes(43, 31, 91, 31, 3);
      canvas.fillRoundRect(55, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 124, 24, 1);
      drawMusicNote(canvas, 10, 52, 1);
      break;
    case 32: case 33:
      canvas.drawEyes(42, 32, 90, 32, 3);
      canvas.fillRoundRect(54, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 126, 16, 1);
      drawMusicNote(canvas, 8, 46, 1);
      break;
    case 34: case 35:
      canvas.drawEyes(40, 33, 88, 33, 3);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 124, 10, 1);
      drawMusicNote(canvas, 6, 38, 2);
      break;
    case 36: case 37:
      canvas.drawEyes(38, 33, 86, 33, 3);
      canvas.fillRoundRect(50, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 122, 6, 1);
      drawMusicNote(canvas, 6, 30, 1);
      break;
    case 38: case 39:
      canvas.drawEyes(36, 32, 84, 32, 3);
      canvas.fillRoundRect(48, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 6, 20, 1);
      drawMusicNote(canvas, 120, 20, 1);
      break;
    case 40: case 41:
      canvas.drawEyes(35, 31, 83, 31, 3);
      canvas.fillRoundRect(47, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 6, 12, 1);
      drawMusicNote(canvas, 122, 12, 1);
      break;
    case 42: case 43:
      canvas.drawEyes(36, 31, 84, 31, 3);
      canvas.fillRoundRect(48, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 6, 8, 1);
      drawMusicNote(canvas, 122, 8, 1);
      break;
    case 44: case 45:
      canvas.drawEyes(37, 32, 85, 32, 3);
      canvas.fillRoundRect(49, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 8, 14, 1);
      drawMusicNote(canvas, 120, 14, 1);
      break;
    case 46: case 47:
      canvas.drawEyes(38, 33, 86, 33, 3);
      canvas.fillRoundRect(50, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 8, 20, 1);
      drawMusicNote(canvas, 118, 20, 1);
      break;
    case 48: case 49:
      canvas.drawEyes(39, 32, 87, 32, 3);
      canvas.fillRoundRect(51, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 10, 26, 1);
      drawMusicNote(canvas, 116, 26, 1);
      break;
    case 50:
      canvas.drawEyes(40, 31, 88, 31, 3);
      canvas.fillRoundRect(52, 50, 24, 6, 3, COLOR_WHITE);
      drawMusicNote(canvas, 12, 30, 1);
      drawMusicNote(canvas, 114, 30, 1);
      break;
  }
}

// ===== DEAD =====

void drawDead(ICanvas& canvas, int frame, const void* ctx) {
  switch (frame) {
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 50, 4, COLOR_WHITE);
      break;
    case 1: case 2:
      canvas.drawEyes(40, 29, 88, 29, 16);
      canvas.drawCircle(64, 50, 4, COLOR_WHITE);
      break;
    case 3: case 4:
      canvas.drawEyes(40, 30, 88, 30, 12);
      canvas.drawCircle(64, 50, 4, COLOR_WHITE);
      break;
    case 5: case 6:
      canvas.drawEyes(40, 31, 88, 31, 8);
      canvas.drawCircle(64, 51, 5, COLOR_WHITE);
      break;
    case 7: case 8:
      canvas.fillRoundRect(38, 30, 18, 4, 2, COLOR_WHITE);
      canvas.fillRoundRect(86, 30, 18, 4, 2, COLOR_WHITE);
      canvas.drawCircle(64, 51, 5, COLOR_WHITE);
      break;
    case 9: case 10:
      drawXEyes(canvas, 5);
      canvas.drawCircle(64, 50, 5, COLOR_WHITE);
      break;
    case 11: case 12: case 13: case 14: case 15:
      drawXEyes(canvas, 6);
      canvas.drawCircle(64, 50, 6, COLOR_WHITE);
      break;
    case 16: case 17: case 18:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      canvas.fillRect(62, 54, 4, 2, COLOR_WHITE);
      break;
    case 19: case 20: case 21:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      canvas.fillRoundRect(61, 54, 6, 4, 2, COLOR_WHITE);
      break;
    case 22: case 23: case 24: case 25:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      canvas.fillRoundRect(60, 54, 8, 6, 3, COLOR_WHITE);
      break;
    case 26: case 27: case 28: case 29: case 30:
    case 31: case 32: case 33: case 34: case 35:
    case 36: case 37: case 38: case 39: case 40:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      canvas.fillRoundRect(60, 54, 8, 6, 3, COLOR_WHITE);
      if (frame % 8 < 4) {
        canvas.drawCircle(20, 20, 3, COLOR_WHITE);
        canvas.drawCircle(108, 20, 3, COLOR_WHITE);
      } else {
        canvas.drawCircle(18, 24, 3, COLOR_WHITE);
        canvas.drawCircle(110, 24, 3, COLOR_WHITE);
      }
      break;
    case 41: case 42:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      canvas.fillRoundRect(61, 54, 6, 4, 2, COLOR_WHITE);
      break;
    case 43: case 44:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      canvas.fillRect(62, 54, 4, 2, COLOR_WHITE);
      break;
    case 45: case 46: case 47: case 48: case 49: case 50:
      drawXEyes(canvas, 6);
      canvas.fillRoundRect(56, 50, 16, 6, 3, COLOR_WHITE);
      break;
  }
}

// ===== NOTIFICATION =====
// 86 frames @ 50ms = 4.3s total

void drawNotification(ICanvas& canvas, int frame, const void* ctx) {
  const NotificationContext* nctx =
      static_cast<const NotificationContext*>(ctx);
  const char* title = nctx ? nctx->title : "";
  const char* message = nctx ? nctx->message : "";

  switch (frame) {
    // === SURPRISE REACTION (0-5) ===
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 27, 88, 27, 22);
      canvas.fillCircle(40, 27, 2, COLOR_BLACK);
      canvas.fillCircle(88, 27, 2, COLOR_BLACK);
      canvas.drawCircle(64, 48, 6, COLOR_WHITE);
      break;
    case 2: case 3:
      canvas.drawEyes(40, 26, 88, 26, 26);
      canvas.fillCircle(40, 26, 3, COLOR_BLACK);
      canvas.fillCircle(88, 26, 3, COLOR_BLACK);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      break;
    case 4: case 5:
      canvas.drawEyes(40, 28, 88, 28, 12);
      canvas.drawLine(52, 50, 76, 50, COLOR_WHITE);
      break;

    // === RUN AWAY (6-12) ===
    case 6:
      canvas.drawEyes(50, 28, 98, 28, 14);
      canvas.drawLine(62, 50, 86, 50, COLOR_WHITE);
      break;
    case 7:
      canvas.drawEyes(60, 28, 108, 28, 14);
      canvas.drawLine(72, 50, 96, 50, COLOR_WHITE);
      break;
    case 8:
      canvas.drawEyes(75, 28, 123, 28, 14);
      canvas.drawLine(87, 50, 111, 50, COLOR_WHITE);
      break;
    case 9: case 10: case 11: case 12:
      break;

    // === BOARD SLIDES IN (13-18) ===
    case 13:
      canvas.fillRect(0, 8, 10, 50, COLOR_WHITE);
      break;
    case 14:
      canvas.drawRect(0, 8, 40, 50, COLOR_WHITE);
      canvas.drawRect(1, 9, 38, 48, COLOR_WHITE);
      break;
    case 15:
      canvas.drawRect(0, 8, 80, 50, COLOR_WHITE);
      canvas.drawRect(1, 9, 78, 48, COLOR_WHITE);
      break;
    case 16:
      canvas.drawRect(2, 8, 120, 50, COLOR_WHITE);
      canvas.drawRect(3, 9, 118, 48, COLOR_WHITE);
      break;
    case 17: case 18:
      canvas.drawRect(4, 8, 120, 52, COLOR_WHITE);
      canvas.drawRect(5, 9, 118, 50, COLOR_WHITE);
      break;

    // === DISPLAY NOTIFICATION (19-70) ===
    case 19: case 20: case 21: case 22: case 23:
    case 24: case 25: case 26: case 27: case 28:
    case 29: case 30: case 31: case 32: case 33:
    case 34: case 35: case 36: case 37: case 38:
    case 39: case 40: case 41: case 42: case 43:
    case 44: case 45: case 46: case 47: case 48:
    case 49: case 50: case 51: case 52: case 53:
    case 54: case 55: case 56: case 57: case 58:
    case 59: case 60: case 61: case 62: case 63:
    case 64: case 65: case 66: case 67: case 68:
    case 69: case 70:
      canvas.drawRect(4, 8, 120, 52, COLOR_WHITE);
      canvas.drawRect(5, 9, 118, 50, COLOR_WHITE);
      canvas.setTextSize(2);
      canvas.setTextColor(COLOR_WHITE);
      if (title && title[0] != '\0') {
        canvas.setCursor(10, 15);
        canvas.print(title);
      }
      canvas.setTextSize(1);
      if (message && message[0] != '\0') {
        canvas.setCursor(10, 38);
        canvas.print(message);
      }
      break;

    // === BOARD SLIDES OUT (71-76) ===
    case 71:
      canvas.drawRect(8, 8, 116, 52, COLOR_WHITE);
      canvas.drawRect(9, 9, 114, 50, COLOR_WHITE);
      break;
    case 72:
      canvas.drawRect(20, 8, 100, 50, COLOR_WHITE);
      canvas.drawRect(21, 9, 98, 48, COLOR_WHITE);
      break;
    case 73:
      canvas.drawRect(50, 8, 70, 50, COLOR_WHITE);
      canvas.drawRect(51, 9, 68, 48, COLOR_WHITE);
      break;
    case 74:
      canvas.drawRect(90, 8, 34, 50, COLOR_WHITE);
      canvas.drawRect(91, 9, 32, 48, COLOR_WHITE);
      break;
    case 75: case 76:
      break;

    // === SANGI RETURNS (77-85) ===
    case 77:
      canvas.drawEyes(10, 28, 58, 28, 14);
      canvas.drawCircle(34, 48, 4, COLOR_WHITE);
      break;
    case 78:
      canvas.drawEyes(20, 28, 68, 28, 16);
      canvas.drawCircle(44, 48, 4, COLOR_WHITE);
      break;
    case 79: case 80:
      canvas.drawEyes(30, 28, 78, 28, 17);
      canvas.drawCircle(54, 48, 5, COLOR_WHITE);
      break;
    case 81: case 82: case 83: case 84: case 85:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
  }
}

// ===== CODING =====
// 25-frame face loop with falling binary overlay
// Note: binary column state is maintained via static vars

void drawCoding(ICanvas& canvas, int frame, const void* ctx) {
  int leftEyeX = 40, leftEyeY = 28;
  int rightEyeX = 88, rightEyeY = 28;
  int eyeHeight = 14;

  int faceFrame = frame % 25;
  switch (faceFrame) {
    case 0: case 1: case 2: case 3: case 4:
    case 5: case 6: case 7: case 8: case 9: case 10:
      canvas.drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeHeight);
      drawGlasses(canvas, leftEyeX, leftEyeY, rightEyeX, rightEyeY);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      break;
    case 11:
      canvas.drawEyes(leftEyeX, leftEyeY + 5, rightEyeX, rightEyeY + 5, 3);
      drawGlasses(canvas, leftEyeX, leftEyeY, rightEyeX, rightEyeY);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      break;
    case 12: case 13: case 14: case 15: case 16:
      canvas.drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeHeight);
      drawGlasses(canvas, leftEyeX, leftEyeY, rightEyeX, rightEyeY);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      break;
    case 17:
      canvas.drawEyes(leftEyeX, leftEyeY + 5, rightEyeX, rightEyeY + 5, 3);
      drawGlasses(canvas, leftEyeX, leftEyeY, rightEyeX, rightEyeY);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      break;
    default:
      canvas.drawEyes(leftEyeX, leftEyeY, rightEyeX, rightEyeY, eyeHeight);
      drawGlasses(canvas, leftEyeX, leftEyeY, rightEyeX, rightEyeY);
      canvas.fillRoundRect(50, 48, 28, 8, 4, COLOR_WHITE);
      break;
  }

  // Falling binary digits
  static int binaryY[6] = {0, 10, 20, 5, 15, 25};
  int colX[6] = {5, 25, 45, 83, 103, 118};

  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);

  for (int col = 0; col < 6; col++) {
    for (int trail = 0; trail < 3; trail++) {
      int y = (binaryY[col] + trail * 15) % 70;
      if (y < 64) {
        canvas.setCursor(colX[col], y);
        if ((col + trail + frame) % 2 == 0) {
          canvas.print("0");
        } else {
          canvas.print("1");
        }
      }
    }
    binaryY[col] = (binaryY[col] + 2) % 70;
  }
}

// ===== GITHUB STATS =====
// 131 frames @ 80ms

void drawGitHubStats(ICanvas& canvas, int frame, const void* ctx) {
  // ctx is not used — stats are fetched from networkManager inside this
  // function. We keep the ctx param for interface compatibility.
  // The actual stats data access will remain via extern for now since
  // it needs NetworkManager.

  switch (frame) {
    // === SURPRISE (0-5) ===
    case 0:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
    case 1:
      canvas.drawEyes(40, 27, 88, 27, 22);
      canvas.fillCircle(40, 27, 2, COLOR_BLACK);
      canvas.fillCircle(88, 27, 2, COLOR_BLACK);
      canvas.drawCircle(64, 48, 6, COLOR_WHITE);
      break;
    case 2: case 3:
      canvas.drawEyes(40, 26, 88, 26, 26);
      canvas.fillCircle(40, 26, 3, COLOR_BLACK);
      canvas.fillCircle(88, 26, 3, COLOR_BLACK);
      canvas.fillCircle(64, 50, 8, COLOR_WHITE);
      break;
    case 4: case 5:
      canvas.drawEyes(40, 28, 88, 28, 12);
      canvas.drawLine(52, 50, 76, 50, COLOR_WHITE);
      break;

    // === RUN OFF (6-12) ===
    case 6:
      canvas.drawEyes(50, 28, 98, 28, 14);
      canvas.drawLine(62, 50, 86, 50, COLOR_WHITE);
      break;
    case 7:
      canvas.drawEyes(60, 28, 108, 28, 14);
      canvas.drawLine(72, 50, 96, 50, COLOR_WHITE);
      break;
    case 8:
      canvas.drawEyes(75, 28, 123, 28, 14);
      canvas.drawLine(87, 50, 111, 50, COLOR_WHITE);
      break;
    case 9: case 10: case 11: case 12:
      break;

    // === TEXT SCROLL (13-27) ===
    case 13: case 14: case 15: case 16: case 17:
    case 18: case 19: case 20: case 21: case 22:
    case 23: case 24: case 25: case 26: case 27: {
      int textOffset = 128 - ((frame - 13) * 14);
      canvas.setTextSize(2);
      canvas.setTextColor(COLOR_WHITE);
      canvas.setCursor(textOffset, 18);
      canvas.print("GitHub");
      canvas.setCursor(textOffset + 12, 38);
      canvas.print("Stats");
      break;
    }

    // === BLANK (28-33) ===
    case 28: case 29: case 30: case 31: case 32: case 33:
      break;

    // === STATS DISPLAY (34-110) ===
    // This section needs access to GitHubStatsData via extern.
    // The actual rendering will be done by the tick() caller or
    // we access it via extern like the original code.
    case 34: case 35: case 36: case 37: case 38: case 39: case 40:
    case 41: case 42: case 43: case 44: case 45: case 46: case 47:
    case 48: case 49: case 50: case 51: case 52: case 53: case 54:
    case 55: case 56: case 57: case 58: case 59: case 60: case 61:
    case 62: case 63: case 64: case 65: case 66: case 67: case 68:
    case 69: case 70: case 71: case 72: case 73: case 74: case 75:
    case 76: case 77: case 78: case 79: case 80: case 81: case 82:
    case 83: case 84: case 85: case 86: case 87: case 88: case 89:
    case 90: case 91: case 92: case 93: case 94: case 95: case 96:
    case 97: case 98: case 99: case 100: case 101: case 102: case 103:
    case 104: case 105: case 106: case 107: case 108: case 109: case 110:
    {
      // Stats data is passed via context from main.cpp
      const GitHubStatsContext* sctx =
          static_cast<const GitHubStatsContext*>(ctx);

      if (!sctx || !sctx->hasData) {
        canvas.setTextSize(2);
        canvas.setTextColor(COLOR_WHITE);
        canvas.setCursor(10, 20);
        canvas.print("No Data");
      } else {
        int statIndex = (frame - 34) / 10;
        canvas.setTextSize(1);
        canvas.setTextColor(COLOR_WHITE);

        // We need to print int values, so we use a buffer
        char buf[32];

        switch (statIndex) {
          case 0:
            canvas.setCursor(10, 10);
            canvas.print("Repositories");
            canvas.setTextSize(3);
            canvas.setCursor(30, 30);
            snprintf(buf, sizeof(buf), "%d", sctx->repos);
            canvas.print(buf);
            break;
          case 1:
            canvas.setCursor(20, 10);
            canvas.print("Followers");
            canvas.setTextSize(3);
            canvas.setCursor(30, 30);
            snprintf(buf, sizeof(buf), "%d", sctx->followers);
            canvas.print(buf);
            break;
          case 2:
            canvas.setCursor(5, 10);
            canvas.print("Contributions");
            canvas.setTextSize(2);
            canvas.setCursor(20, 35);
            snprintf(buf, sizeof(buf), "%d", sctx->contributions);
            canvas.print(buf);
            break;
          case 3:
            canvas.setCursor(25, 10);
            canvas.print("Commits");
            canvas.setTextSize(3);
            canvas.setCursor(30, 30);
            snprintf(buf, sizeof(buf), "%d", sctx->commits);
            canvas.print(buf);
            break;
          case 4:
            canvas.setCursor(5, 10);
            canvas.print("Pull Requests");
            canvas.setTextSize(3);
            canvas.setCursor(30, 30);
            snprintf(buf, sizeof(buf), "%d", sctx->prs);
            canvas.print(buf);
            break;
          case 5:
            canvas.setCursor(30, 10);
            canvas.print("Issues");
            canvas.setTextSize(3);
            canvas.setCursor(30, 30);
            snprintf(buf, sizeof(buf), "%d", sctx->issues);
            canvas.print(buf);
            break;
          case 6:
            canvas.setCursor(15, 10);
            canvas.print("Stars Earned");
            canvas.setTextSize(3);
            canvas.setCursor(30, 30);
            snprintf(buf, sizeof(buf), "%d", sctx->stars);
            canvas.print(buf);
            break;
          case 7:
            canvas.setTextSize(1);
            canvas.setCursor(5, 5);
            canvas.print(sctx->username);
            canvas.setCursor(5, 18);
            snprintf(buf, sizeof(buf), "Repos: %d", sctx->repos);
            canvas.print(buf);
            canvas.setCursor(5, 30);
            snprintf(buf, sizeof(buf), "Followers: %d", sctx->followers);
            canvas.print(buf);
            canvas.setCursor(5, 42);
            snprintf(buf, sizeof(buf), "Commits: %d", sctx->commits);
            canvas.print(buf);
            canvas.setCursor(5, 54);
            snprintf(buf, sizeof(buf), "Stars: %d", sctx->stars);
            canvas.print(buf);
            break;
        }
      }
      break;
    }

    // === FADE OUT (111-115) ===
    case 111: case 112: case 113: case 114: case 115:
      break;

    // === RETURN (116-124) ===
    case 116:
      canvas.drawEyes(10, 28, 58, 28, 14);
      canvas.drawCircle(34, 48, 4, COLOR_WHITE);
      break;
    case 117:
      canvas.drawEyes(20, 28, 68, 28, 16);
      canvas.drawCircle(44, 48, 4, COLOR_WHITE);
      break;
    case 118: case 119:
      canvas.drawEyes(30, 28, 78, 28, 17);
      canvas.drawCircle(54, 48, 5, COLOR_WHITE);
      break;
    case 120: case 121: case 122: case 123: case 124:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;

    // === HOLD IDLE (125-130) ===
    case 125: case 126: case 127: case 128: case 129: case 130:
    default:
      canvas.drawEyes(40, 28, 88, 28, 18);
      canvas.drawCircle(64, 48, 5, COLOR_WHITE);
      break;
  }
}
