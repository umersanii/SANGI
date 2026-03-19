// Minimal Arduino API stub for native testing.
// Provides just enough to compile emotion.cpp, emotion_registry.cpp,
// notification_queue.cpp, github_data.cpp, and animations.cpp on desktop.

#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

// Types
typedef uint8_t byte;

// Timing — shared across translation units via inline function holding a static
inline unsigned long& _stubMillisRef() {
  static unsigned long val = 0;
  return val;
}
inline unsigned long millis() { return _stubMillisRef(); }
inline void stubSetMillis(unsigned long ms) { _stubMillisRef() = ms; }
inline void delay(unsigned long) {}

// Random
inline long random(long min, long max) {
  if (min >= max) return min;
  return min + (rand() % (max - min));
}
inline void randomSeed(unsigned long) {}

// GPIO (no-ops)
#define INPUT_PULLUP 0x05
#define LOW 0
#define HIGH 1
inline void pinMode(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t) { return HIGH; }
inline int analogRead(uint8_t) { return 0; }

// Serial stub
struct SerialStub {
  void begin(int) {}
  void print(const char* s) { if (s) fputs(s, stdout); }
  void print(int v) { printf("%d", v); }
  void println(const char* s = "") { if (s) puts(s); else puts(""); }
  void println(int v) { printf("%d\n", v); }
  void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
  void flush() { fflush(stdout); }
};

// Single global instance — matches Arduino's extern Serial
inline SerialStub& getSerial() {
  static SerialStub instance;
  return instance;
}
#define Serial (getSerial())

// PROGMEM (no-op on native)
#define PROGMEM

// String class (minimal stub for voltage formatting)
class String {
public:
  String(float val, int decimals) {
    snprintf(buf_, sizeof(buf_), "%.*f", decimals, (double)val);
  }
  float toFloat() const { return (float)atof(buf_); }
private:
  char buf_[32];
};

#endif // ARDUINO_H
