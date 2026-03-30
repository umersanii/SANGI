#include "emotion_draws.h"
#include "canvas.h"
#include <stdio.h>

// ===== EASING HELPER =====

// Quadratic ease-in-out: interpolates between start and end over totalFrames.
// frame=0 → start, frame=totalFrames → end. Non-linear for organic motion.
static inline int ease(int start, int end, int frame, int totalFrames) {
  if (totalFrames <= 0) return end;
  float t = (float)frame / (float)totalFrames;
  if (t > 1.0f) t = 1.0f;
  t = (t < 0.5f) ? 2 * t * t : 1 - (-2 * t + 2) * (-2 * t + 2) / 2;
  return start + (int)((end - start) * t);
}

// ===== LOCAL HELPERS =====

// Draw X-shaped eyes (dead emotion) — centered on new grammar positions
static void drawXEyes(ICanvas& c, int thickness) {
  for (int i = 0; i < thickness; i++) {
    // Left X: centered around x=38, eye region y=20-36
    c.drawLine(28, 20 + i, 48, 36 + i, COLOR_WHITE);
    c.drawLine(28, 36 + i, 48, 20 + i, COLOR_WHITE);
    // Right X: centered around x=90, eye region y=20-36
    c.drawLine(80, 20 + i, 100, 36 + i, COLOR_WHITE);
    c.drawLine(80, 36 + i, 100, 20 + i, COLOR_WHITE);
  }
}

// Draw heart eye at center (cx, cy) with given radius
static void drawHeartEye(ICanvas& c, int cx, int cy, int r) {
  // Two circles side by side form the top of the heart
  c.fillCircle(cx - (r / 2 + 1), cy, r, COLOR_WHITE);
  c.fillCircle(cx + (r / 2 + 1), cy, r, COLOR_WHITE);
  // Rectangle fills the gap between circles
  int span = r + (r / 2 + 2);
  c.fillRect(cx - span / 2, cy, span, r, COLOR_WHITE);
  // Triangle forms the point of the heart
  c.fillTriangle(cx - span / 2, cy + r, cx, cy + r + r,
                 cx + span / 2, cy + r, COLOR_WHITE);
}

// ===== 2.2 BLINK — transition mortar =====

void drawBlink(ICanvas& canvas, int frame, const void* ctx) {
  // Eyes at Y=30 (2px lower than neutral), height=4 (nearly shut). No mouth.
  canvas.drawEyes(38, 30, 90, 30, 4);
}

// ===== 2.1 IDLE — restful ambient life =====
// 60 frames @ 50ms = 3.0s loop. Subtle breathing bob with one asymmetric beat.

void drawIdle(ICanvas& canvas, int frame, const void* ctx) {
  // Breathing cycle: eyes drift Y=28→27 on inhale, back on exhale.
  // Two full breaths in 60 frames. Asymmetric beat at F50-55.
  int eyeY;
  if (frame < 15) {
    eyeY = ease(28, 27, frame, 14);
  } else if (frame < 30) {
    eyeY = ease(27, 28, frame - 15, 14);
  } else if (frame < 45) {
    eyeY = ease(28, 27, frame - 30, 14);
  } else {
    eyeY = ease(27, 28, frame - 45, 14);
  }

  int mouthW = (frame >= 7 && frame < 22) ? 12 : 14;
  int mouthH = (frame >= 7 && frame < 22) ? 4 : 5;

  // Asymmetric beat: right eye lifts 1px extra on frames 50-55
  int rightEyeY = eyeY;
  if (frame >= 50 && frame <= 55) {
    rightEyeY = eyeY - 1;
  }

  canvas.drawEyes(38, eyeY, 90, rightEyeY, 22);
  canvas.drawMouth(57, 52, mouthW, mouthH);
}

// ===== 2.3 HAPPY — warm contentment =====
// 50 frames @ 35ms = 1.75s loop.
// Phases: squish (F0-7), hold smile (F8-25), small bounce (F26-33), relax (F34-39), recover (F40-49)

void drawHappy(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 8) {
    // Squish: eyes close from H=20 to H=8, mouth widens
    int eyeH = ease(20, 8, frame, 7);
    int eyeY = ease(28, 31, frame, 7);
    int mouthX = ease(57, 47, frame, 7);
    int mouthW = ease(16, 34, frame, 7);
    int mouthH = ease(5, 9, frame, 7);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.fillRoundRect(mouthX, 49, mouthW, mouthH, 5, COLOR_WHITE);
    if (frame >= 5) {
      canvas.drawBlush(18, 42, 110, 42, frame - 3);
    }
  } else if (frame < 26) {
    // Hold smile: H=8 arcs, blush pulses, sparkles flicker
    canvas.drawEyes(38, 31, 90, 31, 8);
    canvas.fillRoundRect(47, 49, 34, 9, 5, COLOR_WHITE);
    int blushR = ((frame % 6) < 3) ? 4 : 5;
    canvas.drawBlush(18, 42, 110, 42, blushR);
    // Sparkles on frames 10, 14, 18, 22
    if (frame == 10 || frame == 14 || frame == 18 || frame == 22) {
      canvas.drawSparkles(14, 16, 114, 16, 2);
    }
  } else if (frame < 34) {
    // Bounce: face shifts up 2px then back
    int off = (frame < 30) ? -2 : 0;
    canvas.drawEyes(38, 31 + off, 90, 31 + off, 8);
    canvas.fillRoundRect(47, 49 + off, 34, 9, 5, COLOR_WHITE);
    canvas.drawBlush(18, 42 + off, 110, 42 + off, 4);
  } else if (frame < 40) {
    // Relax: eyes partially reopen H=8→16, mouth shrinks
    int eyeH = ease(8, 16, frame - 34, 5);
    int eyeY = ease(31, 29, frame - 34, 5);
    int mouthW = ease(34, 18, frame - 34, 5);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.fillRoundRect(57 - mouthW / 2 + 7, 51, mouthW, 6, 3, COLOR_WHITE);
    canvas.drawBlush(18, 42, 110, 42, 3);
  } else {
    // Recover: ease back to near-neutral (H=22, Y=28) for invisible loop seam
    int eyeH = ease(16, 22, frame - 40, 9);
    int eyeY = ease(29, 28, frame - 40, 9);
    int mouthW = ease(18, 16, frame - 40, 9);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.fillRoundRect(57 - mouthW / 2 + 7, 51, mouthW, 6, 3, COLOR_WHITE);
    canvas.drawBlush(18, 42, 110, 42, 2);
  }
}

// ===== 2.4 SAD — melancholy =====
// 56 frames @ 48ms = 2.69s loop.
// Phases: droop (F0-8), tear forms (F9-14), tear falls (F15-30), tremble (F31-40), recover (F41-55)

void drawSad(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 9) {
    // Droop: eyes lower Y=28→33, H=22→16, mouth narrows
    int eyeY = ease(28, 33, frame, 8);
    int eyeH = ease(22, 16, frame, 8);
    int mouthW = ease(10, 8, frame, 8);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.drawMouth(63 - mouthW / 2, 55, mouthW, 3);
  } else if (frame < 15) {
    // Tear forms below left eye inner edge
    canvas.drawEyes(38, 33, 90, 33, 16);
    canvas.drawMouth(59, 55, 8, 3);
    int tearR = (frame < 12) ? 1 : 2;
    canvas.fillCircle(48, 42, tearR, COLOR_WHITE);
  } else if (frame < 31) {
    // Tear falls: Y=42→60, trail line
    int tearY = ease(42, 60, frame - 15, 15);
    int trailEndY = tearY - 2;
    canvas.drawEyes(38, 33, 90, 33, 16);
    canvas.drawMouth(59, 55, 8, 3);
    canvas.fillCircle(48, tearY, 2, COLOR_WHITE);
    if (trailEndY > 43) {
      canvas.drawLine(48, 43, 48, trailEndY, COLOR_WHITE);
    }
  } else if (frame < 41) {
    // Tremble: 1px horizontal oscillation
    int xOff = ((frame % 2) == 0) ? -1 : 1;
    canvas.drawEyes(38 + xOff, 33, 90 + xOff, 33, 16);
    canvas.drawMouth(59 + xOff, 55, 8, 3);
  } else {
    // Full recovery: eyes rise Y=33→28, H=16→22 — full neutral before next wave
    int eyeY = ease(33, 28, frame - 41, 14);
    int eyeH = ease(16, 22, frame - 41, 14);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    if (frame >= 50) {
      canvas.drawMouth(57, 52, 14, 5);  // neutral mouth for final frames
    } else {
      canvas.drawMouth(59, 53, 10, 4);
    }
  }
}

// ===== 2.5 CONFUSED — head-tilty puzzlement =====
// 44 frames @ 35ms = 1.5s loop.
// Phases: tilt (F0-6), hold confused (F7-20), reverse tilt (F21-28), hold reversed (F29-36), settle (F37-43)

void drawConfused(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 7) {
    // Tilt: left eye grows taller, right shrinks
    int leftH = ease(20, 26, frame, 6);
    int leftY = ease(28, 22, frame, 6);
    int rightH = ease(20, 12, frame, 6);
    int rightY = ease(28, 29, frame, 6);
    canvas.fillRoundRect(26, leftY - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, rightY - rightH / 2, 24, rightH, 7, COLOR_WHITE);
    canvas.drawMouth(57, 53, 14, 4);
  } else if (frame < 21) {
    // Hold confused: left tall, right squat; "?" appears
    canvas.fillRoundRect(26, 22 - 13, 24, 26, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 29 - 6, 24, 12, 7, COLOR_WHITE);
    canvas.drawMouth(60, 53, 12, 4);
    if (frame >= 9) {
      canvas.setTextSize(1);
      canvas.setCursor(110, 8);
      canvas.print("?");
    }
    if (frame >= 15) {
      canvas.setTextSize(1);
      canvas.setCursor(116, 16);
      canvas.print("?");
    }
  } else if (frame < 29) {
    // Reverse tilt: eyes swap sizes
    int leftH = ease(26, 12, frame - 21, 7);
    int leftY_top = ease(22 - 13, 29 - 6, frame - 21, 7);
    int rightH = ease(12, 26, frame - 21, 7);
    int rightY_top = ease(29 - 6, 22 - 13, frame - 21, 7);
    canvas.fillRoundRect(26, leftY_top, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, rightY_top, 24, rightH, 7, COLOR_WHITE);
    canvas.drawMouth(58, 53, 12, 4);
  } else if (frame < 37) {
    // Hold reversed: left squat, right tall; two "?" marks
    canvas.fillRoundRect(26, 29 - 6, 24, 12, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 22 - 13, 24, 26, 7, COLOR_WHITE);
    canvas.drawMouth(58, 53, 12, 4);
    canvas.setTextSize(1);
    canvas.setCursor(108, 6);
    canvas.print("?");
    canvas.setTextSize(2);
    canvas.setCursor(116, 14);
    canvas.print("?");
  } else {
    // Settle: eyes return toward equal size
    int eyeH = ease(12, 20, frame - 37, 6);
    canvas.drawEyes(38, 28, 90, 28, eyeH);
    canvas.drawMouth(57, 53, 14, 4);
  }
}

// ===== 2.6 ANGRY — fuming frustration =====
// 56 frames @ 32ms = 1.79s loop.
// Phases: furrow (F0-6), hold glare (F7-14), shake (F15-34), intensify (F35-44), settle (F45-55)

void drawAngry(ICanvas& canvas, int frame, const void* ctx) {
  // Brow geometry: outer(x=18/110) is higher, inner(x=50/78) is lower — angry V shape.
  // All brow Y coords keep brow bottom pixel ≥ 2px above eye top (eyeY - eyeH/2).
  // Eye top in hold = 33-6 = 27; brow bottom (inner=20, thick=5) = 24 → 3px gap.
  if (frame < 7) {
    // Furrow: brows descend with eyes, gap maintained throughout
    int eyeH = ease(18, 12, frame, 6);
    int eyeY = ease(30, 33, frame, 6);
    int browThick = ease(2, 5, frame, 6);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.drawBrow(18, ease(9, 12, frame, 6), 50, ease(17, 20, frame, 6), browThick);
    canvas.drawBrow(78, ease(17, 20, frame, 6), 110, ease(9, 12, frame, 6), browThick);
    canvas.fillRoundRect(54, 55, 20, 4, 2, COLOR_WHITE);
  } else if (frame < 15) {
    // Hold glare: max furrow — extended pause builds menace before the explosion
    canvas.drawEyes(38, 33, 90, 33, 12);
    canvas.drawBrow(18, 12, 50, 20, 5);
    canvas.drawBrow(78, 20, 110, 12, 5);
    canvas.fillRoundRect(54, 55, 20, 4, 2, COLOR_WHITE);
  } else if (frame < 35) {
    // Shake: ±3px horizontal alternation
    int xOff = ((frame % 2) == 0) ? -3 : 3;
    canvas.drawEyes(38 + xOff, 33, 90 + xOff, 33, 12);
    canvas.drawBrow(18 + xOff, 12, 50 + xOff, 20, 5);
    canvas.drawBrow(78 + xOff, 20, 110 + xOff, 12, 5);
    canvas.fillRoundRect(54 + xOff, 55, 20, 4, 2, COLOR_WHITE);
  } else if (frame < 45) {
    // Intensify: ±4px shake, brows thicker
    int xOff = ((frame % 2) == 0) ? -4 : 4;
    canvas.drawEyes(38 + xOff, 33, 90 + xOff, 33, 11);
    canvas.drawBrow(18 + xOff, 11, 50 + xOff, 19, 6);
    canvas.drawBrow(78 + xOff, 19, 110 + xOff, 11, 6);
    canvas.fillRoundRect(54 + xOff, 55, 20, 4, 2, COLOR_WHITE);
  } else {
    // Settle: brows lift, eyes widen back to furrow-start — invisible seam with F0
    int eyeH = ease(12, 18, frame - 45, 10);
    int eyeY = ease(33, 30, frame - 45, 10);
    int browThick = ease(5, 2, frame - 45, 10);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.drawBrow(18, ease(12, 9, frame - 45, 10), 50, ease(20, 17, frame - 45, 10), browThick);
    canvas.drawBrow(78, ease(20, 17, frame - 45, 10), 110, ease(12, 9, frame - 45, 10), browThick);
    canvas.fillRoundRect(54, 54, 20, 4, 2, COLOR_WHITE);
  }
}

// ===== 2.8 LOVE — adoration =====
// 44 frames @ 35ms = 1.54s loop.
// Phases: transform (F0-5), pulse (F6-25), float (F26-35), settle (F36-43)

void drawLove(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 6) {
    // Transform: rounded eyes morph into hearts
    int r = ease(4, 7, frame, 5);
    if (frame < 2) {
      // Still rounded eyes in early frames
      canvas.drawEyes(38, 28, 90, 28, ease(18, 12, frame, 5));
    } else {
      drawHeartEye(canvas, 38, ease(30, 24, frame, 5), r);
      drawHeartEye(canvas, 90, ease(30, 24, frame, 5), r);
    }
    canvas.fillRoundRect(ease(57, 50, frame, 5), 51,
                         ease(14, 28, frame, 5), ease(5, 8, frame, 5),
                         4, COLOR_WHITE);
  } else if (frame < 26) {
    // Pulse: hearts alternate r=6↔8 every 2 frames
    int r = ((frame % 2) == 0) ? 8 : 6;
    drawHeartEye(canvas, 38, 24, r);
    drawHeartEye(canvas, 90, 24, r);
    canvas.fillRoundRect(48, 50, 32, 9, 4, COLOR_WHITE);
    int blushR = ((frame % 2) == 0) ? 4 : 3;
    canvas.drawBlush(16, 44, 112, 44, blushR);
    // Floating mini-heart outlines (paired circles)
    if ((frame % 4) < 2) {
      canvas.drawCircle(12, 10, 2, COLOR_WHITE);
      canvas.drawCircle(15, 10, 2, COLOR_WHITE);
    } else {
      canvas.drawCircle(116, 8, 2, COLOR_WHITE);
      canvas.drawCircle(119, 8, 2, COLOR_WHITE);
    }
  } else if (frame < 36) {
    // Float: heart outlines drift upward, stable hearts
    drawHeartEye(canvas, 38, 24, 7);
    drawHeartEye(canvas, 90, 24, 7);
    canvas.fillRoundRect(48, 50, 32, 9, 4, COLOR_WHITE);
    canvas.drawBlush(16, 44, 112, 44, 4);
    // Hearts drifting up-right
    int dy = (frame - 26);
    canvas.drawCircle(12 + dy / 2, 10 - dy, 2, COLOR_WHITE);
    canvas.drawCircle(15 + dy / 2, 10 - dy, 2, COLOR_WHITE);
  } else {
    // Settle: hearts at r=7, blush, wide smile
    drawHeartEye(canvas, 38, 24, 7);
    drawHeartEye(canvas, 90, 24, 7);
    canvas.fillRoundRect(48, 50, 32, 9, 4, COLOR_WHITE);
    canvas.drawBlush(16, 44, 112, 44, 5);
  }
}

// ===== 2.9 EXCITED — high energy bounce =====
// 40 frames @ 25ms = 1.0s loop.
// Phases: widen (F0-5), bounce (F6-25), settle (F26-31), ease back (F32-35), return (F36-39)

void drawExcited(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 6) {
    // Eyes widen H=22→28, pupils appear
    int eyeH = ease(22, 28, frame, 5);
    int eyeY = ease(28, 26, frame, 5);
    int mouthW = ease(24, 36, frame, 5);
    canvas.drawEyesWithPupils(38, eyeY, 90, eyeY, eyeH, 3);
    canvas.fillRoundRect(64 - mouthW / 2, 50, mouthW, 10, 5, COLOR_WHITE);
    if (frame >= 3) {
      canvas.drawSparkles(10, 10, 118, 10, 2);
    }
  } else if (frame < 26) {
    // Bounce: alternate up/down every frame
    int yOff = ((frame % 2) == 0) ? -3 : 3;
    canvas.drawEyesWithPupils(38, 26 + yOff, 90, 26 + yOff, 28, 3);
    canvas.fillRoundRect(46, 50 + yOff, 36, 10, 5, COLOR_WHITE);
    canvas.drawSparkles(((frame % 2) == 0) ? 12 : 10,
                        ((frame % 2) == 0) ? 8 : 14,
                        ((frame % 2) == 0) ? 116 : 118,
                        ((frame % 2) == 0) ? 8 : 14, 2);
  } else if (frame < 32) {
    // Settle: bounce dampens 3→2→1→0
    int amp = 3 - (frame - 26) / 2;
    if (amp < 0) amp = 0;
    int yOff = ((frame % 2) == 0) ? -amp : amp;
    canvas.drawEyesWithPupils(38, 26 + yOff, 90, 26 + yOff, 28, 3);
    canvas.fillRoundRect(46, 50 + yOff, 36, 10, 5, COLOR_WHITE);
  } else if (frame < 36) {
    // Ease back: eyes shrink slightly
    int eyeH = ease(28, 24, frame - 32, 3);
    canvas.drawEyesWithPupils(38, 26, 90, 26, eyeH, 3);
    canvas.fillRoundRect(50, 50, 28, 8, 4, COLOR_WHITE);
  } else {
    // Return: ease to neutral (H=22, Y=28) — invisible seam with F0
    int eyeH = ease(24, 22, frame - 36, 3);
    int eyeY = ease(26, 28, frame - 36, 3);
    canvas.drawEyesWithPupils(38, eyeY, 90, eyeY, eyeH, 3);
    canvas.fillRoundRect(51, 51, 26, 7, 4, COLOR_WHITE);
  }
}

// ===== 2.10 SLEEPY — drifting off =====
// 59 frames @ 80ms = 4.7s per cycle, LOOP_RESTART.
// Phases: closing (F0-10), deep sleep/z-cascade (F11-50), fast wake flutter (F51-58)

void drawSleepy(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 11) {
    // Closing: eyes droop H=22→3, Y=28→32; yawn circle grows
    int eyeH = ease(22, 3, frame, 10);
    int eyeY = ease(28, 32, frame, 10);
    int yawnR = ease(4, 7, frame, 10);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    if (frame < 8) {
      canvas.drawCircle(64, 50, yawnR, COLOR_WHITE);
    } else {
      canvas.fillCircle(64, 52, yawnR, COLOR_WHITE);
    }
  } else if (frame < 51) {
    // Deep sleep: slit eyes held, Z cascade builds then holds at full cascade
    canvas.drawEyes(38, 32, 90, 32, 2);
    canvas.fillCircle(64, 52, 7, COLOR_WHITE);
    // Z letters float up-right, one new Z every 5 frames; hold at 4
    int zCount = (frame - 11) / 5 + 1;
    if (zCount > 4) zCount = 4;
    for (int i = 0; i < zCount; i++) {
      canvas.setTextSize(1);
      canvas.setCursor(90 + i * 8, 28 - i * 6);
      canvas.print("z");
    }
    if (zCount >= 4) {
      canvas.setTextSize(1);
      canvas.setCursor(116, 6);
      canvas.print("Z");
    }
  } else {
    // Fast wake flutter: rapid blink as SANGI startles awake (8 frames)
    // Eyes alternate open/close with increasing height, Y springs back to neutral
    static const int8_t wakeEyeH[8] = {2, 14, 4, 18, 4, 22, 20, 22};
    int wakeFrame = frame - 51;
    int eyeH = wakeEyeH[wakeFrame < 8 ? wakeFrame : 7];
    int eyeY = ease(32, 28, wakeFrame, 7);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    // No yawn — snapped awake
  }
}

// ===== 2.11 THINKING — contemplative gaze =====
// 44 frames @ 35ms = 1.54s loop.
// Phases: look up-left (F0-6), hold+dots (F7-20), look up-right (F21-28), aha! (F29-36), return (F37-43)

void drawThinking(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 7) {
    // Squint: eyes shrink in place
    int eyeH = ease(20, 14, frame, 6);
    canvas.drawEyes(38, 28, 90, 28, eyeH);
    canvas.fillRoundRect(58, 53, 12, 4, 2, COLOR_WHITE);
  } else if (frame < 35) {
    // Slow formulation: small eyes, one dot every 9 frames (dots at F7, F16, F25)
    canvas.drawEyes(38, 28, 90, 28, 14);
    canvas.fillRoundRect(58, 53, 12, 4, 2, COLOR_WHITE);
    int dots = (frame - 7) / 9 + 1;
    if (dots > 3) dots = 3;
    for (int i = 0; i < dots; i++) {
      canvas.fillRect(110 + i * 6, 10, 3, 3, COLOR_WHITE);
    }
  } else if (frame < 40) {
    // Aha! Eyes snap open quickly, "!" replaces dots, mouth unchanged
    int eyeH = ease(14, 26, frame - 35, 4);
    canvas.drawEyes(38, 28, 90, 28, eyeH);
    canvas.fillRoundRect(58, 53, 12, 4, 2, COLOR_WHITE);
    canvas.fillRect(114, 6, 3, 14, COLOR_WHITE);
    canvas.fillRect(114, 22, 3, 3, COLOR_WHITE);
  } else {
    // Quick settle: eyes drop back to normal size
    int eyeH = ease(26, 20, frame - 40, 3);
    canvas.drawEyes(38, 28, 90, 28, eyeH);
    canvas.fillRoundRect(58, 53, 12, 4, 2, COLOR_WHITE);
  }
}

// ===== 2.12 SURPRISED — startled snap =====
// 44 frames @ 30ms = 1.32s loop.
// Phases: snap open (F0-5), hold shock (F6-10), double-take blink (F11-13),
//         tremor hold (F14-25), settle (F26-43)

void drawSurprised(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 6) {
    // Snap open: eyes widen H=20→28, mouth opens
    int eyeH = ease(20, 28, frame, 5);
    int eyeY = ease(28, 25, frame, 5);
    int mouthR = ease(4, 7, frame, 5);
    canvas.drawEyesWithPupils(38, eyeY, 90, eyeY, eyeH, 3);
    canvas.fillCircle(64, 53, mouthR, COLOR_WHITE);
  } else if (frame < 11) {
    // Hold shock
    canvas.drawEyesWithPupils(38, 25, 90, 25, 28, 3);
    canvas.fillCircle(64, 53, 7, COLOR_WHITE);
  } else if (frame == 11) {
    // Double-take shut: quick blink, no pupils
    canvas.drawEyes(38, 30, 90, 30, 4);
    canvas.fillCircle(64, 53, 7, COLOR_WHITE);
  } else if (frame < 14) {
    // Double-take reopen
    canvas.drawEyesWithPupils(38, 25, 90, 25, 28, 3);
    canvas.fillCircle(64, 53, 7, COLOR_WHITE);
  } else if (frame < 26) {
    // Tremor hold: 1px vertical jitter, mouth pulses
    int yOff = ((frame % 2) == 0) ? -1 : 1;
    int mouthR = ((frame % 2) == 0) ? 8 : 6;
    canvas.drawEyesWithPupils(38, 25 + yOff, 90, 25 + yOff, 28, 3);
    canvas.fillCircle(64, 53, mouthR, COLOR_WHITE);
  } else {
    // Settle: eyes shrink H=28→20, mouth closes — extended to F43 for invisible seam
    int eyeH = ease(28, 20, frame - 26, 17);
    int eyeY = ease(25, 28, frame - 26, 17);
    int mouthR = ease(7, 4, frame - 26, 17);
    canvas.drawEyesWithPupils(38, eyeY, 90, eyeY, eyeH, 3);
    if (mouthR > 3) {
      canvas.fillCircle(64, 53, mouthR, COLOR_WHITE);
    } else {
      canvas.drawCircle(64, 52, 4, COLOR_WHITE);
    }
  }
}

// ===== 2.13 DEAD — dark comedy knockout =====
// 70 frames @ 55ms = 3.85s loop.
// Phases: collapse (F0-8), X eyes form (F9-14), hold dead (F15-54), twitch (F55-69)

void drawDead(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 9) {
    // Collapse: eyes close slowly
    int eyeH = ease(20, 4, frame, 8);
    int eyeY = ease(28, 30, frame, 8);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.drawCircle(64, 52, 4, COLOR_WHITE);
  } else if (frame < 15) {
    // X eyes form: thickness grows 2→5
    int thick = ease(2, 5, frame - 9, 5);
    drawXEyes(canvas, thick);
    // Tongue starts protruding
    if (frame >= 12) {
      canvas.fillRoundRect(58, 52, 12, 4, 3, COLOR_WHITE);
      canvas.fillRoundRect(61, 55, 6, 3, 2, COLOR_WHITE);
    }
  } else if (frame < 55) {
    // Hold dead: X eyes, tongue out, dizzy circles orbit at edges — extended 2.2s hold
    drawXEyes(canvas, 5);
    canvas.fillRoundRect(58, 52, 12, 6, 3, COLOR_WHITE);
    canvas.fillRoundRect(62, 56, 6, 6, 3, COLOR_WHITE);
    // Dizzy circles shift positions every 7 frames (slower orbit)
    int dizzyPhase = (frame - 15) / 7;
    int dx = (dizzyPhase % 2 == 0) ? 0 : 2;
    canvas.drawCircle(16 + dx, 18, 3, COLOR_WHITE);
    canvas.drawCircle(112 - dx, 22, 3, COLOR_WHITE);
  } else {
    // Twitch: two isolated clusters for dark comedy, holds still between them
    int xOff = 0;
    if (frame == 57 || frame == 58) xOff = 1;
    if (frame == 62 || frame == 63) xOff = -1;
    drawXEyes(canvas, 5);  // X eyes don't move (static)
    canvas.fillRoundRect(58 + xOff, 52, 12, 6, 3, COLOR_WHITE);
    canvas.fillRoundRect(62 + xOff, 56, 6, 6, 3, COLOR_WHITE);
  }
}

// ===== 2.14 BORED — disinterested half-lids =====
// 60 frames @ 65ms = 3.9s half-cycle, 7.8s full ping-pong cycle.
// Phases: droop (F0-12), hold (F13-18), slow blink (F19-22), drift (F23-35),
//         sigh (F36-42), reopen (F43-59)

void drawBored(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 13) {
    // Droop: eyes close H=22→8, right eye 2px lower (head tilt)
    int eyeH = ease(22, 8, frame, 12);
    int leftY = ease(28, 28, frame, 12);   // left stays at 28
    int rightY = ease(30, 30, frame, 12);  // right 2px lower (tilt) — constant
    canvas.fillRoundRect(26, leftY - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, rightY - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(57, 53, 14, 4, 2, COLOR_WHITE);
  } else if (frame < 19) {
    // Hold half-closed: H=8, tilt maintained
    canvas.fillRoundRect(26, 28 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 30 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.fillRoundRect(57, 53, 14, 4, 2, COLOR_WHITE);
  } else if (frame < 23) {
    // Slow blink: H=2 then back to H=8
    int eyeH = (frame < 21) ? 2 : 8;
    canvas.fillRoundRect(26, 28 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 30 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(57, 53, 14, 4, 2, COLOR_WHITE);
  } else if (frame < 36) {
    // Drift: H=8, tilt
    canvas.fillRoundRect(26, 28 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 30 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.fillRoundRect(57, 53, 14, 4, 2, COLOR_WHITE);
  } else if (frame < 43) {
    // Sigh: mouth opens into small O
    canvas.fillRoundRect(26, 28 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 30 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.drawCircle(64, 54, 4, COLOR_WHITE);
  } else {
    // Reopen: eyes to H=22 (full neutral) — clean ping-pong turnaround at F59
    int eyeH = ease(8, 22, frame - 43, 16);
    canvas.fillRoundRect(26, 28 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 30 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(57, 53, 14, 4, 2, COLOR_WHITE);
  }
}

// ===== 2.15 SHY — bashful recovery after neglect =====
// 50 frames @ 60ms = 3.0s loop.
// Phases: startle (F0-7), avert (F8-18), peek (F19-30), warm up (F31-41), linger (F42-49)
// Visually distinct: asymmetric eye heights (one squinting, one peeking), prominent blush.
// Eyes fixed at standard positions (38, 90), Y=28. Expression through scale only.

void drawShy(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 8) {
    // Startle: both eyes squeeze shut asymmetrically — left hides more than right
    int leftH  = ease(22, 4, frame, 7);   // left squints nearly shut
    int rightH = ease(22, 10, frame, 7);   // right stays half-open (peeking)
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 28 - rightH / 2, 24, rightH, 7, COLOR_WHITE);
    // Blush starts appearing
    if (frame >= 4) {
      int blushR = ease(1, 3, frame - 4, 3);
      canvas.drawBlush(16, 42, 108, 42, blushR);
    }
  } else if (frame < 19) {
    // Avert: left eye a narrow slit, right eye half-open — bashful squint
    int leftH  = ease(4, 3, frame - 8, 10);   // left barely open
    int rightH = ease(10, 12, frame - 8, 10);  // right peeks open a touch more
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 28 - rightH / 2, 24, rightH, 7, COLOR_WHITE);
    // Full blush, pulses gently
    int blushR = ((frame % 4) < 2) ? 4 : 5;
    canvas.drawBlush(16, 42, 108, 42, blushR);
    // Tiny closed mouth — pressed together nervously
    canvas.fillRoundRect(60, 53, 8, 3, 2, COLOR_WHITE);
  } else if (frame < 31) {
    // Peek: both eyes open further, left catches up — timid glance
    int f = frame - 19;
    int leftH  = ease(3, 12, f, 11);    // left cracks open
    int rightH = ease(12, 18, f, 11);   // right opens wider
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 28 - rightH / 2, 24, rightH, 7, COLOR_WHITE);
    // Blush steady
    canvas.drawBlush(16, 42, 108, 42, 5);
    // Small tentative smile starts forming
    int mouthW = ease(8, 14, f, 11);
    int mouthH = ease(3, 4, f, 11);
    canvas.fillRoundRect(60, 53, mouthW, mouthH, 2, COLOR_WHITE);
  } else if (frame < 42) {
    // Warm up: eyes become more symmetric, approaching neutral
    int f = frame - 31;
    int leftH  = ease(12, 20, f, 10);   // left nearly matches right
    int rightH = ease(18, 22, f, 10);   // right reaches full open
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 28 - rightH / 2, 24, rightH, 7, COLOR_WHITE);
    // Blush fading
    int blushR = ease(5, 3, f, 10);
    canvas.drawBlush(16, 42, 108, 42, blushR);
    // Smile grows warmer
    int mouthW = ease(14, 18, f, 10);
    int mouthH = ease(4, 5, f, 10);
    canvas.fillRoundRect(60, 53, mouthW, mouthH, 3, COLOR_WHITE);
  } else {
    // Linger: near-neutral with residual blush — left eye still slightly smaller
    int f = frame - 42;
    int leftH  = ease(20, 22, f, 7);
    int rightH = 22;
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 28 - rightH / 2, 24, rightH, 7, COLOR_WHITE);
    // Blush lingers softly
    int blushR = ease(3, 1, f, 7);
    if (blushR > 0) canvas.drawBlush(16, 42, 108, 42, blushR);
    // Gentle smile
    canvas.fillRoundRect(60, 53, 14, 5, 3, COLOR_WHITE);
  }
}

// ===== 2.16 NEEDY — pleading solicitation =====
// 54 frames @ 65ms = 3.5s loop.
// Phases: swell (F0-14), plead (F15-35), quiver (F36-45), settle (F46-53)
// Visually distinct: oversized eyes (H=28 at peak, vs 22 normal), pupils looking
// upward, small open mouth that trembles. No blush — NEEDY is direct, not bashful.
// Eyes at standard X positions (38, 90). Y shifts up slightly to create "looking up at you."

void drawNeedy(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 15) {
    // Swell: eyes grow from neutral H=22 to oversized H=28, shift up Y=28→25
    int eyeH = ease(22, 28, frame, 14);
    int eyeY = ease(28, 25, frame, 14);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    // Sad mouth fades in — thick downturned arc
    int mouthW = ease(0, 14, frame, 14);
    if (mouthW > 4) {
      int mx = 64 - mouthW / 2;
      for (int i = 0; i < 3; i++) {
        canvas.drawLine(mx, 53 + i, 64, 56 + i, COLOR_WHITE);
        canvas.drawLine(64, 56 + i, mx + mouthW, 53 + i, COLOR_WHITE);
      }
    }
  } else if (frame < 36) {
    // Plead: eyes pulse H=28→25→28 in slow rhythm
    int pulseFrame = (frame - 15) % 10;
    int eyeH;
    if (pulseFrame < 5) {
      eyeH = ease(28, 25, pulseFrame, 4);
    } else {
      eyeH = ease(25, 28, pulseFrame - 5, 4);
    }
    canvas.drawEyes(38, 25, 90, 25, eyeH);
    // Small highlight dots near top of each eye — catch-light for pleading look
    canvas.fillCircle(42, 20, 2, COLOR_BLACK);
    canvas.fillCircle(94, 20, 2, COLOR_BLACK);
    // Sad mouth — thick downturned arc, quivers slightly
    int mouthW = ((frame % 6) < 3) ? 14 : 12;
    int mx = 64 - mouthW / 2;
    int dip = ((frame % 6) < 3) ? 56 : 55;
    for (int i = 0; i < 3; i++) {
      canvas.drawLine(mx, 53 + i, 64, dip + i, COLOR_WHITE);
      canvas.drawLine(64, dip + i, mx + mouthW, 53 + i, COLOR_WHITE);
    }
  } else if (frame < 46) {
    // Quiver: eyes tremble — height jitters between 27 and 28
    int eyeH = ((frame % 2) == 0) ? 28 : 27;
    canvas.drawEyes(38, 25, 90, 25, eyeH);
    canvas.fillCircle(42, 20, 2, COLOR_BLACK);
    canvas.fillCircle(94, 20, 2, COLOR_BLACK);
    // Mouth quivers — thick
    int dip = ((frame % 2) == 0) ? 56 : 55;
    for (int i = 0; i < 3; i++) {
      canvas.drawLine(57, 53 + i, 64, dip + i, COLOR_WHITE);
      canvas.drawLine(64, dip + i, 71, 53 + i, COLOR_WHITE);
    }
  } else {
    // Settle: eyes ease back toward neutral H=28→22, Y=25→28
    int f = frame - 46;
    int eyeH = ease(28, 22, f, 7);
    int eyeY = ease(25, 28, f, 7);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    // Mouth fades out — thick
    int mouthW = ease(14, 0, f, 7);
    if (mouthW > 4) {
      int mx = 64 - mouthW / 2;
      for (int i = 0; i < 3; i++) {
        canvas.drawLine(mx, 53 + i, 64, 56 + i, COLOR_WHITE);
        canvas.drawLine(64, 56 + i, mx + mouthW, 53 + i, COLOR_WHITE);
      }
    }
  }
}

// ===== 2.18 PLAYFUL — mischievous invitation =====
// 48 frames @ 40ms = 1.92s loop, LOOP_RESTART.
// Signature: left eye half-squinted (H=10) vs right eye open (H=22) — asymmetric play-face.
// Grin shifted 5px right of face center (center at x=69 vs face center x=64).
// Wink at F9-23 (left eye closes to H=2, holds, reopens). Invitation bob at F24-37.
// vs HAPPY:    asymmetric + no blush — mischievous grin, not warm centered smile.
// vs THINKING: THINKING narrows both eyes, turns inward; PLAYFUL directs outward with open right eye.
// vs EXCITED:  no pupils, no sharp alternation — beckoning, not overwhelmed.

// Draws the PLAYFUL asymmetric grin with thickness lines.
// Left corner at (lx, ly), right corner raised to (rx, ly-rise).
// A two-segment line — left half rises from corner to center, right half continues rising.
static void drawPlayfulGrin(ICanvas& c, int lx, int rx, int ly, int rise, int thick) {
  int cx = (lx + rx) / 2;
  int cy = ly - rise / 2;   // midpoint Y — halfway up the rise
  for (int i = 0; i < thick; i++) {
    c.drawLine(lx, ly + i, cx, cy + i, COLOR_WHITE);   // left half: corner → mid
    c.drawLine(cx, cy + i, rx, (ly - rise) + i, COLOR_WHITE);  // right half: mid → raised corner
  }
}

void drawPlayful(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 9) {
    // Build: left squints H=22→10, right stays open H=22; asymmetric grin grows
    int leftH  = ease(22, 10, frame, 8);
    int mouthW = ease(4, 24, frame, 8);
    int rise   = mouthW * 6 / 24;  // right corner rises proportionally (0→6px)
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 17,             24, 22,    7, COLOR_WHITE);
    drawPlayfulGrin(canvas, 64 - mouthW / 2, 64 + mouthW / 2, 58, rise, 3);
  } else if (frame < 24) {
    // Wink: left closes F9-13, held shut F14-15, reopens F16-23
    int leftH;
    if (frame < 14) {
      leftH = ease(10, 2, frame - 9, 4);
    } else if (frame < 16) {
      leftH = 2;
    } else {
      leftH = ease(2, 10, frame - 16, 7);
    }
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 17,              24, 22,    7, COLOR_WHITE);
    // Asymmetric grin: left corner y=58, right corner raised to y=52 (+6px)
    drawPlayfulGrin(canvas, 52, 76, 58, 6, 3);
  } else if (frame < 38) {
    // Bounce: play-bow bob ±2px, asymmetric grin held
    int yOff = ((frame - 24) % 6 < 3) ? -2 : 0;
    canvas.fillRoundRect(26, 28 + yOff - 5,  24, 10, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 28 + yOff - 11, 24, 22, 7, COLOR_WHITE);
    drawPlayfulGrin(canvas, 52, 76, 58 + yOff, 6, 3);
  } else {
    // Settle: left eye H=10→22, grin shrinks W=24→4 — invisible seam back to F0
    int f      = frame - 38;
    int leftH  = ease(10, 22, f, 9);
    int mouthW = ease(24, 4, f, 9);
    int rise   = mouthW * 6 / 24;
    canvas.fillRoundRect(26, 28 - leftH / 2, 24, leftH, 7, COLOR_WHITE);
    canvas.fillRoundRect(78, 17,             24, 22,    7, COLOR_WHITE);
    drawPlayfulGrin(canvas, 64 - mouthW / 2, 64 + mouthW / 2, 58, rise, 3);
  }
}

// ===== 2.17 CONTENT — quiet satisfaction =====
// 60 frames @ 90ms = 5.4s loop, LOOP_PINGPONG.
// The purring state. Distinct from HAPPY (no bounce, no sparkles) and BORED (not disengaged).
// vs BORED: eyes more open (H=14 vs 8), no head tilt, wide smile, gentle blush pulse.
// vs HAPPY: no bounce, no sparkles, slower tempo — settled warmth, not excitement.
// Phases: settle (F0-14), deep content (F15-40), slow blink (F41-48), warm hold (F49-59)

void drawContent(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 15) {
    // Settle: eyes ease from neutral to relaxed H=14, Y drifts slightly
    int eyeH = ease(22, 14, frame, 14);
    int eyeY = ease(28, 29, frame, 14);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    // Smile grows — wide, gentle, closed-mouth curve
    int mouthW = ease(14, 28, frame, 14);
    int mouthH = ease(5, 7, frame, 14);
    canvas.fillRoundRect(64 - mouthW / 2, 51, mouthW, mouthH, 4, COLOR_WHITE);
    // Blush fades in — warmth signal
    if (frame >= 6) {
      int blushR = ease(1, 4, frame - 6, 8);
      canvas.drawBlush(18, 44, 110, 44, blushR);
    }
  } else if (frame < 41) {
    // Deep content: relaxed eyes with slow breathing bob (Y oscillates 29→28→29)
    int breathCycle = (frame - 15) % 13;
    int eyeY;
    if (breathCycle < 7) {
      eyeY = ease(29, 28, breathCycle, 6);
    } else {
      eyeY = ease(28, 29, breathCycle - 7, 5);
    }
    canvas.drawEyes(38, eyeY, 90, eyeY, 14);
    // Wide gentle smile held steady
    canvas.fillRoundRect(50, 51, 28, 7, 4, COLOR_WHITE);
    // Blush pulses gently — like a purr rhythm
    int blushR = ((frame % 8) < 4) ? 4 : 5;
    canvas.drawBlush(18, 44, 110, 44, blushR);
  } else if (frame < 49) {
    // Slow trust-blink: eyes close gently then reopen
    int blinkFrame = frame - 41;
    int eyeH;
    if (blinkFrame < 4) {
      eyeH = ease(14, 3, blinkFrame, 3);  // close
    } else {
      eyeH = ease(3, 14, blinkFrame - 4, 3);  // reopen
    }
    canvas.drawEyes(38, 29, 90, 29, eyeH);
    canvas.fillRoundRect(50, 51, 28, 7, 4, COLOR_WHITE);
    canvas.drawBlush(18, 44, 110, 44, 4);
  } else {
    // Warm hold: eyes relax slightly more, smile softens — seamless loop point
    int eyeH = ease(14, 16, frame - 49, 10);
    int mouthW = ease(28, 24, frame - 49, 10);
    canvas.drawEyes(38, 29, 90, 29, eyeH);
    canvas.fillRoundRect(64 - mouthW / 2, 51, mouthW, 7, 4, COLOR_WHITE);
    // Blush gently fading toward loop seam
    int blushR = ease(4, 3, frame - 49, 10);
    canvas.drawBlush(18, 44, 110, 44, blushR);
  }
}
