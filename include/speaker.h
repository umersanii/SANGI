#ifndef SPEAKER_H
#define SPEAKER_H

#include "config.h"
#include "emotion.h"

// ===== BEEP TONE STRUCTURE =====
struct BeepTone {
  int frequency;
  int duration;
};

// ===== NON-BLOCKING BEEP MANAGER =====
class BeepManager {
public:
  BeepManager();
  void init();
  void update();  // Call in main loop
  void queueEmotionBeep(EmotionState emotion);
  bool isPlaying() const { return isActive; }
  
private:
  void startBeep(const BeepTone* pattern, int patternLength);
  void stopCurrentBeep();
  
  const BeepTone* currentPattern;
  int patternLength;
  int currentToneIndex;
  unsigned long toneStartTime;
  bool isActive;
  bool isTonePlaying;
};

extern BeepManager beepManager;

#endif // SPEAKER_H
