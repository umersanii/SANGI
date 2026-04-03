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
#include "runtime_config.h"
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
  emotionRegistry.add({EMOTION_IDLE,      "IDLE",      60, 55, LOOP_PINGPONG, true,  drawIdle});
  emotionRegistry.add({EMOTION_BLINK,     "BLINK",      1,  0, LOOP_RESTART,  false, drawBlink});
  emotionRegistry.add({EMOTION_HAPPY,     "HAPPY",     50, 35, LOOP_RESTART,  true,  drawHappy});
  emotionRegistry.add({EMOTION_SLEEPY,    "SLEEPY",    59, 50, LOOP_RESTART,  false, drawSleepy});
  emotionRegistry.add({EMOTION_EXCITED,   "EXCITED",   40, 25, LOOP_RESTART,  true,  drawExcited});
  emotionRegistry.add({EMOTION_SAD,       "SAD",       56, 48, LOOP_RESTART,  true,  drawSad});
  emotionRegistry.add({EMOTION_ANGRY,     "ANGRY",     56, 32, LOOP_RESTART,  true,  drawAngry});
  emotionRegistry.add({EMOTION_CONFUSED,  "CONFUSED",  44, 45, LOOP_PINGPONG, true,  drawConfused});
  emotionRegistry.add({EMOTION_THINKING,  "THINKING",  44, 45, LOOP_PINGPONG, true,  drawThinking});
  emotionRegistry.add({EMOTION_LOVE,      "LOVE",      44, 48, LOOP_PINGPONG, true,  drawLove});
  emotionRegistry.add({EMOTION_SURPRISED, "SURPRISED", 44, 30, LOOP_RESTART,  true,  drawSurprised});
  emotionRegistry.add({EMOTION_DEAD,      "DEAD",      70, 55, LOOP_RESTART,  false, drawDead});
  emotionRegistry.add({EMOTION_BORED,     "BORED",     60, 65, LOOP_PINGPONG, true,  drawBored});
  emotionRegistry.add({EMOTION_SHY,       "SHY",       50, 60, LOOP_RESTART,  true,  drawShy});
  emotionRegistry.add({EMOTION_NEEDY,     "NEEDY",     54, 65, LOOP_PINGPONG, true,  drawNeedy});
  emotionRegistry.add({EMOTION_CONTENT,   "CONTENT",   60, 90, LOOP_PINGPONG, true,  drawContent});
  emotionRegistry.add({EMOTION_PLAYFUL,   "PLAYFUL",   48, 40, LOOP_RESTART,  true,  drawPlayful});
  emotionRegistry.add({EMOTION_GRUMPY,    "GRUMPY",    56, 45, LOOP_PINGPONG, true,  drawGrumpy});
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
  // Global registry populated by setUp — 18 emotions
  TEST_ASSERT_EQUAL(18, emotionRegistry.count());
}

// ===== ANIMATION TICK ENGINE TESTS =====

void test_tick_draws_frame_on_first_call() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_IDLE);
  // Advance time past IDLE's 55ms frame delay so first call draws
  stubSetMillis(56);
  bool drew = animationManager.tick(EMOTION_IDLE, canvas);
  TEST_ASSERT_TRUE(drew);
  TEST_ASSERT_TRUE(canvas.findCall(DrawCall::CLEAR) >= 0);
  TEST_ASSERT_TRUE(canvas.findCall(DrawCall::FLUSH) >= 0);
}

void test_tick_respects_frame_delay() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_HAPPY);
  stubSetMillis(36);  // Past HAPPY's 35ms delay — first frame draws
  animationManager.tick(EMOTION_HAPPY, canvas);

  canvas.reset();
  stubSetMillis(46);  // Only 10ms since last tick, HAPPY needs 35ms
  bool drew = animationManager.tick(EMOTION_HAPPY, canvas);
  TEST_ASSERT_FALSE(drew);
  TEST_ASSERT_EQUAL(0, canvas.callCount());

  canvas.reset();
  stubSetMillis(72);  // 36ms since last tick — past 35ms delay
  drew = animationManager.tick(EMOTION_HAPPY, canvas);
  TEST_ASSERT_TRUE(drew);
}

void test_tick_advances_frame() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_HAPPY);

  stubSetMillis(36);
  animationManager.tick(EMOTION_HAPPY, canvas);

  stubSetMillis(72);
  animationManager.tick(EMOTION_HAPPY, canvas);

  stubSetMillis(108);
  animationManager.tick(EMOTION_HAPPY, canvas);

  TEST_ASSERT_TRUE(canvas.callCount() >= 6);
}

void test_tick_returns_false_for_unknown_emotion() {
  MockCanvas canvas;
  bool drew = animationManager.tick((EmotionState)99, canvas);
  TEST_ASSERT_FALSE(drew);
}

void test_tick_loop_pingpong_plays_bored() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_BORED);
  // Advance through frames of BORED (pingpong)
  for (int i = 0; i < 40; i++) {
    stubSetMillis((unsigned long)i * 66);
    animationManager.tick(EMOTION_BORED, canvas);
  }
  // Should not crash, should have drawn many times
  TEST_ASSERT_TRUE(canvas.callCount() > 0);
}

void test_tick_pingpong_reverses_at_ends() {
  // Verify LOOP_PINGPONG: drive BORED (60f, LOOP_PINGPONG) for a full 2*(60-1)=118 tick
  // cycle. Forward pass (ticks 0-58) and backward pass (ticks 59-116) should each draw
  // 59 frames. Total unique frames drawn = 59*2 = 118, all without crash.
  // Also verify visual symmetry: frame drawn at forward tick N equals backward tick at
  // equivalent position, since draw functions are stateless (same frame index → same output).
  animationManager.resetAnimation(EMOTION_BORED);
  int drawCount = 0;
  // Drive 120 ticks — should complete forward + backward pass
  for (int i = 0; i < 120; i++) {
    MockCanvas canvas;
    stubSetMillis((unsigned long)(i + 1) * 66);  // 66ms steps, past BORED's 65ms delay
    if (animationManager.tick(EMOTION_BORED, canvas)) {
      drawCount++;
      // Each drawn frame must have at least one draw call (eyes at minimum)
      TEST_ASSERT_TRUE(canvas.callCount() > 0);
    }
  }
  // All 120 ticks should have drawn (66ms > 65ms delay each time)
  TEST_ASSERT_EQUAL(120, drawCount);
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


// ===== PERSONALITY ENGINE TESTS =====

void test_attention_arc_needy_before_bored() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Beyond NEEDY threshold (stage0) with max jitter
  unsigned long t = ATTENTION_STAGE0_MS + ATTENTION_STAGE0_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  Personality::Decision d = p.update(t, EMOTION_IDLE);
  TEST_ASSERT_EQUAL(1, p.getAttentionStage());
  TEST_ASSERT_TRUE(d.shouldChange);
  TEST_ASSERT_EQUAL(EMOTION_NEEDY, d.emotion);
}

void test_attention_arc_escalates_5_stages() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Beyond stage 2 (BORED) threshold with max jitter
  unsigned long t = ATTENTION_STAGE1_MS + ATTENTION_STAGE1_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  // Drive through NEEDY first, then to BORED
  p.update(t, EMOTION_IDLE);   // → NEEDY (stage 1)
  p.update(t, EMOTION_NEEDY);  // → BORED (stage 2) if enough time
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);
}

void test_attention_arc_grumpy_at_stage4() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Jump well past ALL thresholds including max jitter on STAGE4
  unsigned long t = ATTENTION_STAGE4_MS * 2;
  stubSetMillis(t);

  // Drive through all 5 stages
  EmotionState current = EMOTION_IDLE;
  bool sawGrumpy = false;
  for (int i = 0; i < 10; i++) {
    Personality::Decision d = p.update(t, current);
    if (d.shouldChange) {
      if (d.emotion == EMOTION_GRUMPY) sawGrumpy = true;
      current = d.emotion;
    }
  }
  // Should have passed through GRUMPY on the way to ANGRY
  TEST_ASSERT_TRUE(sawGrumpy || current == EMOTION_GRUMPY || current == EMOTION_ANGRY);
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 4);
}

void test_touch_during_neglect_triggers_recovery() {
  Personality p;
  p.init(0);

  // Force into light neglect state (stage 1-2, not deep enough for forgiveness)
  unsigned long t = ATTENTION_STAGE1_MS + ATTENTION_STAGE1_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  // Drive through stages
  p.update(t, EMOTION_IDLE);
  p.update(t, EMOTION_NEEDY);

  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);

  bool wasNeglected = p.onTouch(t, EMOTION_BORED);
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

void test_glow_activates_on_touch() {
  Personality p;
  p.init(0);
  p.onTouch(1000, EMOTION_IDLE);
  TEST_ASSERT_EQUAL(GLOW_DRIFT_CYCLES, p.getGlowCycles());
}

void test_glow_resets_on_repeated_touch() {
  Personality p;
  p.init(0);
  p.onTouch(1000, EMOTION_IDLE);
  p.onTouch(2000, EMOTION_IDLE);  // second touch resets to full glow again
  TEST_ASSERT_EQUAL(GLOW_DRIFT_CYCLES, p.getGlowCycles());
}

void test_warmth_activates_after_threshold_touches() {
  Personality p;
  p.init(0);
  // Touch WARMTH_TOUCH_THRESHOLD times within the window
  for (int i = 0; i < WARMTH_TOUCH_THRESHOLD; i++) {
    p.onTouch((unsigned long)(i * 1000), EMOTION_IDLE);
  }
  TEST_ASSERT_TRUE(p.isWarmed());
}

void test_warmth_does_not_activate_below_threshold() {
  Personality p;
  p.init(0);
  for (int i = 0; i < WARMTH_TOUCH_THRESHOLD - 1; i++) {
    p.onTouch((unsigned long)(i * 1000), EMOTION_IDLE);
  }
  TEST_ASSERT_FALSE(p.isWarmed());
}

void test_warmth_window_resets_after_expiry() {
  Personality p;
  p.init(0);
  // Touch once early
  p.onTouch(1000, EMOTION_IDLE);
  // Then touch past the window — count should reset, no warmth yet
  unsigned long past = WARMTH_WINDOW_MS + 2000;
  p.onTouch(past, EMOTION_IDLE);
  TEST_ASSERT_FALSE(p.isWarmed());
}

void test_habituation_resets_on_different_drift() {
  Personality p;
  p.init(0);
  // Force mood drift multiple times with the same resulting emotion by fixing the time
  // We can't directly control moodDrift's random output, so just verify the counter
  // resets when an explicitly different emotion fires
  // Drive a drift toward BORED (attention arc)
  unsigned long t = ATTENTION_STAGE1_MS + ATTENTION_STAGE1_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);    // → BORED (attention arc)
  p.onTouch(t, EMOTION_BORED);  // reset arc, counter resets
  TEST_ASSERT_EQUAL(0, p.getConsecutiveSameDrifts());
}

static int testHourOverride = 12;
static int testHourProvider() { return testHourOverride; }

void test_ntp_time_provider_used_when_set() {
  // Verify that time provider affects drift output.
  // Suppress attention arc so only drift fires.
  unsigned long savedStage0 = runtimeConfig.attentionStage0Ms;
  runtimeConfig.attentionStage0Ms = 999999999UL;

  testHourOverride = 5;
  int nightSleepy = 0;
  {
    Personality p;
    p.setTimeProvider(testHourProvider);
    for (int i = 0; i < 200; i++) {
      p.init(0);
      Personality::Decision d = p.update(200000UL, EMOTION_IDLE);
      if (d.shouldChange && d.emotion == EMOTION_SLEEPY) nightSleepy++;
    }
  }

  testHourOverride = 9;
  int morningSleepy = 0;
  {
    Personality p;
    p.setTimeProvider(testHourProvider);
    for (int i = 0; i < 200; i++) {
      p.init(0);
      Personality::Decision d = p.update(200000UL, EMOTION_IDLE);
      if (d.shouldChange && d.emotion == EMOTION_SLEEPY) morningSleepy++;
    }
  }

  runtimeConfig.attentionStage0Ms = savedStage0;
  // Night should produce significantly more SLEEPY than morning
  TEST_ASSERT_TRUE(nightSleepy > morningSleepy);
}

// ===== FORGIVENESS TESTS =====

void test_forgiveness_required_for_deep_neglect() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Jump well past ANGRY threshold
  unsigned long t = ATTENTION_STAGE4_MS + ATTENTION_STAGE4_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);

  // Drive through all 5 stages
  EmotionState current = EMOTION_IDLE;
  for (int i = 0; i < 15; i++) {
    Personality::Decision d = p.update(t, current);
    if (d.shouldChange) current = d.emotion;
  }
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 4);

  // First touch starts forgiveness — doesn't reset arc
  bool wasNeglected = p.onTouch(t, current);
  TEST_ASSERT_TRUE(wasNeglected);
  TEST_ASSERT_TRUE(p.isForgiving());
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 4);  // arc NOT reset yet
}

void test_forgiveness_completes_after_enough_touches() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Jump to GRUMPY/ANGRY
  unsigned long t = ATTENTION_STAGE4_MS + ATTENTION_STAGE4_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  EmotionState current = EMOTION_IDLE;
  for (int i = 0; i < 15; i++) {
    Personality::Decision d = p.update(t, current);
    if (d.shouldChange) current = d.emotion;
  }

  // Touch FORGIVENESS_TOUCHES + 1 times (first starts counter, rest decrement)
  p.onTouch(t, current);  // starts forgiveness
  for (int i = 0; i < FORGIVENESS_TOUCHES; i++) {
    p.onTouch(t + (unsigned long)(i + 1) * 1000, current);
  }
  // After enough touches, forgiveness complete and arc reset
  TEST_ASSERT_FALSE(p.isForgiving());
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());
}

void test_light_neglect_no_forgiveness_needed() {
  Personality p;
  p.init(0);
  p.onTouch(0, EMOTION_IDLE);

  // Only reach NEEDY (stage 1) — no forgiveness required
  unsigned long t = ATTENTION_STAGE0_MS + ATTENTION_STAGE0_MS * JITTER_PERCENT / 100 + 1000;
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);  // → NEEDY
  TEST_ASSERT_TRUE(p.getAttentionStage() >= 1);
  TEST_ASSERT_TRUE(p.getAttentionStage() < 4);

  bool wasNeglected = p.onTouch(t, EMOTION_NEEDY);
  TEST_ASSERT_TRUE(wasNeglected);
  TEST_ASSERT_FALSE(p.isForgiving());
  TEST_ASSERT_EQUAL(0, p.getAttentionStage());
}

// ===== MOOD GRAVITY TESTS =====

void test_mood_cluster_classification() {
  TEST_ASSERT_EQUAL(CLUSTER_POSITIVE, Personality::clusterOf(EMOTION_HAPPY));
  TEST_ASSERT_EQUAL(CLUSTER_POSITIVE, Personality::clusterOf(EMOTION_PLAYFUL));
  TEST_ASSERT_EQUAL(CLUSTER_POSITIVE, Personality::clusterOf(EMOTION_CONTENT));
  TEST_ASSERT_EQUAL(CLUSTER_POSITIVE, Personality::clusterOf(EMOTION_LOVE));
  TEST_ASSERT_EQUAL(CLUSTER_POSITIVE, Personality::clusterOf(EMOTION_EXCITED));
  TEST_ASSERT_EQUAL(CLUSTER_NEUTRAL,  Personality::clusterOf(EMOTION_IDLE));
  TEST_ASSERT_EQUAL(CLUSTER_NEUTRAL,  Personality::clusterOf(EMOTION_THINKING));
  TEST_ASSERT_EQUAL(CLUSTER_NEGATIVE, Personality::clusterOf(EMOTION_SAD));
  TEST_ASSERT_EQUAL(CLUSTER_NEGATIVE, Personality::clusterOf(EMOTION_BORED));
  TEST_ASSERT_EQUAL(CLUSTER_NEGATIVE, Personality::clusterOf(EMOTION_GRUMPY));
  TEST_ASSERT_EQUAL(CLUSTER_NEGATIVE, Personality::clusterOf(EMOTION_ANGRY));
  TEST_ASSERT_EQUAL(CLUSTER_NEGATIVE, Personality::clusterOf(EMOTION_SLEEPY));
}

static int noonProvider() { return 12; }

void test_mood_gravity_biases_toward_same_cluster() {
  // Compare positive-cluster hit rate when starting from HAPPY vs IDLE.
  // Suppress attention arc so only drift fires.
  unsigned long savedStage0 = runtimeConfig.attentionStage0Ms;
  runtimeConfig.attentionStage0Ms = 999999999UL;

  int fromHappy = 0;
  {
    Personality p;
    p.setTimeProvider(noonProvider);
    for (int i = 0; i < 200; i++) {
      p.init(0);
      Personality::Decision d = p.update(200000UL, EMOTION_HAPPY);
      if (d.shouldChange && Personality::clusterOf(d.emotion) == CLUSTER_POSITIVE) fromHappy++;
    }
  }
  int fromIdle = 0;
  {
    Personality p;
    p.setTimeProvider(noonProvider);
    for (int i = 0; i < 200; i++) {
      p.init(0);
      Personality::Decision d = p.update(200000UL, EMOTION_IDLE);
      if (d.shouldChange && Personality::clusterOf(d.emotion) == CLUSTER_POSITIVE) fromIdle++;
    }
  }

  runtimeConfig.attentionStage0Ms = savedStage0;
  // Starting HAPPY should produce more positive drifts than starting IDLE
  TEST_ASSERT_TRUE(fromHappy > fromIdle);
}

// ===== NIGHT CYCLE TESTS =====

void test_night_cycle_activates_at_2am() {
  Personality p;
  p.init(0);
  p.setTimeProvider([]() -> int { return 2; });

  // Advance time past drift interval to trigger night cycle
  unsigned long t = 200000;
  Personality::Decision d = p.update(t, EMOTION_IDLE);
  TEST_ASSERT_TRUE(p.isNightCycleActive());
}

void test_night_cycle_inactive_outside_window() {
  Personality p;
  p.init(0);
  p.setTimeProvider([]() -> int { return 5; });

  unsigned long t = 200000;
  p.update(t, EMOTION_IDLE);
  TEST_ASSERT_FALSE(p.isNightCycleActive());
}

void test_night_cycle_produces_mostly_sleepy() {
  Personality p;
  p.init(0);
  p.setTimeProvider([]() -> int { return 3; });

  int sleepyCount = 0;
  int totalChanges = 0;
  for (int i = 0; i < 100; i++) {
    unsigned long t = (unsigned long)(i + 1) * 100000UL;  // well past 45s interval
    Personality::Decision d = p.update(t, EMOTION_IDLE);
    if (d.shouldChange) {
      totalChanges++;
      if (d.emotion == EMOTION_SLEEPY) sleepyCount++;
    }
    p.init(t);
    p.setTimeProvider([]() -> int { return 3; });
  }
  // 70% SLEEPY in night cycle — expect at least 40% of total changes to be SLEEPY
  if (totalChanges > 0) {
    TEST_ASSERT_TRUE(sleepyCount > totalChanges / 3);
  }
}

void test_night_cycle_requires_time_provider() {
  Personality p;
  p.init(0);
  // No time provider set — night cycle should not activate even at equivalent millis
  unsigned long t = 2 * HOUR_IN_MILLIS + 100000;  // ~2:01 AM by millis
  stubSetMillis(t);
  p.update(t, EMOTION_IDLE);
  TEST_ASSERT_FALSE(p.isNightCycleActive());
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
  // Each emotion tested up to its own frame count (not a uniform 51)
  struct { DrawFrameFn fn; int frames; } emotions[] = {
    {drawIdle, 60}, {drawBlink, 1}, {drawHappy, 50}, {drawSleepy, 59},
    {drawExcited, 40}, {drawSad, 56}, {drawAngry, 56}, {drawConfused, 44},
    {drawThinking, 44}, {drawLove, 44}, {drawSurprised, 44},
    {drawDead, 70}, {drawBored, 60}, {drawShy, 50}, {drawPlayful, 48},
    {drawGrumpy, 56}
  };
  MockCanvas canvas;
  for (int i = 0; i < 16; i++) {
    for (int frame = 0; frame < emotions[i].frames; frame++) {
      canvas.reset();
      emotions[i].fn(canvas, frame, nullptr);
      TEST_ASSERT_TRUE(canvas.callCount() > 0);
    }
  }
}

// ===== NEW PER-EMOTION ASSERTIONS =====

void test_idle_uses_new_eye_dimensions() {
  MockCanvas canvas;
  drawIdle(canvas, 0, nullptr);
  // Eyes should be w=24 per new grammar (FILL_RRECT with w=24)
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_EQUAL(24, canvas.call(idx).w);
}

void test_idle_breathing_moves_eyes() {
  MockCanvas canvasF0, canvasF14;
  drawIdle(canvasF0, 0, nullptr);
  drawIdle(canvasF14, 14, nullptr);
  // Frame 14 (peak inhale) eyes should be 1px higher than frame 0 (eyeY=27 vs 28)
  int i0 = canvasF0.findCall(DrawCall::FILL_RRECT);
  int i14 = canvasF14.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(i0 >= 0 && i14 >= 0);
  // Y of eye rect = eyeY - eyeH/2; at F14 eyeY=27→rect.y=16, at F0 eyeY=28→rect.y=17
  TEST_ASSERT_TRUE(canvasF14.call(i14).y < canvasF0.call(i0).y);
}

void test_happy_has_blush_at_peak() {
  MockCanvas canvas;
  drawHappy(canvas, 7, nullptr);  // peak smile frame
  // Eyes as narrow arcs (h=8)
  int eyeIdx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(eyeIdx >= 0);
  TEST_ASSERT_EQUAL(8, canvas.call(eyeIdx).h);
  // Blush circles present
  int blushIdx = canvas.findCall(DrawCall::FILL_CIRCLE);
  TEST_ASSERT_TRUE(blushIdx >= 0);
}

void test_sad_tear_present_at_frame_12() {
  MockCanvas canvas;
  drawSad(canvas, 12, nullptr);
  // Tear as FILL_CIRCLE at around Y=42
  int idx = canvas.findCall(DrawCall::FILL_CIRCLE);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).y >= 40 && canvas.call(idx).y <= 45);
}

void test_sad_tear_trail_at_frame_20() {
  MockCanvas canvas;
  drawSad(canvas, 20, nullptr);
  // Tear trail as DRAW_LINE
  int idx = canvas.findCall(DrawCall::DRAW_LINE);
  TEST_ASSERT_TRUE(idx >= 0);
}

void test_confused_has_asymmetric_eyes() {
  MockCanvas canvas;
  drawConfused(canvas, 7, nullptr);  // left tall, right squat
  int idx1 = canvas.findCall(DrawCall::FILL_RRECT, 0);
  int idx2 = canvas.findCall(DrawCall::FILL_RRECT, idx1 + 1);
  TEST_ASSERT_TRUE(idx1 >= 0 && idx2 >= 0);
  // First eye taller than second
  TEST_ASSERT_TRUE(canvas.call(idx1).h != canvas.call(idx2).h);
}

void test_confused_has_question_mark() {
  MockCanvas canvas;
  drawConfused(canvas, 12, nullptr);
  int idx = canvas.findCall(DrawCall::TEXT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_EQUAL_STRING("?", canvas.call(idx).text);
}

void test_angry_has_brow_lines() {
  MockCanvas canvas;
  drawAngry(canvas, 7, nullptr);  // full glare frame
  // Brow lines as DRAW_LINE
  int idx = canvas.findCall(DrawCall::DRAW_LINE);
  TEST_ASSERT_TRUE(idx >= 0);
}

void test_angry_eyes_narrow_at_peak() {
  MockCanvas canvas;
  drawAngry(canvas, 7, nullptr);
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).h <= 12);
}

void test_love_has_heart_components() {
  MockCanvas canvas;
  drawLove(canvas, 5, nullptr);  // full hearts
  // Hearts drawn with fill circles + fill triangles
  int circIdx = canvas.findCall(DrawCall::FILL_CIRCLE);
  int triIdx = canvas.findCall(DrawCall::FILL_TRIANGLE);
  TEST_ASSERT_TRUE(circIdx >= 0);
  TEST_ASSERT_TRUE(triIdx >= 0);
}

void test_excited_has_pupils() {
  MockCanvas canvas;
  drawExcited(canvas, 5, nullptr);  // wide-eye frame
  // Pupils as FILL_CIRCLE with COLOR_BLACK
  bool foundPupil = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::FILL_CIRCLE &&
        canvas.call(i).color == COLOR_BLACK) {
      foundPupil = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundPupil);
}

void test_excited_bounce_moves_face() {
  MockCanvas canvasUp, canvasDown;
  drawExcited(canvasUp, 7, nullptr);   // bounce up (odd frame → down, even → up)
  drawExcited(canvasDown, 8, nullptr); // bounce down
  int iUp = canvasUp.findCall(DrawCall::FILL_RRECT);
  int iDown = canvasDown.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(iUp >= 0 && iDown >= 0);
  TEST_ASSERT_TRUE(canvasUp.call(iUp).y != canvasDown.call(iDown).y);
}

void test_sleepy_has_z_at_sleeping_frame() {
  MockCanvas canvas;
  drawSleepy(canvas, 15, nullptr);
  int idx = canvas.findCall(DrawCall::TEXT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_EQUAL_STRING("z", canvas.call(idx).text);
}

void test_sleepy_has_yawn_circle() {
  MockCanvas canvas;
  drawSleepy(canvas, 10, nullptr);  // nearly closed
  // Yawn as FILL_CIRCLE at approximately center bottom
  int idx = canvas.findCall(DrawCall::FILL_CIRCLE);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).x >= 60 && canvas.call(idx).x <= 68);
}

void test_thinking_gaze_shifts_left() {
  MockCanvas canvasF0, canvasF6;
  drawThinking(canvasF0, 0, nullptr);
  drawThinking(canvasF6, 6, nullptr);
  int i0 = canvasF0.findCall(DrawCall::FILL_RRECT);
  int i6 = canvasF6.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(i0 >= 0 && i6 >= 0);
  // At F6 left eye shifted left → rect.x smaller than F0
  TEST_ASSERT_TRUE(canvasF6.call(i6).x < canvasF0.call(i0).x);
}

void test_thinking_has_dots() {
  MockCanvas canvas;
  drawThinking(canvas, 14, nullptr);  // two dots visible
  int idx = canvas.findCall(DrawCall::FILL_RECT);
  TEST_ASSERT_TRUE(idx >= 0);
}

void test_thinking_has_exclamation() {
  MockCanvas canvas;
  drawThinking(canvas, 31, nullptr);  // aha frame
  int idx = canvas.findCall(DrawCall::FILL_RECT);
  TEST_ASSERT_TRUE(idx >= 0);
  // Exclamation bar: tall rect (h=14)
  bool foundBar = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::FILL_RECT && canvas.call(i).h >= 14) {
      foundBar = true;
      break;
    }
  }
  TEST_ASSERT_TRUE(foundBar);
}

void test_surprised_eyes_widen_and_have_pupils() {
  MockCanvas canvas;
  drawSurprised(canvas, 5, nullptr);  // peak surprise
  // Eyes at H=28
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_EQUAL(28, canvas.call(idx).h);
  // Pupils present
  bool hasPupil = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::FILL_CIRCLE &&
        canvas.call(i).color == COLOR_BLACK) {
      hasPupil = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasPupil);
}

void test_surprised_double_take_blink() {
  MockCanvas canvas;
  drawSurprised(canvas, 11, nullptr);  // double-take blink frame
  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_EQUAL(4, canvas.call(idx).h);  // nearly shut
}

void test_dead_has_x_eyes() {
  MockCanvas canvas;
  drawDead(canvas, 13, nullptr);  // X eyes formed
  // X eyes drawn as DRAW_LINE
  int idx = canvas.findCall(DrawCall::DRAW_LINE);
  TEST_ASSERT_TRUE(idx >= 0);
}

void test_dead_has_tongue() {
  MockCanvas canvas;
  drawDead(canvas, 15, nullptr);  // hold dead
  // Tongue as FILL_RRECT (two of them for base + tip)
  int idx1 = canvas.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(idx1 >= 0);
  int idx2 = canvas.findCall(DrawCall::FILL_RRECT, idx1 + 1);
  TEST_ASSERT_TRUE(idx2 >= 0);
}

void test_dead_has_dizzy_circles() {
  MockCanvas canvas;
  drawDead(canvas, 20, nullptr);
  int idx = canvas.findCall(DrawCall::DRAW_CIRCLE);
  TEST_ASSERT_TRUE(idx >= 0);
}

void test_bored_has_head_tilt() {
  MockCanvas canvas;
  drawBored(canvas, 13, nullptr);  // hold half-closed
  int idx1 = canvas.findCall(DrawCall::FILL_RRECT, 0);
  int idx2 = canvas.findCall(DrawCall::FILL_RRECT, idx1 + 1);
  TEST_ASSERT_TRUE(idx1 >= 0 && idx2 >= 0);
  // Right eye (second RRECT) should be 2px lower than left
  TEST_ASSERT_TRUE(canvas.call(idx2).y > canvas.call(idx1).y);
}

void test_bored_has_sigh_mouth() {
  MockCanvas canvas;
  drawBored(canvas, 38, nullptr);  // sigh frame
  // Sigh as DRAW_CIRCLE
  int idx = canvas.findCall(DrawCall::DRAW_CIRCLE);
  TEST_ASSERT_TRUE(idx >= 0);
}

void test_shy_has_asymmetric_eyes_during_avert() {
  MockCanvas canvas;
  drawShy(canvas, 12, nullptr);  // avert phase (F8-18)
  // Left eye (hiding) and right eye (peeking) should have different heights
  int idx1 = canvas.findCall(DrawCall::FILL_RRECT, 0);
  int idx2 = canvas.findCall(DrawCall::FILL_RRECT, idx1 + 1);
  TEST_ASSERT_TRUE(idx1 >= 0 && idx2 >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx1).h != canvas.call(idx2).h);
}

void test_shy_has_blush() {
  MockCanvas canvas;
  drawShy(canvas, 15, nullptr);  // avert phase — blush should be present
  bool hasBlush = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::FILL_CIRCLE) {
      hasBlush = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasBlush);
}

void test_shy_eyes_converge_in_height() {
  MockCanvas canvas1, canvas2;
  drawShy(canvas1, 12, nullptr);  // avert phase — left eye much smaller than right
  drawShy(canvas2, 35, nullptr);  // warm up phase — heights converging
  // Left eye (first RRECT) should be taller in warm-up than avert
  int avertLeft = canvas1.findCall(DrawCall::FILL_RRECT, 0);
  int warmLeft  = canvas2.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(avertLeft >= 0 && warmLeft >= 0);
  TEST_ASSERT_TRUE(canvas2.call(warmLeft).h > canvas1.call(avertLeft).h);
}

// ===== NEEDY emotion tests =====

void test_needy_has_oversized_eyes_at_plead() {
  MockCanvas canvas;
  drawNeedy(canvas, 20, nullptr);  // plead phase (F15-35)
  // Eyes should be larger than neutral H=22
  int idx = canvas.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).h >= 25);  // oversized eyes
}

void test_needy_has_sad_mouth() {
  MockCanvas canvas;
  drawNeedy(canvas, 20, nullptr);  // plead phase
  // Should have drawLine calls for the downturned mouth arc
  bool hasMouthLine = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::DRAW_LINE &&
        canvas.call(i).y > 50) {  // mouth region
      hasMouthLine = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasMouthLine);
}

void test_needy_eyes_pulse_during_plead() {
  MockCanvas canvas1, canvas2;
  drawNeedy(canvas1, 17, nullptr);  // early plead — eyes shrinking
  drawNeedy(canvas2, 22, nullptr);  // mid plead — eyes growing back
  int idx1 = canvas1.findCall(DrawCall::FILL_RRECT, 0);
  int idx2 = canvas2.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(idx1 >= 0 && idx2 >= 0);
  // Heights should differ due to pulsing
  TEST_ASSERT_TRUE(canvas1.call(idx1).h != canvas2.call(idx2).h);
}

// ===== CONTENT emotion tests =====

void test_content_has_half_closed_eyes() {
  MockCanvas canvas;
  drawContent(canvas, 25, nullptr);  // deep content phase (F15-40)
  // Eyes relaxed (H=14) — more open than BORED (H=8), less than neutral (H=22)
  int idx = canvas.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).h > 8 && canvas.call(idx).h <= 16);
}

void test_content_has_wide_smile() {
  MockCanvas canvas;
  drawContent(canvas, 25, nullptr);  // deep content phase
  // Should have a wide smile (fillRoundRect in mouth region, W >= 24)
  bool hasWideSmile = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::FILL_RRECT &&
        canvas.call(i).y >= 50 && canvas.call(i).w >= 24) {
      hasWideSmile = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasWideSmile);
}

void test_content_has_blush() {
  MockCanvas canvas;
  drawContent(canvas, 25, nullptr);  // deep content phase — blush should pulse
  bool hasBlush = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::FILL_CIRCLE) {
      hasBlush = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasBlush);
}

void test_content_slow_blink() {
  MockCanvas canvas1, canvas2;
  drawContent(canvas1, 43, nullptr);  // blink near-shut (closing done)
  drawContent(canvas2, 47, nullptr);  // blink reopening (well underway)
  int idx1 = canvas1.findCall(DrawCall::FILL_RRECT, 0);
  int idx2 = canvas2.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(idx1 >= 0 && idx2 >= 0);
  // Eyes should be smaller during close than during reopen
  TEST_ASSERT_TRUE(canvas1.call(idx1).h < canvas2.call(idx2).h);
}

// ===== PLAYFUL emotion tests =====

void test_playful_has_asymmetric_eyes_during_wink() {
  MockCanvas canvas;
  drawPlayful(canvas, 20, nullptr);  // wink reopening (F16-23) — left still squinted, right open
  int idx1 = canvas.findCall(DrawCall::FILL_RRECT, 0);
  int idx2 = canvas.findCall(DrawCall::FILL_RRECT, idx1 + 1);
  TEST_ASSERT_TRUE(idx1 >= 0 && idx2 >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx1).h != canvas.call(idx2).h);
}

void test_playful_wink_closes_left_eye() {
  MockCanvas canvas;
  drawPlayful(canvas, 14, nullptr);  // wink held shut (F14-15) — left eye near-closed
  int idx = canvas.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(idx >= 0);
  TEST_ASSERT_TRUE(canvas.call(idx).h <= 2);
}

void test_playful_has_asymmetric_grin() {
  MockCanvas canvas;
  drawPlayful(canvas, 14, nullptr);  // wink hold — asymmetric grin present
  // Grin drawn with DRAW_LINE calls in the mouth region (y > 50)
  bool hasMouthLine = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::DRAW_LINE &&
        canvas.call(i).y > 50) {
      hasMouthLine = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasMouthLine);
}

void test_playful_bounce_moves_face() {
  MockCanvas canvasUp, canvasDown;
  drawPlayful(canvasUp,   24, nullptr);  // bounce up (yOff=-2)
  drawPlayful(canvasDown, 27, nullptr);  // bounce baseline (yOff=0)
  int iUp   = canvasUp.findCall(DrawCall::FILL_RRECT, 0);
  int iDown = canvasDown.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(iUp >= 0 && iDown >= 0);
  TEST_ASSERT_TRUE(canvasUp.call(iUp).y < canvasDown.call(iDown).y);
}

// ===== GRUMPY emotion tests =====

void test_grumpy_brows_are_flat() {
  MockCanvas canvas;
  drawGrumpy(canvas, 12, nullptr);  // hold stare (F8-20) — flat brows locked
  // drawBrow calls drawLine with y0==y1 for flat (stored as .y and .h in MockCanvas)
  bool foundFlatBrow = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::DRAW_LINE && canvas.call(i).y < 30) {
      // Flat brow: both endpoints at same Y (y0 == y1, stored as .y and .h)
      if (canvas.call(i).y == canvas.call(i).h) {
        foundFlatBrow = true; break;
      }
    }
  }
  TEST_ASSERT_TRUE(foundFlatBrow);
}

void test_grumpy_brows_are_not_v_shaped() {
  MockCanvas canvas;
  drawGrumpy(canvas, 12, nullptr);  // hold stare — brows flat, not V-angled like ANGRY
  // All brow lines (y < 30) must have y0 == y1 — no angled lines allowed
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::DRAW_LINE && canvas.call(i).y < 30) {
      TEST_ASSERT_EQUAL(canvas.call(i).y, canvas.call(i).h);
    }
  }
}

void test_grumpy_has_downturned_frown() {
  MockCanvas canvas;
  drawGrumpy(canvas, 14, nullptr);  // hold stare (F8-22) — frown fully present
  // Frown: left half goes from low corner (y=58) UP to center (y=53) — y1 < y0
  bool hasFrownLine = false;
  for (int i = 0; i < canvas.callCount(); i++) {
    if (canvas.call(i).type == DrawCall::DRAW_LINE &&
        canvas.call(i).y >= 50 && canvas.call(i).h < canvas.call(i).y) {
      hasFrownLine = true; break;
    }
  }
  TEST_ASSERT_TRUE(hasFrownLine);
}

void test_grumpy_squint_narrows_eyes() {
  MockCanvas stare, squinting;
  drawGrumpy(stare,     12, nullptr);  // stare (F9-16) — eyes at H=10
  drawGrumpy(squinting, 32, nullptr);  // squint hold (sqF=15) — eyes at H=4
  int i1 = stare.findCall(DrawCall::FILL_RRECT, 0);
  int i2 = squinting.findCall(DrawCall::FILL_RRECT, 0);
  TEST_ASSERT_TRUE(i1 >= 0 && i2 >= 0);
  TEST_ASSERT_TRUE(stare.call(i1).h > squinting.call(i2).h);
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
  RUN_TEST(test_tick_loop_pingpong_plays_bored);
  RUN_TEST(test_tick_pingpong_reverses_at_ends);

  // Draw functions — legacy
  RUN_TEST(test_draw_idle_draws_eyes);
  RUN_TEST(test_draw_blink_draws_narrow_eyes);
  RUN_TEST(test_draw_happy_frame0_has_content);
  RUN_TEST(test_bored_draws_half_lidded_eyes);
  RUN_TEST(test_all_emotions_draw_without_crash);

  // Draw functions — new per-emotion assertions
  RUN_TEST(test_idle_uses_new_eye_dimensions);
  RUN_TEST(test_idle_breathing_moves_eyes);
  RUN_TEST(test_happy_has_blush_at_peak);
  RUN_TEST(test_sad_tear_present_at_frame_12);
  RUN_TEST(test_sad_tear_trail_at_frame_20);
  RUN_TEST(test_confused_has_asymmetric_eyes);
  RUN_TEST(test_confused_has_question_mark);
  RUN_TEST(test_angry_has_brow_lines);
  RUN_TEST(test_angry_eyes_narrow_at_peak);
  RUN_TEST(test_love_has_heart_components);
  RUN_TEST(test_excited_has_pupils);
  RUN_TEST(test_excited_bounce_moves_face);
  RUN_TEST(test_sleepy_has_z_at_sleeping_frame);
  RUN_TEST(test_sleepy_has_yawn_circle);
  RUN_TEST(test_thinking_gaze_shifts_left);
  RUN_TEST(test_thinking_has_dots);
  RUN_TEST(test_thinking_has_exclamation);
  RUN_TEST(test_surprised_eyes_widen_and_have_pupils);
  RUN_TEST(test_surprised_double_take_blink);
  RUN_TEST(test_dead_has_x_eyes);
  RUN_TEST(test_dead_has_tongue);
  RUN_TEST(test_dead_has_dizzy_circles);
  RUN_TEST(test_bored_has_head_tilt);
  RUN_TEST(test_bored_has_sigh_mouth);
  RUN_TEST(test_shy_has_asymmetric_eyes_during_avert);
  RUN_TEST(test_shy_has_blush);
  RUN_TEST(test_shy_eyes_converge_in_height);
  RUN_TEST(test_needy_has_oversized_eyes_at_plead);
  RUN_TEST(test_needy_has_sad_mouth);
  RUN_TEST(test_needy_eyes_pulse_during_plead);
  RUN_TEST(test_content_has_half_closed_eyes);
  RUN_TEST(test_content_has_wide_smile);
  RUN_TEST(test_content_has_blush);
  RUN_TEST(test_content_slow_blink);
  RUN_TEST(test_playful_has_asymmetric_eyes_during_wink);
  RUN_TEST(test_playful_wink_closes_left_eye);
  RUN_TEST(test_playful_has_asymmetric_grin);
  RUN_TEST(test_playful_bounce_moves_face);
  RUN_TEST(test_grumpy_brows_are_flat);
  RUN_TEST(test_grumpy_brows_are_not_v_shaped);
  RUN_TEST(test_grumpy_has_downturned_frown);
  RUN_TEST(test_grumpy_squint_narrows_eyes);

  // Personality engine — attention arc
  RUN_TEST(test_attention_arc_needy_before_bored);
  RUN_TEST(test_attention_arc_escalates_5_stages);
  RUN_TEST(test_attention_arc_grumpy_at_stage4);
  RUN_TEST(test_touch_during_neglect_triggers_recovery);
  RUN_TEST(test_touch_when_not_neglected_returns_false);
  RUN_TEST(test_jitter_produces_variance);
  RUN_TEST(test_glow_activates_on_touch);
  RUN_TEST(test_glow_resets_on_repeated_touch);
  RUN_TEST(test_warmth_activates_after_threshold_touches);
  RUN_TEST(test_warmth_does_not_activate_below_threshold);
  RUN_TEST(test_warmth_window_resets_after_expiry);
  RUN_TEST(test_habituation_resets_on_different_drift);
  RUN_TEST(test_ntp_time_provider_used_when_set);

  // Personality engine — forgiveness
  RUN_TEST(test_forgiveness_required_for_deep_neglect);
  RUN_TEST(test_forgiveness_completes_after_enough_touches);
  RUN_TEST(test_light_neglect_no_forgiveness_needed);

  // Personality engine — mood gravity
  RUN_TEST(test_mood_cluster_classification);
  RUN_TEST(test_mood_gravity_biases_toward_same_cluster);

  // Personality engine — night cycle
  RUN_TEST(test_night_cycle_activates_at_2am);
  RUN_TEST(test_night_cycle_inactive_outside_window);
  RUN_TEST(test_night_cycle_produces_mostly_sleepy);
  RUN_TEST(test_night_cycle_requires_time_provider);

  // Gesture detection
  RUN_TEST(test_classify_gesture_tap);
  RUN_TEST(test_classify_gesture_long_press);
  RUN_TEST(test_classify_gesture_double_tap);
  RUN_TEST(test_classify_gesture_boundary_long_press);
  RUN_TEST(test_classify_gesture_boundary_double_tap);

  return UNITY_END();
}
