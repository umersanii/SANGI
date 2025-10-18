#!/usr/bin/env python3
"""
Test Notification System
Sends sample notifications to SANGI via MQTT to test the notification detection system
"""

import json
import time
import sys
import os

try:
    from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
except ImportError:
    print("❌ AWS IoT SDK not installed")
    print("   Install with: pip install AWSIoTPythonSDK")
    sys.exit(1)


def send_notification(client, notif_type, title, message):
    """Send a test notification to SANGI"""
    payload = json.dumps({
        'type': notif_type,
        'title': title,
        'message': message,
        'timestamp': int(time.time())
    })
    
    print(f"📤 Sending [{notif_type}] {title}: {message}")
    client.publish('sangi/notification/push', payload, 1)
    print("✅ Notification sent!")


def main():
    # Load config
    config_path = os.path.join(os.path.dirname(__file__), 'config.json')
    
    if not os.path.exists(config_path):
        print(f"❌ Config file not found: {config_path}")
        print("💡 Create config.json from config.json.template")
        sys.exit(1)
    
    with open(config_path, 'r') as f:
        config = json.load(f)
    
    # Initialize MQTT client
    print("📡 Connecting to AWS IoT Core...")
    # Use base client ID (same as workspace monitor)
    client = AWSIoTMQTTClient(config['client_id'])
    client.configureEndpoint(config['endpoint'], 8883)
    client.configureCredentials(
        config['root_ca_path'],
        config['private_key_path'],
        config['certificate_path']
    )
    
    client.configureAutoReconnectBackoffTime(1, 32, 20)
    client.configureOfflinePublishQueueing(-1)
    client.configureDrainingFrequency(2)
    client.configureConnectDisconnectTimeout(30)
    client.configureMQTTOperationTimeout(10)
    
    try:
        client.connect()
        print("✅ Connected to AWS IoT Core\n")
    except Exception as e:
        print(f"❌ Connection failed: {e}")
        print("\n💡 Make sure:")
        print("   1. AWS IoT credentials are configured in config.json")
        print("   2. Certificates are in the correct paths")
        print("   3. Your endpoint URL is correct")
        print("   4. You have internet connectivity")
        print("\nSee docs/MQTT_SETUP.md for setup instructions")
        sys.exit(1)
    
    # Test different notification types (only ones you actually use)
    test_notifications = [
        ('discord', 'friend_username', 'new message'),
        ('discord', 'another_user', 'new message'),
        ('system', 'System', 'Update available'),
        ('generic', 'Alert', 'Something happened!')
    ]
    
    print("🎯 Sending test notifications...\n")
    
    for notif_type, title, message in test_notifications:
        send_notification(client, notif_type, title, message)
        print(f"⏳ Waiting 3 seconds before next notification...\n")
        time.sleep(3)
    
    print("✅ All test notifications sent!")
    print("👀 Check SANGI's display for the notification animations")
    
    client.disconnect()


if __name__ == '__main__':
    main()
