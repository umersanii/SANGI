#!/usr/bin/env python3
"""
Test Notification Detector (Local Only)
Tests the D-Bus notification detection without requiring MQTT/AWS IoT connection
"""

import pytest

# NotificationDetector behavior has been moved to the standalone pi-notification-service.
# This test is retained for reference but skipped by default.


@pytest.mark.skip("NotificationDetector moved to standalone pi-notification-service")
def test_notification_detector_import():
    from lib.notification_detector import NotificationDetector
    assert NotificationDetector is not None

# Add lib directory to path
sys.path.insert(0, str(Path(__file__).parent))

try:
    from lib.notification_detector import NotificationDetector
except ImportError as e:
    print(f"❌ Failed to import NotificationDetector: {e}")
    print("💡 Make sure you're in the PC-setup directory")
    sys.exit(1)


def test_notification_capture():
    pytest.skip("Notification capture is now provided by pi-notification-service; run tests there if needed")


if __name__ == '__main__':
    main()
