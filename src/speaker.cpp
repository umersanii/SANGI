#include "speaker.h"
#include "runtime_config.h"
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

static const BeepTone PATTERN_NEEDY[] = {
  {700, 150}, {0, 80},   // rising whimper
  {900, 180}, {0, 100},
  {700, 150}, {0, 80},
  {900, 220}             // held plea
};

static const BeepTone PATTERN_CONTENT[] = {
  {440, 200}, {0, 150},   // warm hum
  {460, 250}               // slightly higher, held — purr
};

static const BeepTone PATTERN_PLAYFUL[] = {
  {900, 70},  {0, 30},    // chirp
  {1100, 70}, {0, 30},    // chirp up
  {900, 70},  {0, 30},    // back down
  {1200, 120}             // inviting held note
};

static const BeepTone PATTERN_GRUMPY[] = {
  {320, 200}, {0, 60},    // low flat grumble
  {260, 280}              // lower held note — dismissive exhale
};

// ===== BEEP MANAGER IMPLEMENTATION =====

// Initializes beep state machine fields to idle; hardware setup is deferred to init().
BeepManager::BeepManager()
  : currentPattern(nullptr),
    patternLength(0),
    currentToneIndex(0),
    toneStartTime(0),
    isActive(false),
    isTonePlaying(false) {
}

static const BeepTone PATTERN_STARTUP[] = {
  {523, 100}, {0, 40},   // C5
  {659, 100}, {0, 40},   // E5
  {784, 100}, {0, 40},   // G5
  {1047, 200}            // C6
};

// Configures the LEDC PWM channel and plays the startup C major arpeggio tone sequence.
void BeepManager::init() {
  ledcSetup(SPEAKER_CHANNEL, SPEAKER_BASE_FREQ, SPEAKER_RESOLUTION);
  ledcAttachPin(SPEAKER_PIN, SPEAKER_CHANNEL);
  ledcWrite(SPEAKER_CHANNEL, 0); // Start silent

  // Startup test tone — C major arpeggio
  Serial.println("[SPEAKER] Playing startup tone...");
  for (int i = 0; i < (int)(sizeof(PATTERN_STARTUP) / sizeof(BeepTone)); i++) {
    int freq = PATTERN_STARTUP[i].frequency;
    int dur  = PATTERN_STARTUP[i].duration;
    if (freq > 0) {
      Serial.printf("[SPEAKER] tone %dHz for %dms\n", freq, dur);
      ledcWriteTone(SPEAKER_CHANNEL, freq);
      ledcWrite(SPEAKER_CHANNEL, runtimeConfig.speakerVolume);
    } else {
      Serial.printf("[SPEAKER] silence %dms\n", dur);
      ledcWrite(SPEAKER_CHANNEL, 0);
    }
    delay(dur);
  }
  ledcWrite(SPEAKER_CHANNEL, 0);
  Serial.printf("[SPEAKER] init done — GPIO %d, volume %d/255\n", SPEAKER_PIN, runtimeConfig.speakerVolume);
}

// Advances the non-blocking beep state machine; moves to the next tone when the current duration elapses.
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
      ledcWrite(SPEAKER_CHANNEL, runtimeConfig.speakerVolume); // Configurable volume to prevent power issues
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
  PATTERN_ENTRY(EMOTION_NEEDY, PATTERN_NEEDY),
  PATTERN_ENTRY(EMOTION_CONTENT, PATTERN_CONTENT),
  PATTERN_ENTRY(EMOTION_PLAYFUL, PATTERN_PLAYFUL),
  PATTERN_ENTRY(EMOTION_GRUMPY, PATTERN_GRUMPY),
};
static const int NUM_PATTERNS = sizeof(EMOTION_PATTERNS) / sizeof(EmotionPattern);

// Looks up the beep pattern for the given emotion and starts playback. No-ops if already active or BLINK.
void BeepManager::queueEmotionBeep(EmotionState emotion) {
  if (isActive) return;
  if (emotion == EMOTION_BLINK) return;  // No sound for blink

  for (int i = 0; i < NUM_PATTERNS; i++) {
    if (EMOTION_PATTERNS[i].emotion == emotion) {
      Serial.printf("[SPEAKER] beep for emotion %d (%d tones)\n", emotion, EMOTION_PATTERNS[i].length);
      startBeep(EMOTION_PATTERNS[i].pattern, EMOTION_PATTERNS[i].length);
      return;
    }
  }
  // Fallback to idle pattern
  Serial.printf("[SPEAKER] no pattern for emotion %d, using idle fallback\n", emotion);
  startBeep(PATTERN_IDLE, sizeof(PATTERN_IDLE) / sizeof(BeepTone));
}

// Begins playback of the given tone pattern; starts the first tone immediately.
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
    ledcWrite(SPEAKER_CHANNEL, runtimeConfig.speakerVolume); // Configurable volume to prevent power issues
    isTonePlaying = true;
  } else {
    ledcWrite(SPEAKER_CHANNEL, 0);
    isTonePlaying = false;
  }
}

// Stops all PWM output and resets the beep state machine to idle.
void BeepManager::stopCurrentBeep() {
  ledcWrite(SPEAKER_CHANNEL, 0);
  currentPattern = nullptr;
  patternLength = 0;
  currentToneIndex = 0;
  isActive = false;
  isTonePlaying = false;
}
