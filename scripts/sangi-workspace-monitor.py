#!/usr/bin/env python3
"""
SANGI Workspace Activity Monitor
Monitors PC/Pi activity (CPU, keyboard, mouse) and publishes to AWS IoT Core MQTT
"""

import json
import time
import psutil
import platform
import ssl
import argparse
from datetime import datetime
import paho.mqtt.client as mqtt

# ===== CONFIGURATION =====
# Device identification (change to "pi" for Raspberry Pi)
DEVICE_NAME = "pc"  # Options: "pc" or "pi"

# AWS IoT Core settings (REPLACE WITH YOUR VALUES)
AWS_IOT_ENDPOINT = "your-endpoint-ats.iot.region.amazonaws.com"
THINGNAME = "SANGI_Robot"

# Certificate paths (adjust to your setup)
CERT_DIR = "/home/your-username/sangi-workspace-monitor"
ROOT_CA = f"{CERT_DIR}/AmazonRootCA1.pem"
CERTIFICATE = f"{CERT_DIR}/SANGI.cert.pem"
PRIVATE_KEY = f"{CERT_DIR}/SANGI.private.key"

# MQTT Topics
ACTIVITY_TOPIC = f"workspace/{DEVICE_NAME}/activity"
STATUS_TOPIC = f"workspace/{DEVICE_NAME}/status"

# Monitoring settings
PUBLISH_INTERVAL = 5  # seconds
CPU_SAMPLE_INTERVAL = 1  # seconds for CPU averaging
INPUT_TIMEOUT = 5  # seconds before considering input inactive

# Activity score weights
CPU_WEIGHT = 0.4  # 40% weight
KEYBOARD_WEIGHT = 0.3  # 30% weight
MOUSE_WEIGHT = 0.3  # 30% weight

# ===== GLOBALS =====
last_keyboard_activity = 0
last_mouse_activity = 0
mqtt_client = None
debug_mode = False

# ===== INPUT MONITORING (Linux-specific) =====
try:
    from Xlib import display, X
    from Xlib.ext import record
    from Xlib.protocol import rq
    
    HAS_XLIB = True
    display_obj = display.Display()
    
    def keyboard_callback(reply):
        global last_keyboard_activity
        if reply.category == record.FromServer:
            if reply.client_swapped:
                return
            if not len(reply.data) or reply.data[0] < 2:
                return
            data = reply.data
            while len(data):
                event, data = rq.EventField(None).parse_binary_value(
                    data, display_obj.display, None, None
                )
                if event.type == X.KeyPress:
                    last_keyboard_activity = time.time()
    
    def mouse_callback(reply):
        global last_mouse_activity
        if reply.category == record.FromServer:
            if reply.client_swapped:
                return
            if not len(reply.data) or reply.data[0] < 2:
                return
            data = reply.data
            while len(data):
                event, data = rq.EventField(None).parse_binary_value(
                    data, display_obj.display, None, None
                )
                if event.type in [X.ButtonPress, X.MotionNotify]:
                    last_mouse_activity = time.time()
    
except ImportError:
    HAS_XLIB = False
    print("Warning: python-xlib not installed. Keyboard/mouse tracking disabled.")
    print("Install with: pip3 install python-xlib")

# ===== ACTIVE WINDOW DETECTION =====
def get_active_window():
    """Get the title of the currently active window"""
    try:
        if platform.system() == "Linux":
            import subprocess
            result = subprocess.run(
                ['xdotool', 'getactivewindow', 'getwindowname'],
                capture_output=True,
                text=True,
                timeout=1
            )
            return result.stdout.strip() if result.returncode == 0 else "Unknown"
        elif platform.system() == "Darwin":  # macOS
            import subprocess
            script = 'tell application "System Events" to get name of first application process whose frontmost is true'
            result = subprocess.run(['osascript', '-e', script], capture_output=True, text=True)
            return result.stdout.strip()
        else:
            return "Unknown"
    except Exception as e:
        if debug_mode:
            print(f"Error getting active window: {e}")
        return "Unknown"

# ===== ACTIVITY CALCULATION =====
def calculate_activity_score():
    """Calculate activity score from 0-10 based on CPU, keyboard, and mouse activity"""
    current_time = time.time()
    
    # CPU component (0-4 points)
    cpu_percent = psutil.cpu_percent(interval=CPU_SAMPLE_INTERVAL)
    cpu_score = (cpu_percent / 100.0) * 4
    
    # Keyboard component (0-3 points)
    keyboard_active = (current_time - last_keyboard_activity) < INPUT_TIMEOUT
    keyboard_score = 3 if keyboard_active else 0
    
    # Mouse component (0-3 points)
    mouse_active = (current_time - last_mouse_activity) < INPUT_TIMEOUT
    mouse_score = 3 if mouse_active else 0
    
    # Calculate total score (0-10)
    total_score = cpu_score + keyboard_score + mouse_score
    total_score = min(10, max(0, int(total_score)))  # Clamp to 0-10
    
    if debug_mode:
        print(f"Activity breakdown: CPU={cpu_score:.1f}, KBD={keyboard_score}, MOUSE={mouse_score} → Total={total_score}")
    
    return {
        "score": total_score,
        "cpu_percent": cpu_percent,
        "keyboard_active": keyboard_active,
        "mouse_active": mouse_active
    }

# ===== MQTT CALLBACKS =====
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"✓ Connected to AWS IoT Core")
        # Publish initial status
        status_msg = json.dumps({
            "device": DEVICE_NAME,
            "status": "online",
            "timestamp": int(time.time())
        })
        client.publish(STATUS_TOPIC, status_msg, qos=1)
        print(f"✓ Published online status to {STATUS_TOPIC}")
    else:
        print(f"✗ Connection failed with code {rc}")
        print("Error codes: 0=Success, 1=Protocol, 2=Client ID, 3=Server unavailable, 4=Credentials, 5=Not authorized")

def on_disconnect(client, userdata, rc):
    if rc != 0:
        print(f"✗ Unexpected disconnection (rc={rc}). Reconnecting...")

def on_publish(client, userdata, mid):
    if debug_mode:
        print(f"Message {mid} published")

# ===== MAIN MONITORING LOOP =====
def main():
    global mqtt_client, debug_mode
    
    # Parse command line arguments
    parser = argparse.ArgumentParser(description="SANGI Workspace Activity Monitor")
    parser.add_argument("--debug", action="store_true", help="Enable debug output")
    parser.add_argument("--device", choices=["pc", "pi"], default=DEVICE_NAME, 
                       help="Device name (pc or pi)")
    args = parser.parse_args()
    
    debug_mode = args.debug
    global DEVICE_NAME, ACTIVITY_TOPIC, STATUS_TOPIC
    DEVICE_NAME = args.device
    ACTIVITY_TOPIC = f"workspace/{DEVICE_NAME}/activity"
    STATUS_TOPIC = f"workspace/{DEVICE_NAME}/status"
    
    print(f"""
╔════════════════════════════════════════╗
║  SANGI Workspace Activity Monitor      ║
║  Device: {DEVICE_NAME.upper():30s}  ║
╚════════════════════════════════════════╝
""")
    
    # Setup MQTT client
    mqtt_client = mqtt.Client(client_id=f"{DEVICE_NAME}-monitor")
    mqtt_client.on_connect = on_connect
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.on_publish = on_publish
    
    # Configure TLS
    try:
        mqtt_client.tls_set(
            ca_certs=ROOT_CA,
            certfile=CERTIFICATE,
            keyfile=PRIVATE_KEY,
            tls_version=ssl.PROTOCOL_TLSv1_2
        )
        print(f"✓ TLS configured with certificates")
    except Exception as e:
        print(f"✗ TLS setup failed: {e}")
        return
    
    # Connect to AWS IoT Core
    try:
        print(f"Connecting to {AWS_IOT_ENDPOINT}:8883...")
        mqtt_client.connect(AWS_IOT_ENDPOINT, 8883, 60)
        mqtt_client.loop_start()
    except Exception as e:
        print(f"✗ Connection failed: {e}")
        return
    
    # Give connection time to establish
    time.sleep(2)
    
    print(f"\n✓ Monitoring started")
    print(f"  - Publishing to: {ACTIVITY_TOPIC}")
    print(f"  - Interval: {PUBLISH_INTERVAL}s")
    print(f"  - Input tracking: {'Enabled' if HAS_XLIB else 'Disabled'}")
    print(f"\nPress Ctrl+C to stop\n")
    
    try:
        while True:
            # Calculate activity
            activity_data = calculate_activity_score()
            active_window = get_active_window()
            
            # Build message
            message = {
                "device": DEVICE_NAME,
                "timestamp": int(time.time()),
                "cpu_percent": round(activity_data["cpu_percent"], 1),
                "active_window": active_window,
                "keyboard_active": activity_data["keyboard_active"],
                "mouse_active": activity_data["mouse_active"],
                "activity_score": activity_data["score"]
            }
            
            # Publish to MQTT
            payload = json.dumps(message)
            result = mqtt_client.publish(ACTIVITY_TOPIC, payload, qos=0)
            
            # Display status
            timestamp = datetime.now().strftime("%H:%M:%S")
            kbd_indicator = "🎹" if activity_data["keyboard_active"] else "⌨️"
            mouse_indicator = "🖱️" if activity_data["mouse_active"] else "🖲️"
            
            print(f"[{timestamp}] Activity: {activity_data['score']:2d}/10 | "
                  f"CPU: {activity_data['cpu_percent']:5.1f}% | "
                  f"{kbd_indicator} {mouse_indicator} | "
                  f"Window: {active_window[:40]}")
            
            time.sleep(PUBLISH_INTERVAL)
            
    except KeyboardInterrupt:
        print("\n\nShutting down...")
        
        # Publish offline status
        status_msg = json.dumps({
            "device": DEVICE_NAME,
            "status": "offline",
            "timestamp": int(time.time())
        })
        mqtt_client.publish(STATUS_TOPIC, status_msg, qos=1)
        
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
        print("✓ Disconnected cleanly")

if __name__ == "__main__":
    main()
