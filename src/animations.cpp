#include "animations.h"
#include "emotion_registry.h"

AnimationManager animationManager;

AnimationManager::AnimationManager() {
  for (int i = 0; i < EmotionRegistry::MAX_EMOTIONS; i++) {
    states_[i].frame = 0;
    states_[i].lastTick = 0;
    states_[i].direction = 1;
  }
}

void AnimationManager::resetAnimation(EmotionState emotion) {
  if ((int)emotion >= 0 && (int)emotion < EmotionRegistry::MAX_EMOTIONS) {
    states_[emotion].frame = 0;
    states_[emotion].lastTick = 0;
    states_[emotion].direction = 1;
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
  } else if (def->loop == LOOP_ONCE) {
    if (s.frame < def->frameCount - 1) s.frame++;
  } else {  // LOOP_PINGPONG
    s.frame += s.direction;
    if (s.frame >= def->frameCount) {
      s.frame = def->frameCount - 2;  // turnaround: don't double-count last frame
      s.direction = -1;
    } else if (s.frame < 0) {
      s.frame = 1;  // turnaround: don't double-count first frame
      s.direction = 1;
    }
  }
  s.lastTick = now;
  return true;
}
