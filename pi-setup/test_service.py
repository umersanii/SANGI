#!/usr/bin/env python3
"""
Test script for notification service
Sends test notifications to verify the service is working
"""

import json
import sys
import time
from pathlib import Path

# Add lib to path
sys.path.insert(0, str(Path(__file__).parent / 'lib'))

from lib import MQTTPublisher


def main():
    print("====================================")
    print("SANGI Notification Service Test")
    print("====================================")
    print()
    
    # Load config
    try:
        with open('config.json', 'r') as f:
            config = json.load(f)
    except FileNotFoundError:
        print("ERROR: config.json not found!")
        print("Please run setup.sh first")
        sys.exit(1)
    
    # Initialize MQTT publisher
    print("[1/4] Initializing MQTT publisher...")
    mqtt = MQTTPublisher(config)
    
    # Connect
    print("[2/4] Connecting to AWS IoT Core...")
    if not mqtt.connect():
        print("ERROR: Failed to connect to MQTT")
        sys.exit(1)
    
    print("✓ Connected successfully")
    print()
    
    # Send test notifications
    print("[3/4] Sending test notifications...")
    print()
    
    test_notifications = [
        ('system', 'Notification Test', 'Service started'),
        ('discord', 'friend_username', 'new message'),
        ('github', 'PR: umersanii/SANGI', 'Add notification service'),
        ('whatsapp', 'Contact Name', 'Hello there!')
    ]
    
    for i, (notif_type, title, message) in enumerate(test_notifications, 1):
        print(f"  [{i}/{len(test_notifications)}] Sending {notif_type}: {title} - {message}")
        success = mqtt.publish_notification(notif_type, title, message)
        
        if success:
            print(f"      ✓ Sent successfully")
        else:
            print(f"      ✗ Failed to send")
        
        time.sleep(2)
    
    print()
    print("[4/4] Disconnecting...")
    mqtt.disconnect()
    
    print()
    print("====================================")
    print("Test Complete!")
    print("====================================")
    print()
    print("Check SANGI's display - it should have shown 4 notifications")
    print("If notifications didn't appear, check:")
    print("  1. SANGI is powered on and connected to WiFi")
    print("  2. SANGI is subscribed to 'sangi/notification/push'")
    print("  3. AWS IoT policy allows publish to this topic")
    print()


if __name__ == '__main__':
    main()
