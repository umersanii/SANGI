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
  if (frame < 7) {
    // Furrow: brows descend, eyes narrow and drop
    int eyeH = ease(18, 12, frame, 6);
    int eyeY = ease(30, 33, frame, 6);
    int browThick = ease(2, 5, frame, 6);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.drawBrow(18, ease(22, 16, frame, 6), 50, ease(28, 24, frame, 6), browThick);
    canvas.drawBrow(78, ease(28, 24, frame, 6), 110, ease(22, 16, frame, 6), browThick);
    canvas.fillRoundRect(54, 55, 20, 4, 2, COLOR_WHITE);
  } else if (frame < 15) {
    // Hold glare: max furrow — extended pause builds menace before the explosion
    canvas.drawEyes(38, 33, 90, 33, 12);
    canvas.drawBrow(18, 16, 50, 24, 5);
    canvas.drawBrow(78, 24, 110, 16, 5);
    canvas.fillRoundRect(54, 55, 20, 4, 2, COLOR_WHITE);
  } else if (frame < 35) {
    // Shake: ±3px horizontal alternation
    int xOff = ((frame % 2) == 0) ? -3 : 3;
    canvas.drawEyes(38 + xOff, 33, 90 + xOff, 33, 12);
    canvas.drawBrow(18 + xOff, 16, 50 + xOff, 24, 5);
    canvas.drawBrow(78 + xOff, 24, 110 + xOff, 16, 5);
    canvas.fillRoundRect(54 + xOff, 55, 20, 4, 2, COLOR_WHITE);
  } else if (frame < 45) {
    // Intensify: ±4px shake, brows thicker
    int xOff = ((frame % 2) == 0) ? -4 : 4;
    canvas.drawEyes(38 + xOff, 33, 90 + xOff, 33, 11);
    canvas.drawBrow(18 + xOff, 15, 50 + xOff, 25, 6);
    canvas.drawBrow(78 + xOff, 25, 110 + xOff, 15, 6);
    canvas.fillRoundRect(54 + xOff, 55, 20, 4, 2, COLOR_WHITE);
  } else {
    // Settle: brows lift, eyes widen back to furrow-start — invisible seam with F0
    int eyeH = ease(12, 18, frame - 45, 10);
    int eyeY = ease(33, 30, frame - 45, 10);
    int browThick = ease(5, 2, frame - 45, 10);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    canvas.drawBrow(18, ease(16, 22, frame - 45, 10), 50, ease(24, 28, frame - 45, 10), browThick);
    canvas.drawBrow(78, ease(24, 28, frame - 45, 10), 110, ease(16, 22, frame - 45, 10), browThick);
    canvas.fillRoundRect(54, 54, 20, 4, 2, COLOR_WHITE);
  }
}

// ===== 2.7 SHY — bashful peek =====
// 36 frames @ 40ms = 1.44s loop (LOOP_ONCE — holds final frame)
// Phases: shrink (F0-7), look down (F8-15), peek (F16-23), settle (F24-31), hold (F32-35)

void drawShy(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 8) {
    // Eyes narrow and move inward, blush appears
    // Left eye moves from x=38 rightward to x=44; right eye from x=90 leftward to x=84
    int leftX = ease(38, 44, frame, 7);
    int rightX = ease(90, 84, frame, 7);
    int eyeH = ease(20, 14, frame, 7);
    canvas.fillRoundRect(leftX - 12, 24 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(rightX - 12, 24 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    if (frame >= 3) {
      canvas.drawBlush(18, 42, 110, 42, frame - 1);
    }
  } else if (frame < 16) {
    // Look down: eyes shift to Y=32, H=10
    int eyeY = ease(24, 32, frame - 8, 7);
    int eyeH = ease(14, 10, frame - 8, 7);
    canvas.fillRoundRect(44 - 12, eyeY - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(84 - 12, eyeY - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.drawBlush(18, 42, 110, 42, 5);
    canvas.drawMouth(59, 53, 10, 4);
  } else if (frame < 24) {
    // Peek: right eye rises, left stays down
    int rightY = ease(32, 27, frame - 16, 7);
    canvas.fillRoundRect(44 - 12, 32 - 5, 24, 10, 7, COLOR_WHITE);  // left stays
    canvas.fillRoundRect(84 - 12, rightY - 7, 24, 14, 7, COLOR_WHITE);  // right rises
    int blushR = 6 - (frame - 16) / 4;
    canvas.drawBlush(18, 42, 110, 42, blushR > 4 ? blushR : 4);
    canvas.drawMouth(59, 53, 10, 4);
  } else if (frame < 32) {
    // Settle: both eyes partially recover
    int leftX = ease(44, 38, frame - 24, 7);
    int rightX = ease(84, 90, frame - 24, 7);
    int eyeH = ease(10, 16, frame - 24, 7);
    canvas.fillRoundRect(leftX - 12, 28 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.fillRoundRect(rightX - 12, 27 - eyeH / 2, 24, eyeH, 7, COLOR_WHITE);
    canvas.drawBlush(18, 42, 110, 42, 4);
    canvas.fillRoundRect(56, 52, 16, 5, 3, COLOR_WHITE);
  } else {
    // Hold: shy pose with blush and small smile, right eye 1px higher (glance)
    canvas.drawEyes(38, 28, 90, 27, 16);
    canvas.drawBlush(18, 42, 110, 42, 4);
    canvas.fillRoundRect(56, 52, 16, 5, 3, COLOR_WHITE);
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
// 60 frames @ 50ms = 3.0s half-cycle, 6.0s full ping-pong cycle.
// Phases: closing (F0-10), sleeping/z-cascade (F11-35), micro-stir (F36-42), reopen (F43-59)

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
  } else if (frame < 36) {
    // Sleeping: slit eyes, Z cascade builds up
    canvas.drawEyes(38, 32, 90, 32, 2);
    canvas.fillCircle(64, 52, 7, COLOR_WHITE);
    // Z letters float up-right, one new Z added every 5 frames
    int zCount = (frame - 11) / 5 + 1;
    if (zCount > 4) zCount = 4;
    for (int i = 0; i < zCount; i++) {
      canvas.setTextSize(1);
      canvas.setCursor(90 + i * 8, 28 - i * 6);
      canvas.print("z");
    }
    // Capital Z at top when cascade is full
    if (zCount >= 4) {
      canvas.setTextSize(1);
      canvas.setCursor(116, 6);
      canvas.print("Z");
    }
  } else if (frame < 43) {
    // Micro-stir: eyes crack open then close again
    int stirH = (frame < 39) ? ease(2, 5, frame - 36, 3) : ease(5, 2, frame - 39, 3);
    canvas.drawEyes(38, 32, 90, 32, stirH);
    canvas.fillCircle(64, 52, 6, COLOR_WHITE);
  } else {
    // Reopen: eyes to H=22 (full neutral), yawn shrinks to 0 — clean ping-pong turnaround
    int eyeH = ease(3, 22, frame - 43, 16);
    int eyeY = ease(32, 28, frame - 43, 16);
    int yawnR = ease(7, 0, frame - 43, 16);
    canvas.drawEyes(38, eyeY, 90, eyeY, eyeH);
    if (yawnR > 0) {
      canvas.drawCircle(64, 50, yawnR, COLOR_WHITE);
    }
  }
}

// ===== 2.11 THINKING — contemplative gaze =====
// 44 frames @ 35ms = 1.54s loop.
// Phases: look up-left (F0-6), hold+dots (F7-20), look up-right (F21-28), aha! (F29-36), return (F37-43)

void drawThinking(ICanvas& canvas, int frame, const void* ctx) {
  if (frame < 7) {
    // Look up-left: eyes shift -4px X, -4px Y
    int eyeLX = ease(38, 34, frame, 6);
    int eyeRX = ease(90, 86, frame, 6);
    int eyeY = ease(28, 24, frame, 6);
    canvas.drawEyes(eyeLX, eyeY, eyeRX, eyeY, 20);
    canvas.fillRoundRect(58, 53, 12, 4, 2, COLOR_WHITE);
  } else if (frame < 21) {
    // Hold: eyes looking up-left, dots appear sequentially
    canvas.drawEyes(34, 24, 86, 24, 20);
    canvas.fillRoundRect(56, 53, 12, 4, 2, COLOR_WHITE);
    // One dot appears every 4 frames
    int dots = (frame - 7) / 4 + 1;
    if (dots > 3) dots = 3;
    for (int i = 0; i < dots; i++) {
      canvas.fillRect(110 + i * 6, 10, 3, 3, COLOR_WHITE);
    }
  } else if (frame < 29) {
    // Look up-right: eyes shift +4px X
    int eyeLX = ease(34, 42, frame - 21, 7);
    int eyeRX = ease(86, 94, frame - 21, 7);
    canvas.drawEyes(eyeLX, 24, eyeRX, 24, 20);
    canvas.fillRoundRect(58, 53, 12, 4, 2, COLOR_WHITE);
    canvas.fillRect(110, 10, 3, 3, COLOR_WHITE);
    canvas.fillRect(116, 10, 3, 3, COLOR_WHITE);
    canvas.fillRect(122, 10, 3, 3, COLOR_WHITE);
  } else if (frame < 37) {
    // Aha! Eyes widen +2, "!" replaces dots
    int eyeH = ease(20, 22, frame - 29, 4);
    canvas.drawEyes(42, 24, 94, 24, eyeH);
    canvas.fillRoundRect(60, 53, 12, 4, 2, COLOR_WHITE);
    // Exclamation mark
    canvas.fillRect(114, 6, 3, 14, COLOR_WHITE);
    canvas.fillRect(114, 22, 3, 3, COLOR_WHITE);
  } else {
    // Return to center
    int eyeLX = ease(42, 38, frame - 37, 6);
    int eyeRX = ease(94, 90, frame - 37, 6);
    int eyeY = ease(24, 28, frame - 37, 6);
    canvas.drawEyes(eyeLX, eyeY, eyeRX, eyeY, 20);
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
    // Drift: H=8, tilt; side glance on F28-30 (right eye shifts 2px right)
    int rightXOff = (frame >= 28 && frame <= 30) ? 2 : 0;
    canvas.fillRoundRect(26, 28 - 4, 24, 8, 7, COLOR_WHITE);
    canvas.fillRoundRect(78 + rightXOff, 30 - 4, 24, 8, 7, COLOR_WHITE);
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
