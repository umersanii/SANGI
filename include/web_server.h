#ifndef WEB_SERVER_H
#define WEB_SERVER_H

// WebServerManager — WiFi AP-mode HTTP server for local web UI.
// Serves the SANGI control page at 192.168.4.1 with no router required.
// Excluded from native builds via NATIVE_BUILD guard.

#ifndef NATIVE_BUILD

#include <Arduino.h>
#include <functional>
#include <WebServer.h>
#include "emotion.h"
#include "emotion_registry.h"
#include "input.h"
#include "battery.h"
#include "personality.h"
#include "runtime_config.h"
#include "config.h"

class WebServerManager {
public:
  using EmotionSetFn = std::function<void(EmotionState)>;
  using GestureFn    = std::function<void(TouchGesture)>;

  WebServerManager();

  // Start WiFi AP and HTTP server; register all routes.
  void init();

  // Poll for incoming HTTP clients — non-blocking, call every loop().
  void update();

  void setOnEmotionSet(EmotionSetFn fn) { onEmotionSet_ = fn; }
  void setOnGesture(GestureFn fn)       { onGesture_    = fn; }

  // Set module references before calling init().
  void setEmotionManager(EmotionManager* em) { em_  = em; }
  void setBatteryManager(BatteryManager* bm) { bm_  = bm; }
  void setInputManager(InputManager* im)     { im_  = im; }
  void setRuntimeConfig(RuntimeConfig* cfg)  { cfg_ = cfg; }
  void setPersonality(Personality* p)        { p_   = p; }

private:
  WebServer server_;

  EmotionManager* em_;
  BatteryManager* bm_;
  InputManager*   im_;
  RuntimeConfig*  cfg_;
  Personality*    p_;

  EmotionSetFn onEmotionSet_;
  GestureFn    onGesture_;

  // Route handlers — registered in init().
  void handleRoot();
  void handleApiStatus();
  void handleApiEmotion();
  void handleApiGesture();
  void handleApiConfigGet();
  void handleApiConfigPost();
  void handleApiConfigReset();
  void handleNotFound();

  // Log free heap to Serial with a context label.
  void logHeap(const char* ctx);
};

extern WebServerManager webServerManager;

#endif // NATIVE_BUILD
#endif // WEB_SERVER_H
