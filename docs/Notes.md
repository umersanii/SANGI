## for myself
- ~~emotions are way too short in duration~~
- ~~comments like jsdoc at the start of function to tell exactly what they do~~
~~- angry emotion rework
- thinking emotion rework~~
-~~ transition for the dead emotion at the start~~
- ~~the webportal changes~~
- too much of emotion changes can feel very difficult fthe webportal changesor user to manage

## for future
-- mimi claw on esp32?
- 3 am SANGI randomly does things and goes back to sleep

## AI personality engine (explored 2026-03-29)

**Core idea:** Replace the rule-based `PersonalityEngine` with a small trained model so SANGI has the *texture* of a personality, not just the *rules* of one.

**What the current engine lacks:**
- Transitions are instant on timer fire (always BORED at exactly 5min)
- Mood drift is dice rolls, not context-aware
- Two SANGIs with the same firmware behave identically

**What a model would give:**
- Emotional inertia — resists snapping between states
- Context sensitivity — sees full state, not just one timer
- Individuality — retrain on real usage logs, each SANGI diverges over time

**Hardware constraints (ESP32-C3 with WiFi+BLE active):**
- ~80–130 KB free heap after stacks
- Model must fit in that headroom

**Options researched:**

| Option | Flash | RAM | Effort | Notes |
|--------|-------|-----|--------|-------|
| micromlgen Random Forest | ~6 KB | ~300 B | Low | Single `.h` file, zero deps — **best start** |
| emlearn MLP | ~10 KB | ~500 B | Low | Same workflow, neural net |
| EloquentTinyML (TFLite) | ~60 KB + 5 KB model | ~8 KB | Medium | PlatformIO compatible |
| Q-table RL (const flash) | 16–32 KB | ~50 B | Medium | Learned offline, just array lookup |
| RLtools deep RL | ~50 KB | ~10 KB | High | Benchmarked on C3 at 7ms/inference |

**Recommended path:**
1. Generate synthetic training data by simulating current `PersonalityEngine` in Python (~10k samples)
2. Train `RandomForestClassifier(n_estimators=10, max_depth=6)` via sklearn
3. Export with `micromlgen` → single `emotion_model.h` (~6 KB flash, ~300 B RAM)
4. Replace `personality.cpp`'s tick decision logic with `predict(features)`

**Input features:** `time_idle_ms` (norm), `current_emotion` (0–13), `interaction_count` (norm), `time_of_day_bucket` (0–3), `battery_level` (0–1)

**Tradeoff to watch:** Attention arc narrative (BORED→SAD→CONFUSED→ANGRY) needs to be deliberately encoded in training data or kept as a hard constraint — a naive model won't preserve it.