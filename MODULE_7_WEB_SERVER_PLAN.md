# Module 7: WiFi AP Web Server — Implementation Plan

## Context

SANGI has no WiFi or web interface yet. This adds a self-hosted web UI served from the ESP32-C3 itself in Access Point mode — the device broadcasts its own "SANGI" WiFi network and serves the UI at `192.168.4.1`. No router, no internet, no dependencies. The user can connect their phone and control SANGI from a browser even if there's no external WiFi.

Primary constraints: ESP32-C3 has 400KB RAM. NimBLE already uses ~200–300KB. WiFi stack takes another ~80–100KB. This is tight — every phase includes a heap gate before proceeding.

---

## Files to Create

| File | Purpose |
|------|---------|
| `include/web_server.h` | `WebServerManager` class interface |
| `src/web_server.cpp` | AP init, route handlers, JSON responses |
| `include/web_ui.h` | Full HTML/CSS/JS in PROGMEM (Phase 5 only) |

## Files to Modify

| File | Change |
|------|--------|
| `include/config.h` | Add `WIFI_AP_SSID`, `WIFI_SERVER_PORT`, `WEB_MIN_FREE_HEAP` |
| `platformio.ini` | Add `-DCONFIG_BT_NIMBLE_MAX_CONNECTIONS=1` build flag to hardware env |
| `src/main.cpp` | Include header, wire module refs + callbacks, call `init()`/`update()` |

**Note:** `platformio.ini` native env already excludes all unlisted `src/` files via `build_src_filter`. The hardware env has no filter (includes all `src/`). `web_server.cpp` is hardware-only — wrap everything in `#ifndef NATIVE_BUILD` (already defined as `-DNATIVE_BUILD=1` in native env).

---

## Module Interface (`web_server.h`)

```cpp
#ifndef NATIVE_BUILD
class WebServerManager {
public:
  using EmotionSetFn = std::function<void(EmotionState)>;
  using GestureFn    = std::function<void(TouchGesture)>;

  WebServerManager();         // value-initializes WebServer(WIFI_SERVER_PORT)
  void init();                // start AP + register routes + server.begin()
  void update();              // server.handleClient() — non-blocking

  void setOnEmotionSet(EmotionSetFn fn);
  void setOnGesture(GestureFn fn);

  // Set module refs before calling init()
  void setEmotionManager(EmotionManager* em);
  void setBatteryManager(BatteryManager* bm);
  void setInputManager(InputManager* im);
  void setRuntimeConfig(RuntimeConfig* cfg);
  void setPersonality(Personality* p);

private:
  WebServer server_;          // value member, not pointer (no malloc)
  EmotionManager*  em_;
  BatteryManager*  bm_;
  InputManager*    im_;
  RuntimeConfig*   cfg_;
  Personality*     p_;
  EmotionSetFn onEmotionSet_;
  GestureFn    onGesture_;

  void handleRoot();
  void handleApiStatus();
  void handleApiEmotion();
  void handleApiGesture();
  void handleApiConfigGet();
  void handleApiConfigPost();
  void handleApiConfigReset();
  void handleNotFound();
  void logHeap(const char* ctx);
};
extern WebServerManager webServerManager;
#endif
```

---

## 5-Phase Modular Plan

### Phase 1 — AP Mode + Minimal Response

**Add to `config.h`:**
```cpp
#define WIFI_AP_SSID        "SANGI"
#define WIFI_AP_CHANNEL     1         // channel 1 = minimal BLE overlap
#define WIFI_SERVER_PORT    80
#define WEB_MIN_FREE_HEAP   51200     // 50KB — log warning if breached
```

**Add to `platformio.ini` `build_flags`** (hardware env only):
```ini
-DCONFIG_BT_NIMBLE_MAX_CONNECTIONS=1   ; reduces NimBLE RAM, SANGI only needs 1 BLE connection
```

**`web_server.cpp` — `init()`:**
```cpp
logHeap("pre-WiFi");
WiFi.mode(WIFI_AP);
WiFi.softAP(WIFI_AP_SSID, nullptr, WIFI_AP_CHANNEL);  // open network
Serial.printf("[WEB] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
logHeap("post-WiFi");
if (ESP.getFreeHeap() < WEB_MIN_FREE_HEAP)
    Serial.printf("[WEB] WARNING: heap below threshold\n");
server_.on("/", [this]() { handleRoot(); });
server_.onNotFound([this]() { handleNotFound(); });
server_.begin();
```

**`handleRoot()`** returns `"SANGI OK"` as `text/plain` (placeholder until Phase 5).

**`main.cpp` wiring** (after `bleControl.init()`):
```cpp
webServerManager.setEmotionManager(&emotionManager);
webServerManager.setBatteryManager(&batteryManager);
webServerManager.setInputManager(&inputManager);
webServerManager.setRuntimeConfig(&runtimeConfig);
webServerManager.setPersonality(&personality);
webServerManager.init();
```

In `loop()` after `beepManager.update()`:
```cpp
webServerManager.update();
```

**Gate:** Connect phone to "SANGI" WiFi → `http://192.168.4.1` shows "SANGI OK". Serial shows `[WEB] Free heap (post-WiFi): XXXX`. BLE advertising still visible in nRF Connect. **Heap must be > 50KB to proceed.**

---

### Phase 2 — JSON Status API

**Route:** `GET /api/status`

Hand-craft JSON into a 256-byte stack buffer via `snprintf` — no ArduinoJson.

Uses existing APIs:
- `em_->getCurrentEmotion()` → int
- `emotionRegistry.getName(emotion)` → `const char*` (`include/emotion_registry.h:49`)
- `bm_->readVoltage()` → float (`include/battery.h:14`)
- `p_->getAttentionStage()` → int 0–4 (`include/personality.h:23`)
- `ESP.getFreeHeap()` → uint32_t

Response shape:
```json
{"emotion":1,"emotionName":"HAPPY","batteryVoltage":3.85,"uptimeMs":12430,"attentionStage":0,"freeHeap":52480}
```

Add `Access-Control-Allow-Origin: *` header on every API response.

**Gate:** `curl http://192.168.4.1/api/status | python3 -m json.tool` returns valid JSON with all 6 fields. Heap stable across 10 repeated polls.

---

### Phase 3 — Control API

**Routes:**
- `POST /api/emotion` — body: `emotion=N` (0–12). Validates via `emotionRegistry.get((EmotionState)id) != nullptr` (same check as BLE). Calls `onEmotionSet_` callback.
- `POST /api/gesture` — body: `gesture=tap|long|double`. Maps to `GESTURE_TAP`, `GESTURE_LONG_PRESS`, `GESTURE_DOUBLE_TAP`. Calls `onGesture_` callback.
- `POST /api/config/reset` — calls `runtimeConfigReset()`.

**`main.cpp` callback wiring** (after `webServerManager.init()`):
```cpp
webServerManager.setOnEmotionSet([](EmotionState e) {
    emotionManager.setTargetEmotion(e);
    Serial.printf("[WEB] emotion → %s\n", emotionRegistry.getName(e));
});
webServerManager.setOnGesture([](TouchGesture g) {
    onGesture(g, millis());   // reuse existing gesture handler
});
```

**Gate:** `curl -X POST http://192.168.4.1/api/emotion -d "emotion=1"` → OLED transitions to HAPPY.

---

### Phase 4 — Config Read/Write API

**Routes:**
- `GET /api/config` — returns all 7 `RuntimeConfig` fields as JSON (320-byte stack buffer).
- `POST /api/config` — accepts any subset of fields as form params, updates `*cfg_` fields, calls `runtimeConfigSave()`.

Bounds enforcement: `microExpressionChance` clamped to 0–100, `jitterPercent` clamped to 0–50.

**Gate:**
1. `curl -X POST http://192.168.4.1/api/config -d "attentionStage1Ms=60000"` — value changes.
2. Power-cycle, re-read config — NVS value survives reboot.
3. `POST /api/config/reset` — returns to defaults.

---

### Phase 5 — Full HTML UI

**Storage:** `include/web_ui.h` stores the complete page as:
```cpp
const char WEB_UI_HTML[] PROGMEM = R"rawhtml(<!DOCTYPE html>...)rawhtml";
```

**Served via:** `server_.send_P(200, "text/html", WEB_UI_HTML)` — reads directly from flash, zero RAM copy.

**Target size:** < 8KB total (fits in one TCP segment on AP connection).

**UI sections:**

1. **Status bar** — `emotion | voltage (%) | uptime | heap | stage` — auto-refresh every 3s via `setInterval(pollStatus, 3000)` calling `fetch('/api/status')`.

2. **Attention arc bar** — 5-segment CSS div labeled `NORMAL → BORED → SAD → CONFUSED → ANGRY`. JS highlights the active segment from `attentionStage` (0–4).

3. **Emotion grid** — 4-column CSS grid, 13 buttons in `EmotionState` enum order (IDLE→BLINK). Each POSTs `emotion=N`. Active emotion button highlighted via status poll.

4. **Gesture row** — 3 buttons: TAP / LONG PRESS / DOUBLE TAP → `POST /api/gesture`.

5. **Config panel** — `<details><summary>` collapsible (no JS needed for expand/collapse). Inside: 7 labeled `<input type="range">` sliders with live ms readout. Save button POSTs all 7 fields. Reset button POSTs to `/api/config/reset` then reloads sliders.

**CSS theme:** Black background, `#00ff41` (matrix green) text, monospace font. `max-width: 480px` centered — optimized for phone. Total CSS: ~80 lines. Total JS: ~100 lines vanilla.

**Gate:**
1. Open on mobile browser — all sections render.
2. Tap emotion button — OLED changes within 1s.
3. Adjust slider, Save — persists across reboot.
4. `freeHeap` in status bar remains > 50000 during active use.
5. Page load < 500ms (check with browser DevTools Network tab).

---

## RAM Budget

| Component | Estimated RAM |
|-----------|--------------|
| NimBLE (with MAX_CONNECTIONS=1) | ~150–200KB |
| WiFi AP stack | ~80–100KB |
| WebServer library (8 routes + buffers) | ~10–14KB |
| HTML in PROGMEM | 0 bytes RAM |
| Per-request stack buffers (256–320 bytes) | ~320B peak, freed after handler returns |
| **Worst case total** | **~315KB / 400KB** |

Free heap target post-init: **> 85KB** (healthy). 50KB is the hard stop gate.

---

## BLE + WiFi Coexistence Notes

- ESP32-C3 uses time-division multiplexing — both run simultaneously on the shared radio.
- Expected: BLE scan/connect latency increases slightly during WiFi page loads. Acceptable for SANGI's use case.
- `bleControl.updateCurrentEmotion()` runs every loop tick (~50ms) — interleaves safely with `handleClient()`.
- Test at Phase 3: connect nRF Connect + curl simultaneously, send emotions from both — verify neither drops.
- If heap < 50KB after Phase 1, add `-DCONFIG_BT_NIMBLE_ROLE_OBSERVER=0` to disable BLE scanning (SANGI only advertises, never scans).

---

## Diagnostics

Extend the existing 10-second Serial diagnostic block in `main.cpp`:
```cpp
Serial.printf("[WEB] heap: %u | clients: %d | AP: %s\n",
    ESP.getFreeHeap(),
    WiFi.softAPgetStationNum(),
    WiFi.softAPIP().toString().c_str());
```

---

## Test Commands

```bash
# Build & upload
~/.platformio/penv/bin/pio run --target upload

# Monitor serial (heap logs, AP IP, emotion changes)
~/.platformio/penv/bin/pio device monitor --port /dev/ttyACM0 --baud 115200

# Native tests must still pass (web_server.cpp excluded from native)
~/.platformio/penv/bin/pio test -e native

# Phase 2 gate
curl http://192.168.4.1/api/status | python3 -m json.tool

# Phase 3 gate
curl -X POST http://192.168.4.1/api/emotion -d "emotion=1"
curl -X POST http://192.168.4.1/api/gesture -d "gesture=long"

# Phase 4 gate
curl -X POST http://192.168.4.1/api/config -d "attentionStage1Ms=60000"
curl http://192.168.4.1/api/config
curl -X POST http://192.168.4.1/api/config/reset
```
