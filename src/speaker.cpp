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

static const BeepTone PATTERN_MUSIC[] = {
  {523, 100}, {0, 50},   // C5
  {659, 100}, {0, 50},   // E5
  {784, 100}, {0, 50},   // G5
  {1047, 150}            // C6
};

static const BeepTone PATTERN_NOTIFICATION[] = {
  {1200, 80}, {0, 60},
  {1200, 80}, {0, 60},
  {1500, 150}
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

void BeepManager::queueEmotionBeep(EmotionState emotion) {
  // If already playing, let it finish (no interruption)
  // You can change this behavior to interrupt if needed
  if (isActive) {
    return;
  }
  
  const BeepTone* pattern = nullptr;
  int length = 0;
  
  switch(emotion) {
    case EMOTION_IDLE:
      pattern = PATTERN_IDLE;
      length = sizeof(PATTERN_IDLE) / sizeof(BeepTone);
      break;
    case EMOTION_HAPPY:
      pattern = PATTERN_HAPPY;
      length = sizeof(PATTERN_HAPPY) / sizeof(BeepTone);
      break;
    case EMOTION_SLEEPY:
      pattern = PATTERN_SLEEPY;
      length = sizeof(PATTERN_SLEEPY) / sizeof(BeepTone);
      break;
    case EMOTION_EXCITED:
      pattern = PATTERN_EXCITED;
      length = sizeof(PATTERN_EXCITED) / sizeof(BeepTone);
      break;
    case EMOTION_SAD:
      pattern = PATTERN_SAD;
      length = sizeof(PATTERN_SAD) / sizeof(BeepTone);
      break;
    case EMOTION_ANGRY:
      pattern = PATTERN_ANGRY;
      length = sizeof(PATTERN_ANGRY) / sizeof(BeepTone);
      break;
    case EMOTION_CONFUSED:
      pattern = PATTERN_CONFUSED;
      length = sizeof(PATTERN_CONFUSED) / sizeof(BeepTone);
      break;
    case EMOTION_THINKING:
      pattern = PATTERN_THINKING;
      length = sizeof(PATTERN_THINKING) / sizeof(BeepTone);
      break;
    case EMOTION_LOVE:
      pattern = PATTERN_LOVE;
      length = sizeof(PATTERN_LOVE) / sizeof(BeepTone);
      break;
    case EMOTION_SURPRISED:
      pattern = PATTERN_SURPRISED;
      length = sizeof(PATTERN_SURPRISED) / sizeof(BeepTone);
      break;
    case EMOTION_DEAD:
      pattern = PATTERN_DEAD;
      length = sizeof(PATTERN_DEAD) / sizeof(BeepTone);
      break;
    case EMOTION_MUSIC:
      pattern = PATTERN_MUSIC;
      length = sizeof(PATTERN_MUSIC) / sizeof(BeepTone);
      break;
    case EMOTION_NOTIFICATION:
      pattern = PATTERN_NOTIFICATION;
      length = sizeof(PATTERN_NOTIFICATION) / sizeof(BeepTone);
      break;
    case EMOTION_BLINK:
      // No sound for blink
      return;
    default:
      pattern = PATTERN_IDLE;
      length = sizeof(PATTERN_IDLE) / sizeof(BeepTone);
      break;
  }
  
  startBeep(pattern, length);
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
