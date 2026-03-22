// SANGI Robot — Native Unit Tests
// Tests for emotion management, emotion registry, animation tick engine,
// and draw functions via MockCanvas.
//
// Run with: pio test -e native

#include <unity.h>
#include "emotion.h"
#include "emotion_registry.h"
#include "emotion_draws.h"
#include "animations.h"
#include "input.h"
#include "personality.h"
#include "mock_canvas.h"

// ===== TEST HELPERS =====
static EmotionState lastCompletedEmotion = EMOTION_IDLE;
static EmotionState lastChangeFrom = EMOTION_IDLE;
static EmotionState lastChangeTo = EMOTION_IDLE;

void stubTransitionComplete(EmotionState e) { lastCompletedEmotion = e; }
void stubEmotionChange(EmotionState f, EmotionState t) {
  lastChangeFrom = f;
  lastChangeTo = t;
}

static void registerTestEmotions() {
  emotionRegistry = EmotionRegistry();
  emotionRegistry.add({EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, drawIdle});
  emotionRegistry.add({EMOTION_BLINK, "BLINK", 1, 0, LOOP_RESTART, false, drawBlink});
  emotionRegistry.add({EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, drawHappy});
  emotionRegistry.add({EMOTION_SLEEPY, "SLEEPY", 51, 30, LOOP_RESTART, false, drawSleepy});
  emotionRegistry.add({EMOTION_EXCITED, "EXCITED", 51, 30, LOOP_RESTART, true, drawExcited});
  emotionRegistry.add({EMOTION_SAD, "SAD", 51, 30, LOOP_RESTART, true, drawSad});
  emotionRegistry.add({EMOTION_ANGRY, "ANGRY", 51, 30, LOOP_RESTART, true, drawAngry});
  emotionRegistry.add({EMOTION_CONFUSED, "CONFUSED", 51, 30, LOOP_RESTART, true, drawConfused});
  emotionRegistry.add({EMOTION_THINKING, "THINKING", 51, 30, LOOP_RESTART, true, drawThinking});
  emotionRegistry.add({EMOTION_LOVE, "LOVE", 51, 30, LOOP_RESTART, true, drawLove});
  emotionRegistry.add({EMOTION_SURPRISED, "SURPRISED", 51, 30, LOOP_RESTART, true, drawSurprised});
  emotionRegistry.add({EMOTION_DEAD, "DEAD", 51, 30, LOOP_RESTART, false, drawDead});
  emotionRegistry.add({EMOTION_BORED, "BORED", 51, 60, LOOP_RESTART, true, drawBored});
  emotionRegistry.add({EMOTION_SHY, "SHY", 30, 40, LOOP_ONCE, false, drawShy});
}

void setUp() {
  stubSetMillis(0);
  registerTestEmotions();
  emotionManager.init(0);
  emotionManager.setOnTransitionComplete(stubTransitionComplete);
  emotionManager.setOnEmotionChange(stubEmotionChange);
  lastCompletedEmotion = EMOTION_IDLE;
  lastChangeFrom = EMOTION_IDLE;
  lastChangeTo = EMOTION_IDLE;
}

void tearDown() {}

// ===== EMOTION MANAGER TESTS =====

void test_initial_state_is_idle() {
  TEST_ASSERT_EQUAL(EMOTION_IDLE, emotionManager.getCurrentEmotion());
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
}

void test_set_target_starts_transition() {
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  TEST_ASSERT_TRUE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(0, emotionManager.getTransitionFrame());
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, emotionManager.getTargetEmotion());
  // Current hasn't changed yet
  TEST_ASSERT_EQUAL(EMOTION_IDLE, emotionManager.getCurrentEmotion());
}

void test_advance_increments_frame() {
  emotionManager.setTargetEmotion(EMOTION_SAD);
  emotionManager.advanceTransition();
  TEST_ASSERT_EQUAL(1, emotionManager.getTransitionFrame());
  emotionManager.advanceTransition();
  TEST_ASSERT_EQUAL(2, emotionManager.getTransitionFrame());
}

void test_complete_sets_current_and_fires_callback() {
  emotionManager.setTargetEmotion(EMOTION_EXCITED);
  emotionManager.completeTransition();
  TEST_ASSERT_EQUAL(EMOTION_EXCITED, emotionManager.getCurrentEmotion());
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(EMOTION_EXCITED, lastCompletedEmotion);
}

void test_same_emotion_does_not_transition() {
  emotionManager.setTargetEmotion(EMOTION_IDLE);
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
}

void test_invalid_emotion_rejected() {
  emotionManager.setTargetEmotion((EmotionState)99);
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(EMOTION_IDLE, emotionManager.getCurrentEmotion());
}

void test_change_callback_receives_from_and_to() {
  emotionManager.setTargetEmotion(EMOTION_LOVE);
  TEST_ASSERT_EQUAL(EMOTION_IDLE, lastChangeFrom);
  TEST_ASSERT_EQUAL(EMOTION_LOVE, lastChangeTo);
}

void test_previous_emotion_tracks_history() {
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  emotionManager.completeTransition();
  emotionManager.setTargetEmotion(EMOTION_SAD);
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, emotionManager.getPreviousEmotion());
}

void test_transition_frame_advance_and_complete() {
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  TEST_ASSERT_EQUAL(0, emotionManager.getTransitionFrame());

  for (int i = 1; i <= 6; i++) {
    emotionManager.advanceTransition();
    TEST_ASSERT_EQUAL(i, emotionManager.getTransitionFrame());
  }

  emotionManager.completeTransition();
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  TEST_ASSERT_EQUAL(0, emotionManager.getTransitionFrame());
}

void test_emotion_validation_uses_registry() {
  // Unregistered value rejected
  emotionManager.setTargetEmotion((EmotionState)99);
  TEST_ASSERT_FALSE(emotionManager.isTransitionActive());
  // Valid registered emotion works
  emotionManager.setTargetEmotion(EMOTION_HAPPY);
  TEST_ASSERT_TRUE(emotionManager.isTransitionActive());
}

// ===== BLE VALIDATION TEST (logic only — NimBLE not available natively) =====

void test_ble_emotion_validation_rejects_unregistered() {
  EmotionState invalid = (EmotionState)99;
  TEST_ASSERT_NULL(emotionRegistry.get(invalid));
  TEST_ASSERT_NOT_NULL(emotionRegistry.get(EMOTION_HAPPY));
}

// ===== EMOTION REGISTRY TESTS =====

void test_registry_add_and_get() {
  EmotionRegistry reg;
  EmotionDef def = {EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr};
  TEST_ASSERT_TRUE(reg.add(def));
  const EmotionDef* got = reg.get(EMOTION_IDLE);
  TEST_ASSERT_NOT_NULL(got);
  TEST_ASSERT_EQUAL_STRING("IDLE", got->name);
}

void test_registry_rejects_duplicate() {
  EmotionRegistry reg;
  EmotionDef def = {EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr};
  reg.add(def);
  TEST_ASSERT_FALSE(reg.add(def));
}

void test_registry_get_name_unknown() {
  EmotionRegistry reg;
  TEST_ASSERT_EQUAL_STRING("UNKNOWN", reg.getName(EMOTION_HAPPY));
}

void test_registry_cyclable_excludes_blink() {
  EmotionRegistry reg;
  reg.add({EMOTION_IDLE, "IDLE", 1, 0, LOOP_RESTART, true, nullptr});
  reg.add({EMOTION_BLINK, "BLINK", 1, 0, LOOP_RESTART, false, nullptr});
  reg.add({EMOTION_HAPPY, "HAPPY", 51, 30, LOOP_RESTART, true, nullptr});

  EmotionState out[10];
  int n = reg.getCyclable(out, 10);
  TEST_ASSERT_EQUAL(2, n);
  TEST_ASSERT_EQUAL(EMOTION_IDLE, out[0]);
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, out[1]);
}

void test_registry_count() {
  // Global registry populated by setUp — 14 emotions
  TEST_ASSERT_EQUAL(14, emotionRegistry.count());
}

// ===== ANIMATION TICK ENGINE TESTS =====

void test_tick_draws_frame_on_first_call() {
  MockCanvas canvas;
  bool drew = animationManager.tick(EMOTION_IDLE, canvas);
  TEST_ASSERT_TRUE(drew);
  TEST_ASSERT_TRUE(canvas.findCall(DrawCall::CLEAR) >= 0);
  TEST_ASSERT_TRUE(canvas.findCall(DrawCall::FLUSH) >= 0);
}

void test_tick_respects_frame_delay() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_HAPPY);
  stubSetMillis(0);
  animationManager.tick(EMOTION_HAPPY, canvas);

  canvas.reset();
  stubSetMillis(10);  // Only 10ms, HAPPY needs 30ms
  bool drew = animationManager.tick(EMOTION_HAPPY, canvas);
  TEST_ASSERT_FALSE(drew);
  TEST_ASSERT_EQUAL(0, canvas.callCount());

  canvas.reset();
  stubSetMillis(31);  // Past 30ms delay
  drew = animationManager.tick(EMOTION_HAPPY, canvas);
  TEST_ASSERT_TRUE(drew);
}

void test_tick_advances_frame() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_HAPPY);

  stubSetMillis(0);
  animationManager.tick(EMOTION_HAPPY, canvas);

  stubSetMillis(31);
  animationManager.tick(EMOTION_HAPPY, canvas);

  stubSetMillis(62);
  animationManager.tick(EMOTION_HAPPY, canvas);

  TEST_ASSERT_TRUE(canvas.callCount() >= 6);
}

void test_tick_returns_false_for_unknown_emotion() {
  MockCanvas canvas;
  bool drew = animationManager.tick((EmotionState)99, canvas);
  TEST_ASSERT_FALSE(drew);
}

void test_tick_loop_once_holds_last_frame() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_SHY);
  // Advance through all 30 frames of SHY
  for (int i = 0; i < 40; i++) {
    stubSetMillis((unsigned long)i * 41);
    animationManager.tick(EMOTION_SHY, canvas);
  }
  // Should not crash, should have drawn many times
  TEST_ASSERT_TRUE(canvas.callCount() > 0);
}

// ===== DRAW FUNCTION TESTS (via MockCanvas) =====

void test_draw_idle_draws_eyes() {
  MockCanvas canvas;
  drawIdle(canvas, 0, nullptr);

  int first = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(first >= 0);

  int second = canvas.findCall(DrawCall::FILL_RRECT, first + 1);
  TEST_ASSERT_TRUE(second >= 0);
}

void test_draw_blink_draws_narrow_eyes() {
  MockCanvas canvas;
  drawBlink(canvas, 0, nullptr);

  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).h <= 6);
}

void test_draw_happy_frame0_has_content() {
  MockCanvas canvas;
  drawHappy(canvas, 0, nullptr);
  TEST_ASSERT_TRUE(canvas.callCount() > 0);
}

void test_bored_draws_half_lidded_eyes() {
  MockCanvas canvas;
  drawBored(canvas, 20, nullptr);  // Mid-animation, eyes should be half-closed
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).h < 18);
}

void test_shy_uses_blush() {
  MockCanvas canvas;
  drawShy(canvas, 10, nullptr);  // Mid-animation, blush should be visible
  int idx = canvas.findCall(DrawCall::FILL_CIRCLE);
  TEST_ASSERT_TRUE(idx >= 0);
}

// ===== PERSONALITY ENGINE TESTS =====

void test_attention_arc_escalates() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Beyond stage 1 threshold with max jitter
  unsigned long t = ATTENTION_STAGE1_MS + ATTENTION_STAGE1_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);
}

void test_touch_during_neglect_triggers_recovery() {
  Personality p;
  p.init(0);

  // Force into neglect state (stage 2)
  unsigned long t = ATTENTION_STAGE2_MS + ATTENTION_STAGE2_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  // Drive through stages
  p.update(t, EMOTION_IDLE);
  p.update(t, EMOTION_BORED);

  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);

  bool wasNeglected = p.onTouch(t, EMOTION_SAD);
  TEST_ASSERT_TRUE(wasNeglected);
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());
}

void test_touch_when_not_neglected_returns_false() {
  Personality p;
  p.init(0);
  bool wasNeglected = p.onTouch(100, EMOTION_IDLE);
  TEST_ASSERT_FALSE(wasNeglected);
}

void test_jitter_produces_variance() {
  Personality p;
  p.init(0);
  unsigned long results[20];
  for (int i = 0; i < 20; i++) {
    results[i] = p.jitter(100000);
  }
  bool allSame = true;
  for (int i = 1; i < 20; i++) {
    if (results[i] != results[0]) { allSame = false; break; }
  }
  TEST_ASSERT_FALSE(allSame);
}

void test_recovery_transitions_to_happy() {
  Personality p;
  p.init(0);

  // Force neglect and touch
  unsigned long t = ATTENTION_STAGE1_MS + ATTENTION_STAGE1_MS * JITTER_PERCENT / 100 + 1000;
  p.update(t, EMOTION_IDLE);  // trigger stage 1
  bool wasNeglected = p.onTouch(t, EMOTION_BORED);
  TEST_ASSERT_TRUE(wasNeglected);

  // After SHY timer elapses (1300ms), should return HAPPY
  unsigned long afterShy = t + 1400;
  stubSetMillis(afterShy);
  Personality::Decision d = p.update(afterShy, EMOTION_SHY);
  TEST_ASSERT_TRUE(d.shouldChange);
  TEST_ASSERT_EQUAL(EMOTION_HAPPY, d.emotion);
}

// ===== GESTURE DETECTION TESTS =====

void test_classify_gesture_tap() {
  TEST_ASSERT_EQUAL(GESTURE_TAP, classifyGesture(100, 999));
}

void test_classify_gesture_long_press() {
  TEST_ASSERT_EQUAL(GESTURE_LONG_PRESS, classifyGesture(700, 999));
}

void test_classify_gesture_double_tap() {
  TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, classifyGesture(100, 200));
}

void test_classify_gesture_boundary_long_press() {
  // At exactly LONG_PRESS_MS → long press
  TEST_ASSERT_EQUAL(GESTURE_LONG_PRESS, classifyGesture(LONG_PRESS_MS, 999));
  // One below → tap
  TEST_ASSERT_EQUAL(GESTURE_TAP, classifyGesture(LONG_PRESS_MS - 1, 999));
}

void test_classify_gesture_boundary_double_tap() {
  // At exactly DOUBLE_TAP_WINDOW_MS → double tap
  TEST_ASSERT_EQUAL(GESTURE_DOUBLE_TAP, classifyGesture(100, DOUBLE_TAP_WINDOW_MS));
  // One above → tap
  TEST_ASSERT_EQUAL(GESTURE_TAP, classifyGesture(100, DOUBLE_TAP_WINDOW_MS + 1));
}

void test_all_emotions_draw_without_crash() {
  MockCanvas canvas;
  DrawFrameFn drawFns[] = {
    drawIdle, drawBlink, drawHappy, drawSleepy, drawExcited,
    drawSad, drawAngry, drawConfused, drawThinking, drawLove,
    drawSurprised, drawDead, drawBored, drawShy
  };

  for (int fn = 0; fn < 14; fn++) {
    for (int frame = 0; frame < 51; frame++) {
      canvas.reset();
      drawFns[fn](canvas, frame, nullptr);
      if (frame == 0) {
        TEST_ASSERT_TRUE(canvas.callCount() > 0);
      }
    }
  }
}

// ===== RUNNER =====
int main(int argc, char** argv) {
  UNITY_BEGIN();

  // Emotion manager
  RUN_TEST(test_initial_state_is_idle);
  RUN_TEST(test_set_target_starts_transition);
  RUN_TEST(test_advance_increments_frame);
  RUN_TEST(test_complete_sets_current_and_fires_callback);
  RUN_TEST(test_same_emotion_does_not_transition);
  RUN_TEST(test_invalid_emotion_rejected);
  RUN_TEST(test_change_callback_receives_from_and_to);
  RUN_TEST(test_previous_emotion_tracks_history);
  RUN_TEST(test_transition_frame_advance_and_complete);
  RUN_TEST(test_emotion_validation_uses_registry);
  RUN_TEST(test_ble_emotion_validation_rejects_unregistered);

  // Emotion registry
  RUN_TEST(test_registry_add_and_get);
  RUN_TEST(test_registry_rejects_duplicate);
  RUN_TEST(test_registry_get_name_unknown);
  RUN_TEST(test_registry_cyclable_excludes_blink);
  RUN_TEST(test_registry_count);

  // Animation tick engine
  RUN_TEST(test_tick_draws_frame_on_first_call);
  RUN_TEST(test_tick_respects_frame_delay);
  RUN_TEST(test_tick_advances_frame);
  RUN_TEST(test_tick_returns_false_for_unknown_emotion);
  RUN_TEST(test_tick_loop_once_holds_last_frame);

  // Draw functions
  RUN_TEST(test_draw_idle_draws_eyes);
  RUN_TEST(test_draw_blink_draws_narrow_eyes);
  RUN_TEST(test_draw_happy_frame0_has_content);
  RUN_TEST(test_bored_draws_half_lidded_eyes);
  RUN_TEST(test_shy_uses_blush);
  RUN_TEST(test_all_emotions_draw_without_crash);

  // Personality engine
  RUN_TEST(test_attention_arc_escalates);
  RUN_TEST(test_touch_during_neglect_triggers_recovery);
  RUN_TEST(test_touch_when_not_neglected_returns_false);
  RUN_TEST(test_jitter_produces_variance);
  RUN_TEST(test_recovery_transitions_to_happy);

  // Gesture detection
  RUN_TEST(test_classify_gesture_tap);
  RUN_TEST(test_classify_gesture_long_press);
  RUN_TEST(test_classify_gesture_double_tap);
  RUN_TEST(test_classify_gesture_boundary_long_press);
  RUN_TEST(test_classify_gesture_boundary_double_tap);

  return UNITY_END();
}
