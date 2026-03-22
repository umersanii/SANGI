#ifndef EMOTION_DRAWS_H
#define EMOTION_DRAWS_H

// Forward declarations of all DrawFrameFn implementations.
// Each function draws a single frame of an emotion onto the provided canvas.
// The tick() method in AnimationManager handles clear/flush and frame advancement.

#include "canvas.h"

// Static emotions (1 frame)
void drawIdle(ICanvas& canvas, int frame, const void* ctx);
void drawBlink(ICanvas& canvas, int frame, const void* ctx);

// Standard animated emotions (51 frames @ 30ms)
void drawHappy(ICanvas& canvas, int frame, const void* ctx);
void drawSleepy(ICanvas& canvas, int frame, const void* ctx);
void drawExcited(ICanvas& canvas, int frame, const void* ctx);
void drawSad(ICanvas& canvas, int frame, const void* ctx);
void drawAngry(ICanvas& canvas, int frame, const void* ctx);
void drawConfused(ICanvas& canvas, int frame, const void* ctx);
void drawThinking(ICanvas& canvas, int frame, const void* ctx);
void drawLove(ICanvas& canvas, int frame, const void* ctx);
void drawSurprised(ICanvas& canvas, int frame, const void* ctx);
void drawDead(ICanvas& canvas, int frame, const void* ctx);
void drawBored(ICanvas& canvas, int frame, const void* ctx);
void drawShy(ICanvas& canvas, int frame, const void* ctx);

#endif // EMOTION_DRAWS_H
