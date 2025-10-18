#!/usr/bin/env python3
"""
SANGI Workspace Activity Monitor
Monitors PC workspace activity and sends emotion updates to SANGI robot via MQTT
"""

import os
import sys
import json
import time
import psutil
import logging
from pathlib import Path
from datetime import datetime, timedelta
from collections import deque
from logging.handlers import RotatingFileHandler
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

# Emotion mapping (matches SANGI's EmotionState enum)
EMOTIONS = {
    'idle': 0,
    'happy': 1,
    'sad': 2,
    'angry': 3,
    'surprised': 4,
    'confused': 5,
    'sleepy': 6,
    'thinking': 7,
    'excited': 8,
    'love': 9
}

class ActivityTracker:
    """Tracks workspace activity metrics"""
    def __init__(self, window_size_minutes=5):
        self.window_size = timedelta(minutes=window_size_minutes)
        self.file_changes = deque()
        self.terminal_commands = deque()
        self.build_events = deque()
        self.error_events = deque()
        self.last_activity = datetime.now()
        
    def _clean_old_events(self, event_queue):
        """Remove events older than window_size"""
        cutoff = datetime.now() - self.window_size
        while event_queue and event_queue[0] < cutoff:
            event_queue.popleft()
    
    def add_file_change(self, filepath):
        """Record file modification"""
        self.file_changes.append(datetime.now())
        self._clean_old_events(self.file_changes)
        self.last_activity = datetime.now()
        
    def add_terminal_command(self):
        """Record terminal command execution"""
        self.terminal_commands.append(datetime.now())
        self._clean_old_events(self.terminal_commands)
        self.last_activity = datetime.now()
        
    def add_build_event(self, success=True):
        """Record build/compile event"""
        self.build_events.append((datetime.now(), success))
        self._clean_old_events(self.build_events)
        self.last_activity = datetime.now()
        
    def add_error_event(self):
        """Record error occurrence"""
        self.error_events.append(datetime.now())
        self._clean_old_events(self.error_events)
        
    def get_stats(self):
        """Get current activity statistics"""
        self._clean_old_events(self.file_changes)
        self._clean_old_events(self.terminal_commands)
        self._clean_old_events(self.error_events)
        
        idle_minutes = (datetime.now() - self.last_activity).total_seconds() / 60
        
        return {
            'file_changes_per_min': len(self.file_changes) / 5,
            'terminal_commands_per_min': len(self.terminal_commands) / 5,
            'total_errors': len(self.error_events),
            'idle_minutes': idle_minutes,
            'recent_build_success': self._get_recent_build_status()
        }
    
    def _get_recent_build_status(self):
        """Check if recent builds were successful"""
        if not self.build_events:
            return None
        recent_builds = [success for ts, success in self.build_events if ts > datetime.now() - timedelta(minutes=2)]
        if not recent_builds:
            return None
        return all(recent_builds)

class CodeFileWatcher(FileSystemEventHandler):
    """Watches for code file changes"""
    def __init__(self, tracker, extensions=None):
        self.tracker = tracker
        self.extensions = extensions or {'.py', '.cpp', '.c', '.h', '.js', '.ts', '.java', '.go', '.rs'}
        self.last_event_time = {}
        self.debounce_seconds = 1  # Ignore rapid successive events
        
    def on_modified(self, event):
        if event.is_directory:
            return
        
        filepath = Path(event.src_path)
        if filepath.suffix in self.extensions:
            # Debounce rapid events
            now = time.time()
            if filepath in self.last_event_time:
                if now - self.last_event_time[filepath] < self.debounce_seconds:
                    return
            
            self.last_event_time[filepath] = now
            self.tracker.add_file_change(str(filepath))
            print(f"📝 File changed: {filepath.name}")

class WorkspaceMonitor:
    """Main monitoring class that tracks workspace activity and sends updates to SANGI"""
    
    def __init__(self, config_path='config.json'):
        self.config = self._load_config(config_path)
        self.activity_tracker = ActivityTracker()
        self.last_emotion = 'idle'
        self.last_publish_time = time.time()
        self.message_history = []
        
        # Setup logging
        self.logger = self._setup_logging()
        self.logger.info("="*60)
        self.logger.info("SANGI Workspace Monitor Starting")
        self.logger.info("="*60)
        
        # MQTT client
        self.mqtt_client = self._init_mqtt()
        
        # Emotion mappings
        self.emotions = {
            'idle': 0,
            'happy': 1,
            'sad': 2,
            'angry': 3,
            'surprised': 4,
            'confused': 5,
            'sleepy': 6,
            'thinking': 7,
            'excited': 8,
            'love': 9
        }
        
    def _init_mqtt(self):
        """Initialize AWS IoT MQTT client"""
        client = AWSIoTMQTTClient(self.config['client_id'])
        client.configureEndpoint(self.config['endpoint'], 8883)
        client.configureCredentials(
            self.config['root_ca_path'],
            self.config['private_key_path'],
            self.config['certificate_path']
        )
        
        # Connection settings
        client.configureAutoReconnectBackoffTime(1, 32, 20)
        client.configureOfflinePublishQueueing(-1)
        client.configureDrainingFrequency(2)
        client.configureConnectDisconnectTimeout(10)
        client.configureMQTTOperationTimeout(5)
        
        client.connect()
        print("✅ Connected to AWS IoT Core")
        return client
    
    def start_file_watcher(self):
        """Start watching workspace for file changes"""
        watch_paths = self.config.get('watch_paths', [os.path.expanduser('~/workspace')])
        
        event_handler = CodeFileWatcher(self.tracker)
        self.observer = Observer()
        
        for path in watch_paths:
            if os.path.exists(path):
                self.observer.schedule(event_handler, path, recursive=True)
                print(f"👁️  Watching: {path}")
        
        self.observer.start()
    
    def monitor_terminal_activity(self):
        """Check for active terminal/build processes"""
        # Look for common build/compile processes
        build_processes = ['gcc', 'g++', 'make', 'cmake', 'cargo', 'npm', 'pip', 'go', 'javac', 'platformio']
        
        for proc in psutil.process_iter(['name']):
            try:
                proc_name = proc.info['name'].lower()
                if any(build_proc in proc_name for build_proc in build_processes):
                    self.tracker.add_terminal_command()
                    return True
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass
        return False
    
    def determine_emotion(self):
        """Map activity metrics to SANGI emotion"""
        stats = self.tracker.get_stats()
        
        # High file activity → Excited
        if stats['file_changes_per_min'] > 3:
            return 'excited'
        
        # Build/compile activity → Thinking
        if stats['terminal_commands_per_min'] > 1:
            return 'thinking'
        
        # Recent build failure → Confused/Sad
        if stats['recent_build_success'] is False:
            return 'confused'
        
        # Recent build success → Happy/Love
        if stats['recent_build_success'] is True:
            return 'love'
        
        # High error rate → Angry
        if stats['total_errors'] > 5:
            return 'angry'
        
        # Long idle → Sleepy
        if stats['idle_minutes'] > 10:
            return 'sleepy'
        
        # Moderate activity → Happy
        if stats['file_changes_per_min'] > 0:
            return 'happy'
        
        # Default → Idle
        return 'idle'
    
    def publish_emotion(self, emotion):
        """Publish emotion to SANGI via MQTT"""
        if emotion == self.current_emotion:
            return  # No change
        
        payload = json.dumps({
            'emotion': EMOTIONS[emotion],
            'source': 'pc_monitor',
            'timestamp': int(time.time())
        })
        
        self.mqtt_client.publish('sangi/emotion/set', payload, 1)
        print(f"📤 SANGI emotion → {emotion.upper()}")
        self.current_emotion = emotion
    
    def publish_activity_stats(self):
        """Publish detailed activity stats"""
        stats = self.tracker.get_stats()
        stats['device'] = 'pc'
        stats['timestamp'] = int(time.time())
        
        payload = json.dumps(stats)
        self.mqtt_client.publish('workspace/pc/activity', payload, 0)
    
    def run(self):
        """Main monitoring loop"""
        print("🚀 SANGI Workspace Monitor started")
        print(f"📡 Device ID: {self.config['client_id']}")
        
        self.start_file_watcher()
        
        try:
            while True:
                # Check terminal activity
                self.monitor_terminal_activity()
                
                # Determine and publish emotion
                emotion = self.determine_emotion()
                self.publish_emotion(emotion)
                
                # Publish detailed stats every 30 seconds
                if int(time.time()) % 30 == 0:
                    self.publish_activity_stats()
                
                time.sleep(5)  # Check every 5 seconds
                
        except KeyboardInterrupt:
            print("\n🛑 Stopping monitor...")
            if self.observer:
                self.observer.stop()
                self.observer.join()
            self.mqtt_client.disconnect()
            print("✅ Disconnected")

def main():
    if len(sys.argv) > 1:
        config_path = sys.argv[1]
    else:
        config_path = os.path.join(os.path.dirname(__file__), 'config.json')
    
    if not os.path.exists(config_path):
        print(f"❌ Config file not found: {config_path}")
        print("💡 Create config.json from config.json.template")
        sys.exit(1)
    
    monitor = WorkspaceMonitor(config_path)
    monitor.run()

if __name__ == '__main__':
    main()
