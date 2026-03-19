// AnimationManager — generic frame-based animation engine.
// CHANGED in Phase 2:
//   - 28 individual frame/timing variables replaced by AnimState array
//   - 14 animateXxx() methods replaced by single generic tick()
//   - tick() looks up the emotion in EmotionRegistry and calls its DrawFrameFn

#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <Arduino.h>
#include "config.h"
#include "emotion.h"
#include "canvas.h"

struct AnimState {
  int frame;
  unsigned long lastTick;
};

class AnimationManager {
public:
  AnimationManager();

  void resetAnimation(EmotionState emotion);

  // Generic: advances frame and calls the registered DrawFrameFn.
  // Returns true if a frame was drawn this call.
  bool tick(EmotionState emotion, ICanvas& canvas, const void* context = nullptr);

private:
  AnimState states_[EMOTION_GITHUB_STATS + 1];
};

extern AnimationManager animationManager;

#endif // ANIMATIONS_H
