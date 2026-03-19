#ifndef EMOTION_REGISTRY_H
#define EMOTION_REGISTRY_H

// EmotionRegistry — central catalog of all emotion definitions.
// Replaces hardcoded switch statements and ternary chains with a
// data-driven lookup. Adding a new emotion = one registry.add() call.

#include "emotion.h"  // EmotionState enum

class ICanvas;  // Forward declaration

// How an animation behaves when it reaches the last frame
enum LoopBehavior {
  LOOP_RESTART,  // Loop from frame 0 (most emotions)
  LOOP_ONCE      // Play once, hold last frame (notification)
};

// Signature for a function that draws one animation frame.
//   canvas:  the drawing surface (real OLED or mock)
//   frame:   current frame index [0, frameCount)
//   context: optional payload (e.g., notification title/message)
typedef void (*DrawFrameFn)(ICanvas& canvas, int frame, const void* context);

// Metadata describing a single emotion
struct EmotionDef {
  EmotionState id;
  const char* name;            // Human-readable name (for logs and debug)
  int frameCount;              // 1 = static face, >1 = animated sequence
  unsigned long frameDelay;    // ms between frames (30 = ~33 FPS)
  LoopBehavior loop;
  bool blinkable;              // Whether natural-blink overlay can apply
  DrawFrameFn drawFrame;       // Render function (nullptr until Phase 2)
};

class EmotionRegistry {
public:
  static const int MAX_EMOTIONS = 20;

  EmotionRegistry();

  // Register a new emotion. Returns false if full or duplicate id.
  bool add(const EmotionDef& def);

  // Look up by id. Returns nullptr if not found.
  const EmotionDef* get(EmotionState id) const;

  // Shorthand: get(id)->name with "UNKNOWN" fallback.
  const char* getName(EmotionState id) const;

  int count() const { return count_; }

  // Fill 'out' with ids suitable for autonomous cycling (excludes BLINK).
  // Returns number of entries written.
  int getCyclable(EmotionState* out, int maxCount) const;

private:
  EmotionDef entries_[MAX_EMOTIONS];
  int count_;
};

extern EmotionRegistry emotionRegistry;

#endif // EMOTION_REGISTRY_H
