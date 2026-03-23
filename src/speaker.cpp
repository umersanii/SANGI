#include "speaker.h"
#include <Arduino.h>

BeepManager beepManager;

// ===== EMOTION-SPECIFIC BEEP PATTERNS =====
// Each pattern is an array of {frequency, duration} pairs

static const BeepTone PATTERN_IDLE[] = {
  {800, 150}
};

static const BeepTone PATTERN_HAPPY[] = {
  {600, 60}, {0, 30},   // 0 = silence
  {900, 60}, {0, 30},
  {1200, 100}
};

static const BeepTone PATTERN_SLEEPY[] = {
  {700, 200}, {0, 100},
  {500, 250}, {0, 100},
  {300, 300}
};

static const BeepTone PATTERN_EXCITED[] = {
  {1400, 40}, {0, 40},
  {1400, 40}, {0, 40},
  {1400, 40}, {0, 40},
  {1400, 40}, {0, 40},
  {1600, 120}
};

static const BeepTone PATTERN_SAD[] = {
  {600, 180}, {0, 80},
  {450, 200}, {0, 80},
  {350, 250}
};

static const BeepTone PATTERN_ANGRY[] = {
  {1800, 50}, {0, 30},
  {1600, 50}, {0, 30},
  {1800, 50}, {0, 30},
  {1600, 50}, {0, 30},
  {1800, 50}, {0, 30},
  {1600, 50}, {0, 30},
  {1800, 50}, {0, 30},
  {1600, 50}, {0, 30},
  {1800, 50}, {0, 30},
  {1600, 50}
};

static const BeepTone PATTERN_CONFUSED[] = {
  {700, 100}, {0, 50},
  {900, 80}, {0, 50},
  {600, 100}, {0, 50},
  {800, 80}
};

static const BeepTone PATTERN_THINKING[] = {
  {1000, 100}, {0, 150},
  {1000, 100}, {0, 150},
  {1000, 100}
};

static const BeepTone PATTERN_LOVE[] = {
  {880, 120}, {0, 50},   // A5
  {1047, 120}, {0, 50},  // C6
  {1319, 200}, {0, 50},  // E6
  {1047, 150}            // C6
};

static const BeepTone PATTERN_SURPRISED[] = {
  {400, 50}, {0, 20},
  {800, 50}, {0, 20},
  {1400, 50}, {0, 20},
  {1800, 120}
};

static const BeepTone PATTERN_DEAD[] = {
  {800, 150}, {0, 50},
  {600, 150}, {0, 50},
  {400, 150}, {0, 50},
  {200, 300}
};

static const BeepTone PATTERN_BORED[] = {
  {400, 300}, {0, 200}, {300, 400}
};

// ===== BEEP MANAGER IMPLEMENTATION =====

BeepManager::BeepManager()
  : currentPattern(nullptr),
    patternLength(0),
    currentToneIndex(0),
    toneStartTime(0),
    isActive(false),
    isTonePlaying(false) {
}

void BeepManager::init() {
  // Configure PWM for speaker on GPIO 9
  ledcSetup(SPEAKER_CHANNEL, SPEAKER_BASE_FREQ, SPEAKER_RESOLUTION);
  ledcAttachPin(SPEAKER_PIN, SPEAKER_CHANNEL);
  ledcWrite(SPEAKER_CHANNEL, 0); // Start silent
  Serial.println("🔊 BeepManager initialized on GPIO 9");
}

void BeepManager::update() {
  if (!isActive || currentPattern == nullptr) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // Handle millis() overflow (49 days)
  if (currentTime < toneStartTime) {
    toneStartTime = currentTime;
    return;
  }
  
  // Check if current tone duration has elapsed
  if (currentTime - toneStartTime >= (unsigned long)currentPattern[currentToneIndex].duration) {
    // Stop current tone
    ledcWrite(SPEAKER_CHANNEL, 0);
    
    // Move to next tone
    currentToneIndex++;
    
    // Check if pattern is complete
    if (currentToneIndex >= patternLength) {
      stopCurrentBeep();
      return;
    }
    
    // Start next tone
    int frequency = currentPattern[currentToneIndex].frequency;
    if (frequency > 0) {
      ledcWriteTone(SPEAKER_CHANNEL, frequency);
      ledcWrite(SPEAKER_CHANNEL, SPEAKER_VOLUME); // Configurable volume to prevent power issues
      isTonePlaying = true;
    } else {
      // Silence (frequency = 0)
      ledcWrite(SPEAKER_CHANNEL, 0);
      isTonePlaying = false;
    }
    
    toneStartTime = currentTime;
  }
}

// Lookup table replaces the 14-case switch
struct EmotionPattern {
  EmotionState emotion;
  const BeepTone* pattern;
  int length;
};

#define PATTERN_ENTRY(e, p) {e, p, sizeof(p) / sizeof(BeepTone)}

static const EmotionPattern EMOTION_PATTERNS[] = {
  PATTERN_ENTRY(EMOTION_IDLE, PATTERN_IDLE),
  PATTERN_ENTRY(EMOTION_HAPPY, PATTERN_HAPPY),
  PATTERN_ENTRY(EMOTION_SLEEPY, PATTERN_SLEEPY),
  PATTERN_ENTRY(EMOTION_EXCITED, PATTERN_EXCITED),
  PATTERN_ENTRY(EMOTION_SAD, PATTERN_SAD),
  PATTERN_ENTRY(EMOTION_ANGRY, PATTERN_ANGRY),
  PATTERN_ENTRY(EMOTION_CONFUSED, PATTERN_CONFUSED),
  PATTERN_ENTRY(EMOTION_THINKING, PATTERN_THINKING),
  PATTERN_ENTRY(EMOTION_LOVE, PATTERN_LOVE),
  PATTERN_ENTRY(EMOTION_SURPRISED, PATTERN_SURPRISED),
  PATTERN_ENTRY(EMOTION_DEAD, PATTERN_DEAD),
  PATTERN_ENTRY(EMOTION_BORED, PATTERN_BORED),
};
static const int NUM_PATTERNS = sizeof(EMOTION_PATTERNS) / sizeof(EmotionPattern);

void BeepManager::queueEmotionBeep(EmotionState emotion) {
  if (isActive) return;
  if (emotion == EMOTION_BLINK) return;  // No sound for blink

  for (int i = 0; i < NUM_PATTERNS; i++) {
    if (EMOTION_PATTERNS[i].emotion == emotion) {
      startBeep(EMOTION_PATTERNS[i].pattern, EMOTION_PATTERNS[i].length);
      return;
    }
  }
  // Fallback to idle pattern
  startBeep(PATTERN_IDLE, sizeof(PATTERN_IDLE) / sizeof(BeepTone));
}

void BeepManager::startBeep(const BeepTone* pattern, int patternLength) {
  currentPattern = pattern;
  this->patternLength = patternLength;
  currentToneIndex = 0;
  toneStartTime = millis();
  isActive = true;
  
  // Start first tone
  int frequency = currentPattern[0].frequency;
  if (frequency > 0) {
    ledcWriteTone(SPEAKER_CHANNEL, frequency);
    ledcWrite(SPEAKER_CHANNEL, SPEAKER_VOLUME); // Configurable volume to prevent power issues
    isTonePlaying = true;
  } else {
    ledcWrite(SPEAKER_CHANNEL, 0);
    isTonePlaying = false;
  }
}

void BeepManager::stopCurrentBeep() {
  ledcWrite(SPEAKER_CHANNEL, 0);
  currentPattern = nullptr;
  patternLength = 0;
  currentToneIndex = 0;
  isActive = false;
  isTonePlaying = false;
}
