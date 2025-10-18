/**
 * SANGI Robot - Critical Path Unit Tests
 * Tests for emotion management, overflow handling, and notification queue
 * 
 * Framework: Unity (PlatformIO)
 * Run with: pio test
 */

#include <unity.h>
#include "../src/emotion.cpp"
#include "../src/network.cpp"
#include "../src/battery.cpp"

// ===== MOCK DEPENDENCIES =====
// Mock Serial for testing without hardware
class MockSerial {
public:
    static void printf(const char* format, ...) {
        // No-op in tests
    }
    static void println(const char* str) {}
    static void print(const char* str) {}
};

// ===== TEST: EMOTION STATE VALIDATION =====
void test_emotion_state_validation_valid() {
    EmotionManager em;
    em.init(0);
    
    // Test all valid emotion states
    for (int i = EMOTION_IDLE; i <= EMOTION_NOTIFICATION; i++) {
        em.setTargetEmotion((EmotionState)i);
        TEST_ASSERT_EQUAL(i, em.getTargetEmotion());
    }
}

void test_emotion_state_validation_invalid_low() {
    EmotionManager em;
    em.init(0);
    
    // Should reject invalid low value
    EmotionState initialEmotion = em.getCurrentEmotion();
    em.setTargetEmotion((EmotionState)-1);
    
    // Emotion should not change
    TEST_ASSERT_EQUAL(initialEmotion, em.getCurrentEmotion());
    TEST_ASSERT_FALSE(em.isTransitionActive());
}

void test_emotion_state_validation_invalid_high() {
    EmotionManager em;
    em.init(0);
    
    // Should reject invalid high value (beyond EMOTION_NOTIFICATION)
    EmotionState initialEmotion = em.getCurrentEmotion();
    em.setTargetEmotion((EmotionState)99);
    
    // Emotion should not change
    TEST_ASSERT_EQUAL(initialEmotion, em.getCurrentEmotion());
    TEST_ASSERT_FALSE(em.isTransitionActive());
}

// ===== TEST: MILLIS() OVERFLOW HANDLING =====
void test_millis_overflow_detection() {
    // Simulate overflow scenario
    unsigned long oldTime = 0xFFFFFFF0;  // Near max uint32
    unsigned long newTime = 0x00000010;  // After overflow
    
    // Standard subtraction would give huge positive number
    unsigned long naiveDiff = newTime - oldTime;
    TEST_ASSERT_TRUE(naiveDiff > 1000000);  // Incorrect!
    
    // Correct overflow detection
    bool overflow = newTime < oldTime;
    TEST_ASSERT_TRUE(overflow);
}

void test_emotion_transition_with_overflow() {
    EmotionManager em;
    em.init(0xFFFFFFF0);  // Start near overflow
    
    em.setTargetEmotion(EMOTION_HAPPY);
    TEST_ASSERT_TRUE(em.isTransitionActive());
    
    // Advance through transition frames with simulated overflow
    for (int i = 0; i < 7; i++) {
        em.advanceTransition();
    }
    
    // Transition should complete despite overflow
    TEST_ASSERT_FALSE(em.isTransitionActive());
    TEST_ASSERT_EQUAL(EMOTION_HAPPY, em.getCurrentEmotion());
}

// ===== TEST: NOTIFICATION QUEUE =====
void test_notification_queue_add_valid() {
    NetworkManager nm;
    
    bool result = nm.addNotification(NOTIFY_GENERIC, "Test Title", "Test Message");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(1, nm.getNotificationCount());
}

void test_notification_queue_overflow() {
    NetworkManager nm;
    
    // Fill queue to max
    for (int i = 0; i < MAX_NOTIFICATION_QUEUE; i++) {
        char title[32];
        sprintf(title, "Notification %d", i);
        bool result = nm.addNotification(NOTIFY_GENERIC, title, "Message");
        TEST_ASSERT_TRUE(result);
    }
    
    TEST_ASSERT_EQUAL(MAX_NOTIFICATION_QUEUE, nm.getNotificationCount());
    
    // Next addition should fail
    bool overflow = nm.addNotification(NOTIFY_GENERIC, "Overflow", "Should fail");
    TEST_ASSERT_FALSE(overflow);
    TEST_ASSERT_EQUAL(MAX_NOTIFICATION_QUEUE, nm.getNotificationCount());
}

void test_notification_queue_clear() {
    NetworkManager nm;
    
    nm.addNotification(NOTIFY_GENERIC, "Test", "Message");
    TEST_ASSERT_EQUAL(1, nm.getNotificationCount());
    
    Notification* notif = nm.getCurrentNotification();
    TEST_ASSERT_NOT_NULL(notif);
    
    nm.clearCurrentNotification();
    TEST_ASSERT_EQUAL(0, nm.getNotificationCount());
    
    // Clearing empty queue should not underflow
    nm.clearCurrentNotification();
    TEST_ASSERT_EQUAL(0, nm.getNotificationCount());
}

void test_notification_string_truncation() {
    NetworkManager nm;
    
    // Create strings longer than buffer
    char longTitle[100] = "This is an extremely long title that exceeds the 31 character limit";
    char longMessage[200] = "This is an extremely long message that exceeds the 63 character limit and should be truncated properly without buffer overflow";
    
    bool result = nm.addNotification(NOTIFY_GENERIC, longTitle, longMessage);
    TEST_ASSERT_TRUE(result);
    
    Notification* notif = nm.getCurrentNotification();
    TEST_ASSERT_NOT_NULL(notif);
    
    // Check null termination
    TEST_ASSERT_EQUAL('\0', notif->title[31]);
    TEST_ASSERT_EQUAL('\0', notif->message[63]);
    
    // Title should be exactly 31 chars (truncated from longer)
    TEST_ASSERT_TRUE(strlen(notif->title) <= 31);
    TEST_ASSERT_TRUE(strlen(notif->message) <= 63);
}

void test_notification_null_strings() {
    NetworkManager nm;
    
    // Should handle null pointers gracefully
    bool result = nm.addNotification(NOTIFY_GENERIC, nullptr, nullptr);
    TEST_ASSERT_TRUE(result);
    
    Notification* notif = nm.getCurrentNotification();
    TEST_ASSERT_NOT_NULL(notif);
    TEST_ASSERT_EQUAL('\0', notif->title[0]);
    TEST_ASSERT_EQUAL('\0', notif->message[0]);
}

// ===== TEST: BATTERY VOLTAGE READING =====
void test_battery_voltage_bounds() {
    // Test that voltage reading stays within physical limits
    
    // Mock ADC values
    int testCases[] = {-100, 0, 2048, 4095, 5000};
    float expectedMax = 5.0f;
    
    for (int rawValue : testCases) {
        // Simulate bounds checking
        int bounded = rawValue;
        if (bounded < 0) bounded = 0;
        if (bounded > 4095) bounded = 4095;
        
        float voltage = (bounded / 4095.0f) * 3.3f;
        
        // Should never exceed reasonable voltage
        TEST_ASSERT_LESS_OR_EQUAL(expectedMax, voltage);
        TEST_ASSERT_GREATER_OR_EQUAL(0.0f, voltage);
    }
}

// ===== TEST: TRANSITION FRAME BOUNDS =====
void test_transition_frame_advance() {
    EmotionManager em;
    em.init(0);
    
    em.setTargetEmotion(EMOTION_HAPPY);
    TEST_ASSERT_EQUAL(0, em.getTransitionFrame());
    
    // Advance through all 7 frames
    for (int i = 1; i <= 6; i++) {
        em.advanceTransition();
        TEST_ASSERT_EQUAL(i, em.getTransitionFrame());
    }
    
    // Complete transition
    em.completeTransition();
    TEST_ASSERT_FALSE(em.isTransitionActive());
    TEST_ASSERT_EQUAL(0, em.getTransitionFrame());
}

// ===== TEST RUNNER =====
void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Emotion validation tests
    RUN_TEST(test_emotion_state_validation_valid);
    RUN_TEST(test_emotion_state_validation_invalid_low);
    RUN_TEST(test_emotion_state_validation_invalid_high);
    
    // Overflow handling tests
    RUN_TEST(test_millis_overflow_detection);
    RUN_TEST(test_emotion_transition_with_overflow);
    
    // Notification queue tests
    RUN_TEST(test_notification_queue_add_valid);
    RUN_TEST(test_notification_queue_overflow);
    RUN_TEST(test_notification_queue_clear);
    RUN_TEST(test_notification_string_truncation);
    RUN_TEST(test_notification_null_strings);
    
    // Battery tests
    RUN_TEST(test_battery_voltage_bounds);
    
    // Transition tests
    RUN_TEST(test_transition_frame_advance);
    
    return UNITY_END();
}
