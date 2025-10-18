#!/usr/bin/env python3
"""
Test Notification Detector (Local Only)
Tests the D-Bus notification detection without requiring MQTT/AWS IoT connection
"""

import sys
import time
import logging
from pathlib import Path

# Add lib directory to path
sys.path.insert(0, str(Path(__file__).parent))

try:
    from lib.notification_detector import NotificationDetector
except ImportError as e:
    print(f"❌ Failed to import NotificationDetector: {e}")
    print("💡 Make sure you're in the PC-setup directory")
    sys.exit(1)


def main():
    """Test the notification detector locally"""
    
    # Setup logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s | %(levelname)-8s | %(message)s',
        datefmt='%H:%M:%S'
    )
    logger = logging.getLogger('NotificationTest')
    
    print("=" * 60)
    print("SANGI Notification Detector - Local Test")
    print("=" * 60)
    print()
    
    # Callback for when notifications are detected
    def on_notification(notif_type, title, message):
        logger.info(f"🔔 [{notif_type.upper()}] {title}")
        logger.info(f"   Message: {message[:50]}")
        print()
    
    # Initialize detector
    detector = NotificationDetector(logger=logger, callback=on_notification)
    
    print("📋 Current Status:")
    print(f"   D-Bus available: {detector.bus is not None}")
    print()
    
    if detector.bus is None:
        print("❌ D-Bus not available!")
        print()
        print("💡 Install D-Bus Python bindings:")
        print("   pip install dbus-python pygobject")
        print()
        print("   Or on Ubuntu/Debian:")
        print("   sudo apt-get install python3-dbus python3-gi")
        return
    
    print("✅ Notification detector initialized!")
    print()
    print("📢 Testing Instructions:")
    print("   1. Open another terminal")
    print("   2. Send test notifications with:")
    print()
    print("      notify-send 'Discord' 'New message from @friend'")
    print("      notify-send 'Slack' 'Meeting in 5 minutes'")
    print("      notify-send 'Email' 'You have 3 unread messages'")
    print("      notify-send 'GitHub' 'PR #42 needs review'")
    print()
    print("   3. Watch for detections below")
    print("   4. Press Ctrl+C to stop")
    print()
    print("-" * 60)
    print("Monitoring notifications...")
    print("-" * 60)
    print()
    
    try:
        # Start monitoring (blocking)
        detector.start_monitoring()
    except KeyboardInterrupt:
        print("\n")
        print("=" * 60)
        print("Stopping notification detector...")
        print("=" * 60)
        print()
        
        # Show statistics
        stats = detector.get_notification_stats()
        if stats:
            print("📊 Session Statistics:")
            print(f"   Total notifications: {stats['total']}")
            print()
            print("   By type:")
            for notif_type, count in stats['by_type'].items():
                print(f"      {notif_type}: {count}")
            print()
        else:
            print("📊 No notifications detected this session")
            print()
        
        print("👋 Goodbye!")


if __name__ == '__main__':
    main()
