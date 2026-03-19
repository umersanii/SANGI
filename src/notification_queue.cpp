#include "notification_queue.h"

NotificationQueue::NotificationQueue()
  : count_(0), currentIdx_(0) {
  for (int i = 0; i < MAX_SIZE; i++) {
    items_[i].active = false;
  }
}

bool NotificationQueue::add(NotificationType type, const char* title, const char* msg) {
  if (count_ >= MAX_SIZE) {
    return false;  // Queue full
  }

  // Validate input parameters
  if (!title) title = "";
  if (!msg) msg = "";

  // Find next available slot
  for (int i = 0; i < MAX_SIZE; i++) {
    if (!items_[i].active) {
      items_[i].type = type;

      // Safe string copy with explicit null termination
      strncpy(items_[i].title, title, 31);
      items_[i].title[31] = '\0';

      strncpy(items_[i].message, msg, 63);
      items_[i].message[63] = '\0';

      items_[i].timestamp = millis();
      items_[i].active = true;
      count_++;
      return true;
    }
  }

  return false;
}

Notification* NotificationQueue::current() {
  if (count_ == 0) {
    return nullptr;
  }

  // Find first active notification sequentially
  for (int i = 0; i < MAX_SIZE; i++) {
    if (items_[i].active) {
      currentIdx_ = i;
      return &items_[i];
    }
  }

  // Inconsistency detected: count says we have notifications but none found
  Serial.println("WARNING: Notification count mismatch - resetting");
  count_ = 0;
  return nullptr;
}

void NotificationQueue::clearCurrent() {
  if (currentIdx_ >= MAX_SIZE) {
    Serial.println("ERROR: Invalid notification index");
    return;
  }

  if (items_[currentIdx_].active) {
    items_[currentIdx_].active = false;

    // Prevent underflow
    if (count_ > 0) {
      count_--;
    } else {
      Serial.println("WARNING: Notification count already at 0");
    }

    Serial.printf("Notification cleared. Queue: %d remaining\n", count_);
  }
}
