#ifndef NOTIFICATION_QUEUE_H
#define NOTIFICATION_QUEUE_H

#include <Arduino.h>

// ===== NOTIFICATION TYPES =====
enum NotificationType {
  NOTIFY_GENERIC,
  NOTIFY_DISCORD,
  NOTIFY_SLACK,
  NOTIFY_EMAIL,
  NOTIFY_GITHUB,
  NOTIFY_CALENDAR,
  NOTIFY_SYSTEM
};

// ===== NOTIFICATION STRUCTURE =====
struct Notification {
  NotificationType type;
  char title[32];
  char message[64];
  unsigned long timestamp;
  bool active;
};

// ===== NOTIFICATION QUEUE =====
class NotificationQueue {
public:
  static const int MAX_SIZE = 5;

  NotificationQueue();

  bool add(NotificationType type, const char* title, const char* msg);
  Notification* current();
  void clearCurrent();
  bool hasItems() const { return count_ > 0; }
  int count() const { return count_; }

private:
  Notification items_[MAX_SIZE];
  int count_;
  int currentIdx_;
};

#endif // NOTIFICATION_QUEUE_H
