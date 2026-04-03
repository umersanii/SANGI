// EmotionRegistry implementation.
// Phase 1: Provides name lookup and emotion cycling.
// Phase 2: drawFrame pointers will be populated to replace the main.cpp switch.

#include "emotion_registry.h"

EmotionRegistry emotionRegistry;

// Initializes the registry with zero registered entries.
EmotionRegistry::EmotionRegistry() : count_(0) {}

// Adds an emotion definition. Returns false if the emotion ID is already registered or the registry is full.
bool EmotionRegistry::add(const EmotionDef& def) {
  // Reject duplicates
  for (int i = 0; i < count_; i++) {
    if (entries_[i].id == def.id) return false;
  }
  if (count_ >= MAX_EMOTIONS) return false;
  entries_[count_++] = def;
  return true;
}

// Returns a pointer to the EmotionDef for the given emotion ID, or nullptr if not found.
const EmotionDef* EmotionRegistry::get(EmotionState id) const {
  for (int i = 0; i < count_; i++) {
    if (entries_[i].id == id) return &entries_[i];
  }
  return nullptr;
}

// Returns the display name for the given emotion ID, or "UNKNOWN" if not registered.
const char* EmotionRegistry::getName(EmotionState id) const {
  const EmotionDef* def = get(id);
  return def ? def->name : "UNKNOWN";
}

// Fills out[] with all cyclable emotion IDs (excludes BLINK). Returns the number of entries written.
int EmotionRegistry::getCyclable(EmotionState* out, int maxCount) const {
  int written = 0;
  for (int i = 0; i < count_ && written < maxCount; i++) {
    if (entries_[i].id != EMOTION_BLINK) {
      out[written++] = entries_[i].id;
    }
  }
  return written;
}
