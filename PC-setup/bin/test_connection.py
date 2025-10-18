#!/usr/bin/env python3
"""
Test AWS IoT connection for SANGI PC Monitor
Run this after setting up certificates to verify connectivity
"""

import json
import sys
import time
from pathlib import Path
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

def test_connection(config_path='config.json'):
    """Test MQTT connection to AWS IoT Core"""
    
    # Load config
    with open(config_path, 'r') as f:
        config = json.load(f)
    
    print("=" * 60)
    print("🔧 SANGI PC Monitor - Connection Test")
    print("=" * 60)
    
    # Verify files exist
    print("\n📁 Checking certificate files...")
    cert_files = {
        'Certificate': config['certificate_path'],
        'Private Key': config['private_key_path'],
        'Root CA': config['root_ca_path']
    }
    
    for name, path in cert_files.items():
        if Path(path).exists():
            print(f"  ✅ {name}: {path}")
        else:
            print(f"  ❌ {name}: NOT FOUND - {path}")
            print(f"\n⚠️  Missing certificate file. See AWS_IOT_SETUP.md for setup instructions.")
            return False
    
    # Test connection
    print(f"\n🌐 Connecting to AWS IoT Core...")
    print(f"  Endpoint: {config['endpoint']}")
    print(f"  Client ID: {config['client_id']}")
    
    try:
        client = AWSIoTMQTTClient(config['client_id'])
        client.configureEndpoint(config['endpoint'], 8883)
        client.configureCredentials(
            config['root_ca_path'],
            config['private_key_path'],
            config['certificate_path']
        )
        
        # Connection settings
        client.configureAutoReconnectBackoffTime(1, 32, 20)
        client.configureOfflinePublishQueueing(-1)
        client.configureDrainingFrequency(2)
        client.configureConnectDisconnectTimeout(10)
        client.configureMQTTOperationTimeout(5)
        
        print(f"  Attempting connection (timeout: 10s)...")
        client.connect()
        print(f"  ✅ Connected successfully!\n")
        
        # Test publish
        print("📤 Testing publish to 'sangi/emotion/set'...")
        test_payload = json.dumps({
            'emotion': 1,  # HAPPY
            'source': 'connection_test',
            'timestamp': int(time.time())
        })
        client.publish('sangi/emotion/set', test_payload, 1)
        print(f"  ✅ Published: {test_payload}\n")
        
        # Test subscribe
        print("📥 Testing subscribe to 'sangi/workspace/pc/control'...")
        
        def test_callback(client, userdata, message):
            print(f"  ✅ Received message on {message.topic}")
            print(f"     Payload: {message.payload.decode()}")
        
        client.subscribe('sangi/workspace/pc/control', 1, test_callback)
        print(f"  ✅ Subscribed successfully\n")
        
        # Wait a bit
        print("⏳ Waiting 3 seconds for any incoming messages...")
        time.sleep(3)
        
        # Disconnect
        client.disconnect()
        print("🔌 Disconnected\n")
        
        print("=" * 60)
        print("✅ ALL TESTS PASSED!")
        print("=" * 60)
        print("\nYou can now run the full monitor:")
        print("  python3 workspace_monitor.py")
        print()
        return True
        
    except Exception as e:
        print(f"  ❌ Connection failed!\n")
        print(f"Error: {str(e)}\n")
        print("Common issues:")
        print("  1. Wrong endpoint in config.json")
        print("  2. Certificates not registered in AWS IoT")
        print("  3. Policy doesn't allow client_id or topics")
        print("  4. Firewall blocking port 8883")
        print("\nSee AWS_IOT_SETUP.md for troubleshooting steps.")
        return False

if __name__ == '__main__':
    config_file = sys.argv[1] if len(sys.argv) > 1 else 'config.json'
    success = test_connection(config_file)
    sys.exit(0 if success else 1)
