// SANGI Robot — Native Unit Tests
// Tests for emotion management, emotion registry, notification queue,
// animation tick engine, and draw functions via MockCanvas.
//
// Run with: pio test -e native

#include <unity.h>
#include "emotion.h"
#include "emotion_registry.h"
#include "emotion_draws.h"
#include "animations.h"
#include "notification_queue.h"
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
  // Reset the global registry by reconstructing it
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
  emotionRegistry.add({EMOTION_MUSIC, "MUSIC", 51, 30, LOOP_RESTART, false, drawMusic});
  emotionRegistry.add({EMOTION_NOTIFICATION, "NOTIFICATION", 86, 50, LOOP_ONCE, false, drawNotification});
  emotionRegistry.add({EMOTION_CODING, "CODING", 25, 100, LOOP_RESTART, false, drawCoding});
  emotionRegistry.add({EMOTION_GITHUB_STATS, "GITHUB_STATS", 131, 80, LOOP_RESTART, false, drawGitHubStats});
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
  // Global registry populated by setUp
  TEST_ASSERT_EQUAL(16, emotionRegistry.count());
}

// ===== NOTIFICATION QUEUE TESTS =====

void test_notification_add_and_retrieve() {
  NotificationQueue q;
  TEST_ASSERT_TRUE(q.add(NOTIFY_GENERIC, "Test Title", "Test Message"));
  TEST_ASSERT_EQUAL(1, q.count());
  TEST_ASSERT_TRUE(q.hasItems());

  Notification* n = q.current();
  TEST_ASSERT_NOT_NULL(n);
  TEST_ASSERT_EQUAL_STRING("Test Title", n->title);
  TEST_ASSERT_EQUAL_STRING("Test Message", n->message);
  TEST_ASSERT_EQUAL(NOTIFY_GENERIC, n->type);
}

void test_notification_queue_overflow() {
  NotificationQueue q;

  for (int i = 0; i < NotificationQueue::MAX_SIZE; i++) {
    char title[32];
    snprintf(title, sizeof(title), "Notif %d", i);
    TEST_ASSERT_TRUE(q.add(NOTIFY_GENERIC, title, "Message"));
  }
  TEST_ASSERT_EQUAL(NotificationQueue::MAX_SIZE, q.count());

  // Should fail — queue full
  TEST_ASSERT_FALSE(q.add(NOTIFY_GENERIC, "Overflow", "Fail"));
  TEST_ASSERT_EQUAL(NotificationQueue::MAX_SIZE, q.count());
}

void test_notification_clear_and_empty() {
  NotificationQueue q;
  q.add(NOTIFY_DISCORD, "Test", "Msg");
  TEST_ASSERT_EQUAL(1, q.count());

  q.current();  // sets currentIdx_
  q.clearCurrent();
  TEST_ASSERT_EQUAL(0, q.count());
  TEST_ASSERT_FALSE(q.hasItems());
}

void test_notification_string_truncation() {
  NotificationQueue q;
  char longTitle[100] = "This title is far too long for the 31 char buffer and should be truncated";
  char longMsg[200] = "This message exceeds the 63 character limit and must be truncated to prevent buffer overflows in embedded";

  q.add(NOTIFY_GENERIC, longTitle, longMsg);
  Notification* n = q.current();
  TEST_ASSERT_NOT_NULL(n);
  TEST_ASSERT_EQUAL('\0', n->title[31]);
  TEST_ASSERT_EQUAL('\0', n->message[63]);
  TEST_ASSERT_TRUE(strlen(n->title) <= 31);
  TEST_ASSERT_TRUE(strlen(n->message) <= 63);
}

void test_notification_null_strings() {
  NotificationQueue q;
  TEST_ASSERT_TRUE(q.add(NOTIFY_SYSTEM, nullptr, nullptr));
  Notification* n = q.current();
  TEST_ASSERT_NOT_NULL(n);
  TEST_ASSERT_EQUAL('\0', n->title[0]);
  TEST_ASSERT_EQUAL('\0', n->message[0]);
}

// ===== ANIMATION TICK ENGINE TESTS =====

void test_tick_draws_frame_on_first_call() {
  MockCanvas canvas;
  // millis() = 0, lastTick = 0, so delay check passes
  bool drew = animationManager.tick(EMOTION_IDLE, canvas);
  TEST_ASSERT_TRUE(drew);
  // Should have CLEAR + draw calls + FLUSH
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

  // Draw frame 0
  stubSetMillis(0);
  animationManager.tick(EMOTION_HAPPY, canvas);

  // Draw frame 1
  stubSetMillis(31);
  animationManager.tick(EMOTION_HAPPY, canvas);

  // Draw frame 2
  stubSetMillis(62);
  animationManager.tick(EMOTION_HAPPY, canvas);

  // Verify multiple frames drawn (3 CLEAR + 3 FLUSH = at least 6 calls)
  TEST_ASSERT_TRUE(canvas.callCount() >= 6);
}

void test_tick_loop_once_holds_last_frame() {
  MockCanvas canvas;
  animationManager.resetAnimation(EMOTION_NOTIFICATION);

  // Advance through all 86 frames of NOTIFICATION
  for (int i = 0; i < 100; i++) {
    stubSetMillis((unsigned long)i * 51);
    animationManager.tick(EMOTION_NOTIFICATION, canvas);
  }

  // Should not crash, should have drawn many times
  TEST_ASSERT_TRUE(canvas.callCount() > 0);
}

void test_tick_returns_false_for_unknown_emotion() {
  MockCanvas canvas;
  bool drew = animationManager.tick((EmotionState)99, canvas);
  TEST_ASSERT_FALSE(drew);
}

// ===== DRAW FUNCTION TESTS (via MockCanvas) =====

void test_draw_idle_draws_eyes() {
  MockCanvas canvas;
  drawIdle(canvas, 0, nullptr);

  // IDLE draws two eyes via fillRoundRect
  int first = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(first >= 0);

  // Second eye
  int second = canvas.findCall(DrawCall::FILL_RRECT, first + 1);
  TEST_ASSERT_TRUE(second >= 0);
}

void test_draw_blink_draws_narrow_eyes() {
  MockCanvas canvas;
  drawBlink(canvas, 0, nullptr);

  int idx = canvas.findCall(DrawCall::FILL_RRECT);
  TEST_ASSERT_TRUE(idx >= 0);
  // Blink eyes should be short (height ~4)
  TEST_ASSERT_TRUE(canvas.call(idx).h <= 6);
}

void test_draw_happy_frame0_has_content() {
  MockCanvas canvas;
  drawHappy(canvas, 0, nullptr);
  TEST_ASSERT_TRUE(canvas.callCount() > 0);
}

void test_draw_notification_uses_context() {
  MockCanvas canvas;
  NotificationContext ctx = {"Alert", "Hello World"};
  drawNotification(canvas, 40, &ctx);

  // Should have text draws for title/message
  int textIdx = canvas.findCall(DrawCall::TEXT);
  TEST_ASSERT_TRUE(textIdx >= 0);
}

void test_draw_github_stats_with_data() {
  MockCanvas canvas;
  GitHubStatsContext ctx = {};
  ctx.hasData = true;
  ctx.username = "testuser";
  ctx.repos = 25;
  ctx.followers = 100;
  ctx.contributions = 500;
  ctx.commits = 300;
  ctx.prs = 20;
  ctx.issues = 10;
  ctx.stars = 50;
  ctx.following = 15;

  drawGitHubStats(canvas, 0, &ctx);
  TEST_ASSERT_TRUE(canvas.callCount() > 0);
}

void test_all_emotions_draw_without_crash() {
  MockCanvas canvas;
  DrawFrameFn drawFns[] = {
    drawIdle, drawBlink, drawHappy, drawSleepy, drawExcited,
    drawSad, drawAngry, drawConfused, drawThinking, drawLove,
    drawSurprised, drawDead, drawMusic, drawCoding
  };

  for (int fn = 0; fn < 14; fn++) {
    for (int frame = 0; frame < 51; frame++) {
      canvas.reset();
      drawFns[fn](canvas, frame, nullptr);
      // Just verify no crash and some drawing happened on frame 0
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

  // Emotion registry
  RUN_TEST(test_registry_add_and_get);
  RUN_TEST(test_registry_rejects_duplicate);
  RUN_TEST(test_registry_get_name_unknown);
  RUN_TEST(test_registry_cyclable_excludes_blink);
  RUN_TEST(test_registry_count);

  // Notification queue
  RUN_TEST(test_notification_add_and_retrieve);
  RUN_TEST(test_notification_queue_overflow);
  RUN_TEST(test_notification_clear_and_empty);
  RUN_TEST(test_notification_string_truncation);
  RUN_TEST(test_notification_null_strings);

  // Animation tick engine
  RUN_TEST(test_tick_draws_frame_on_first_call);
  RUN_TEST(test_tick_respects_frame_delay);
  RUN_TEST(test_tick_advances_frame);
  RUN_TEST(test_tick_loop_once_holds_last_frame);
  RUN_TEST(test_tick_returns_false_for_unknown_emotion);

  // Draw functions
  RUN_TEST(test_draw_idle_draws_eyes);
  RUN_TEST(test_draw_blink_draws_narrow_eyes);
  RUN_TEST(test_draw_happy_frame0_has_content);
  RUN_TEST(test_draw_notification_uses_context);
  RUN_TEST(test_draw_github_stats_with_data);
  RUN_TEST(test_all_emotions_draw_without_crash);

  return UNITY_END();
}
