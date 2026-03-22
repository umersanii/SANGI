# How to Add a New Emotion to SANGI

After the Phase 2 refactoring, adding a new emotion requires changes
in exactly **2 files** and **under 10 lines of setup code** (plus your
animation art).

## Step 1: Add the enum value

**File: `include/emotion.h`**

Add your emotion before `EMOTION_GITHUB_STATS`:

```cpp
  EMOTION_CODING,
  EMOTION_GITHUB_STATS,
  EMOTION_DIZZY,          // <-- add here
```

## Step 2: Write the draw function

**File: `src/emotion_draws.cpp`**

```cpp
void drawDizzy(ICanvas& canvas, int frame, const void* ctx) {
  // Spiral eyes + wobbling
  int wobble = (frame % 10) - 5;
  canvas.drawEyes(40 + wobble, 28, 88 - wobble, 28, 18);
  canvas.drawCircle(40 + wobble, 28, 6, COLOR_WHITE);
  canvas.drawCircle(88 - wobble, 28, 6, COLOR_WHITE);
  canvas.drawMouth(52, 50, 24, 5);
}
```

## Step 3: Register it

**File: `src/main.cpp`** — in `registerEmotions()`:

```cpp
emotionRegistry.add({EMOTION_DIZZY, "DIZZY", 51, 30, LOOP_RESTART, true, drawDizzy});
```

## Step 4 (optional): Add a beep pattern

**File: `src/speaker.cpp`** — add to the lookup table:

```cpp
static const BeepTone PATTERN_DIZZY[] = {{800,80},{600,80},{400,80}};
// ...
{EMOTION_DIZZY, PATTERN_DIZZY, 3},
```

## That's it.

No switch statements to update. No frame variables to declare.
No display.cpp changes. No animations.h changes.
The registry, tick engine, and ICanvas handle everything.
