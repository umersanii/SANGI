#!/usr/bin/env python3
"""
SANGI Workspace Activity Monitor v2
Monitors PC workspace activity and sends emotion updates to SANGI robot via MQTT
With comprehensive logging and message history tracking
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
        
        # Recent build results
        recent_builds = [success for ts, success in self.build_events if ts > datetime.now() - timedelta(minutes=2)]
        
        return {
            'code_changes_per_min': len(self.file_changes) / max(1, self.window_size.total_seconds() / 60),
            'terminal_commands_per_min': len(self.terminal_commands) / max(1, self.window_size.total_seconds() / 60),
            'error_rate': len(self.error_events) / max(1, len(self.file_changes)) if self.file_changes else 0,
            'idle_minutes': idle_minutes,
            'recent_build_success': all(recent_builds) if recent_builds else None,
            'recent_build_failure': any(not s for s in recent_builds) if recent_builds else None,
        }

class CodeFileHandler(FileSystemEventHandler):
    """Watches for code file changes"""
    def __init__(self, tracker, extensions, logger):
        self.tracker = tracker
        self.extensions = extensions
        self.logger = logger
        
    def on_modified(self, event):
        if not event.is_directory:
            filepath = Path(event.src_path)
            if filepath.suffix in self.extensions:
                self.tracker.add_file_change(str(filepath))
                self.logger.debug(f"File changed: {filepath.name}")

class WorkspaceMonitor:
    """Main monitoring class that tracks workspace activity and sends updates to SANGI"""
    
    def __init__(self, config_path='config.json'):
        self.config = self._load_config(config_path)
        
        # Setup logging FIRST
        self.logger, self.mqtt_logger = self._setup_logging()
        
        self.logger.info("="*60)
        self.logger.info("SANGI Workspace Monitor v2 Starting")
        self.logger.info("="*60)
        
        self.activity_tracker = ActivityTracker()
        self.last_emotion = 'idle'
        self.last_publish_time = time.time()
        
        # Message history
        self.history_file = Path(__file__).parent / 'logs' / 'message_history.json'
        self.message_history = self._load_history()
        
        # MQTT client
        self.mqtt_client = self._init_mqtt()
        
    def _load_config(self, path):
        """Load configuration from JSON file"""
        try:
            with open(path, 'r') as f:
                config = json.load(f)
            print(f"✅ Loaded config from {path}")
            return config
        except FileNotFoundError:
            print(f"❌ Config file not found: {path}")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"❌ Invalid JSON in config: {e}")
            sys.exit(1)
    
    def _setup_logging(self):
        """Setup rotating file logging and console output"""
        log_dir = Path(__file__).parent / 'logs'
        log_dir.mkdir(exist_ok=True)
        
        # Main logger
        logger = logging.getLogger('SANGIMonitor')
        logger.setLevel(logging.DEBUG)
        logger.handlers = []  # Clear any existing handlers
        
        # Rotating file handler (max 5MB per file, keep 5 files)
        log_file = log_dir / 'workspace_monitor.log'
        file_handler = RotatingFileHandler(
            log_file,
            maxBytes=5*1024*1024,  # 5MB
            backupCount=5
        )
        file_handler.setLevel(logging.DEBUG)
        file_formatter = logging.Formatter(
            '%(asctime)s | %(levelname)-8s | %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        file_handler.setFormatter(file_formatter)
        
        # Console handler
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.INFO)
        console_formatter = logging.Formatter(
            '%(asctime)s | %(message)s',
            datefmt='%H:%M:%S'
        )
        console_handler.setFormatter(console_formatter)
        
        logger.addHandler(file_handler)
        logger.addHandler(console_handler)
        
        # MQTT message logger (separate file for communication history)
        mqtt_log_file = log_dir / 'mqtt_messages.log'
        mqtt_logger = logging.getLogger('MQTT')
        mqtt_logger.setLevel(logging.INFO)
        mqtt_logger.handlers = []  # Clear any existing handlers
        
        mqtt_handler = RotatingFileHandler(
            mqtt_log_file,
            maxBytes=10*1024*1024,  # 10MB
            backupCount=10
        )
        mqtt_handler.setFormatter(logging.Formatter(
            '%(asctime)s | %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        ))
        mqtt_logger.addHandler(mqtt_handler)
        
        print(f"📁 Logs directory: {log_dir}")
        print(f"📄 Main log: {log_file}")
        print(f"📡 MQTT log: {mqtt_log_file}")
        
        return logger, mqtt_logger
    
    def _load_history(self):
        """Load previous message history"""
        try:
            if self.history_file.exists():
                with open(self.history_file, 'r') as f:
                    history = json.load(f)
                self.logger.info(f"📜 Loaded {len(history)} historical messages")
                return history
        except Exception as e:
            self.logger.warning(f"Could not load history: {e}")
        return []
    
    def _save_history(self):
        """Save message history to disk"""
        try:
            # Keep last 500 messages
            with open(self.history_file, 'w') as f:
                json.dump(self.message_history[-500:], f, indent=2)
        except Exception as e:
            self.logger.error(f"Could not save history: {e}")
    
    def _init_mqtt(self):
        """Initialize MQTT connection to AWS IoT Core"""
        try:
            self.logger.info("Initializing MQTT connection...")
            
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
            
            self.logger.info(f"Connecting to {self.config['endpoint']}...")
            client.connect()
            self.logger.info(f"✅ Connected to AWS IoT Core as {self.config['client_id']}")
            self.mqtt_logger.info(f"CONNECTED | Client: {self.config['client_id']}")
            
            return client
            
        except Exception as e:
            self.logger.error(f"❌ MQTT connection failed: {e}")
            raise
    
    def determine_emotion(self):
        """Determine emotion based on workspace activity"""
        stats = self.activity_tracker.get_stats()
        
        self.logger.debug(f"Stats: {stats}")
        
        # High activity → Excited
        if stats['code_changes_per_min'] > self.config['monitoring']['high_activity_threshold'] * 2:
            return 'excited'
        
        # Recent build failure → Sad/Confused
        if stats['recent_build_failure']:
            return 'confused' if stats['error_rate'] > 0.3 else 'sad'
        
        # Recent build success → Love/Happy
        if stats['recent_build_success']:
            return 'love' if stats['code_changes_per_min'] > 2 else 'happy'
        
        # High error rate → Confused
        if stats['error_rate'] > 0.3:
            return 'confused'
        
        # Terminal activity → Thinking
        if stats['terminal_commands_per_min'] > 2:
            return 'thinking'
        
        # Long idle → Sleepy
        if stats['idle_minutes'] > self.config['monitoring']['idle_timeout_minutes']:
            return 'sleepy'
        
        # Active but calm → Happy
        if stats['code_changes_per_min'] > self.config['monitoring']['high_activity_threshold']:
            return 'happy'
        
        # Low activity → Idle
        return 'idle'
    
    def publish_emotion_update(self, emotion):
        """Send emotion to SANGI and log it"""
        stats = self.activity_tracker.get_stats()
        
        payload = {
            'emotion': EMOTIONS[emotion],
            'emotion_name': emotion,
            'source': 'workspace_monitor',
            'hostname': os.uname().nodename,
            'timestamp': int(time.time()),
            'stats': {
                'changes_per_min': round(stats['code_changes_per_min'], 2),
                'commands_per_min': round(stats['terminal_commands_per_min'], 2),
                'error_rate': round(stats['error_rate'], 2),
                'idle_minutes': round(stats['idle_minutes'], 1)
            }
        }
        
        topic = 'sangi/emotion/set'
        payload_json = json.dumps(payload)
        
        try:
            self.mqtt_client.publish(topic, payload_json, 1)
            
            # Log to console
            self.logger.info(f"📤 {emotion.upper():8} → SANGI | Activity: {stats['code_changes_per_min']:.1f} changes/min")
            
            # Log to MQTT log
            self.mqtt_logger.info(f"PUBLISH | {topic} | {payload_json}")
            
            # Add to history
            self.message_history.append({
                'timestamp': datetime.now().isoformat(),
                'topic': topic,
                'payload': payload
            })
            self._save_history()
            
        except Exception as e:
            self.logger.error(f"Failed to publish: {e}")
    
    def publish_status(self):
        """Publish monitor status to SANGI"""
        status_payload = {
            'status': 'active',
            'client_id': self.config['client_id'],
            'hostname': os.uname().nodename,
            'uptime_seconds': int(time.time() - self.start_time),
            'last_emotion': self.last_emotion,
            'timestamp': int(time.time())
        }
        
        topic = 'sangi/workspace/pc/status'
        payload_json = json.dumps(status_payload)
        
        try:
            self.mqtt_client.publish(topic, payload_json, 1)
            self.mqtt_logger.info(f"STATUS | {topic} | {payload_json}")
        except Exception as e:
            self.logger.error(f"Failed to publish status: {e}")
    
    def setup_file_watchers(self):
        """Setup file system watchers for code changes"""
        self.logger.info("Setting up file watchers...")
        
        event_handler = CodeFileHandler(
            self.activity_tracker,
            self.config['code_extensions'],
            self.logger
        )
        
        observer = Observer()
        
        for path_str in self.config['watch_paths']:
            path = Path(path_str).expanduser()
            if path.exists():
                observer.schedule(event_handler, str(path), recursive=True)
                self.logger.info(f"👁️  Watching: {path}")
            else:
                self.logger.warning(f"⚠️  Path not found: {path}")
        
        observer.start()
        return observer
    
    def run(self):
        """Main monitoring loop"""
        self.start_time = time.time()
        observer = self.setup_file_watchers()
        
        self.logger.info("📊 Monitoring workspace activity...")
        self.logger.info("Press Ctrl+C to stop")
        
        try:
            while True:
                current_emotion = self.determine_emotion()
                current_time = time.time()
                
                # Publish emotion change immediately
                if current_emotion != self.last_emotion:
                    self.logger.info(f"🔄 Emotion change: {self.last_emotion} → {current_emotion}")
                    self.publish_emotion_update(current_emotion)
                    self.last_emotion = current_emotion
                    self.last_publish_time = current_time
                
                # Periodic status update
                elif current_time - self.last_publish_time > self.config['monitoring']['stats_publish_interval_seconds']:
                    self.publish_status()
                    self.last_publish_time = current_time
                
                time.sleep(self.config['monitoring']['update_interval_seconds'])
                
        except KeyboardInterrupt:
            self.logger.info("\n⏹️  Stopping workspace monitor...")
            observer.stop()
            observer.join()
            self.mqtt_client.disconnect()
            self.logger.info("✅ Disconnected from AWS IoT Core")
            self.logger.info("👋 Goodbye!")

def main():
    """Entry point"""
    config_path = sys.argv[1] if len(sys.argv) > 1 else 'config.json'
    
    try:
        monitor = WorkspaceMonitor(config_path)
        monitor.run()
    except Exception as e:
        print(f"❌ Fatal error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
