#include "animations.h"
#include "emotion_registry.h"

AnimationManager animationManager;

AnimationManager::AnimationManager() {
  for (int i = 0; i <= EMOTION_GITHUB_STATS; i++) {
    states_[i].frame = 0;
    states_[i].lastTick = 0;
  }
}

void AnimationManager::resetAnimation(EmotionState emotion) {
  if (emotion >= 0 && emotion <= EMOTION_GITHUB_STATS) {
    states_[emotion].frame = 0;
    states_[emotion].lastTick = 0;
  }
}

bool AnimationManager::tick(EmotionState emotion, ICanvas& canvas,
                            const void* context) {
  const EmotionDef* def = emotionRegistry.get(emotion);
  if (!def || !def->drawFrame) return false;

  AnimState& s = states_[emotion];
  unsigned long now = millis();

  // Handle millis() overflow
  if (now < s.lastTick) {
    s.lastTick = now;
  }

  if (now - s.lastTick < def->frameDelay) return false;

  canvas.clear();
  def->drawFrame(canvas, s.frame, context);
  canvas.flush();

  // Advance frame
  if (def->loop == LOOP_RESTART) {
    s.frame = (s.frame + 1) % def->frameCount;
  } else {  // LOOP_ONCE
    if (s.frame < def->frameCount - 1) s.frame++;
  }
  s.lastTick = now;
  return true;
}
