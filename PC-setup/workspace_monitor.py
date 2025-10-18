#!/usr/bin/env python3
"""
SANGI Workspace Activity Monitor - Modular Edition
Monitors PC workspace activity and sends emotion updates to SANGI robot via MQTT

Refactored with clean module separation:
- lib.activity_tracker: Event tracking and statistics
- lib.audio_detector: Multi-method audio playback detection
- lib.emotion_mapper: Priority-based emotion determination
"""

import os
import sys
import json
import time
import psutil
import logging
import subprocess
from pathlib import Path
from datetime import datetime
from logging.handlers import RotatingFileHandler
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

# Import modular library components
from lib import ActivityTracker, AudioDetector, EmotionMapper, NotificationDetector, EMOTIONS

# Try to import pynput for keyboard/mouse tracking (optional)
try:
    from pynput import keyboard, mouse
    PYNPUT_AVAILABLE = True
except ImportError:
    PYNPUT_AVAILABLE = False
    print("⚠️  pynput not available - keyboard/mouse tracking disabled")
    print("   Install with: pip install pynput")


class CodeFileWatcher(FileSystemEventHandler):
    """Watches for code file changes"""
    def __init__(self, tracker, logger, extensions=None):
        self.tracker = tracker
        self.logger = logger
        self.extensions = extensions or {'.py', '.cpp', '.c', '.h', '.js', '.ts', '.java', '.go', '.rs'}
        self.last_event_time = {}
        self.debounce_seconds = 0.5
        
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
            self.logger.debug(f"📝 File changed: {filepath.name}")


class WorkspaceMonitor:
    """Workspace monitor orchestrator - coordinates modules"""
    
    def __init__(self, config_path='config.json'):
        self.config = self._load_config(config_path)
        
        # Setup logging first
        self.logger = self._setup_logging()
        self.logger.info("="*60)
        self.logger.info("SANGI Workspace Monitor - Modular Edition")
        self.logger.info("="*60)
        
        # Initialize modular components
        self.activity_tracker = ActivityTracker()
        self.audio_detector = AudioDetector(logger=self.logger)
        self.emotion_mapper = EmotionMapper(logger=self.logger)
        self.notification_detector = NotificationDetector(
            logger=self.logger,
            callback=self._on_notification
        )
        
        # Monitor state
        self.current_emotion = 'idle'
        self.last_publish_time = time.time()
        self.last_clipboard_check = time.time()
        self.last_git_check = time.time()
        self.last_audio_check = time.time()
        self.message_history = []
        self.observer = None
        
        # MQTT client
        self.mqtt_client = self._init_mqtt()
        
        # Input monitoring
        if PYNPUT_AVAILABLE:
            self._setup_input_monitoring()
        else:
            self.logger.warning("⚠️  Input monitoring disabled (pynput not installed)")
        
    def _load_config(self, config_path):
        """Load configuration"""
        if not os.path.exists(config_path):
            print(f"❌ Config file not found: {config_path}")
            sys.exit(1)
        
        with open(config_path, 'r') as f:
            return json.load(f)
    
    def _setup_logging(self):
        """Setup logging"""
        log_dir = Path(__file__).parent / 'logs'
        log_dir.mkdir(exist_ok=True)
        
        logger = logging.getLogger('SANGI')
        logger.setLevel(logging.DEBUG)
        logger.handlers = []
        
        # File handler
        handler = RotatingFileHandler(
            log_dir / 'workspace_monitor.log',
            maxBytes=5*1024*1024,
            backupCount=5
        )
        handler.setFormatter(logging.Formatter(
            '%(asctime)s | %(levelname)-8s | %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        ))
        logger.addHandler(handler)
        
        # Console handler
        console = logging.StreamHandler()
        console.setLevel(logging.INFO)
        console.setFormatter(logging.Formatter('%(message)s'))
        logger.addHandler(console)
        
        return logger
    
    def _setup_input_monitoring(self):
        """Setup keyboard and mouse listeners"""
        self.logger.info("🖱️  Setting up input monitoring...")
        
        def on_key_press(key):
            self.activity_tracker.add_keyboard_event()
        
        def on_mouse_move(x, y):
            self.activity_tracker.add_mouse_event()
        
        def on_mouse_click(x, y, button, pressed):
            if pressed:
                self.activity_tracker.add_mouse_event()
        
        # Start listeners in daemon threads
        keyboard_listener = keyboard.Listener(on_press=on_key_press)
        keyboard_listener.daemon = True
        keyboard_listener.start()
        
        mouse_listener = mouse.Listener(
            on_move=on_mouse_move,
            on_click=on_mouse_click
        )
        mouse_listener.daemon = True
        mouse_listener.start()
        
        self.logger.info("✅ Input monitoring active")
        
    def _init_mqtt(self):
        """Initialize AWS IoT MQTT client"""
        try:
            self.logger.info("📡 Initializing MQTT connection...")
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
            self.logger.info(f"✅ Connected to AWS IoT Core")
            return client
        except Exception as e:
            self.logger.error(f"❌ MQTT connection failed: {e}")
            raise
    
    def _on_notification(self, notif_type, title, message):
        """Callback for when a notification is detected"""
        try:
            payload = json.dumps({
                'type': notif_type,
                'title': title,
                'message': message,
                'timestamp': int(time.time())
            })
            
            self.mqtt_client.publish('sangi/notification/push', payload, 1)
            self.logger.info(f"📬 Notification sent: [{notif_type}] {title}")
        except Exception as e:
            self.logger.error(f"Failed to send notification: {e}")
    
    def check_clipboard_activity(self):
        """Check for clipboard changes (copy/paste activity)"""
        try:
            result = subprocess.run(['xclip', '-o', '-selection', 'clipboard'], 
                                  capture_output=True, text=True, timeout=0.5)
            current_clipboard = result.stdout
            
            if current_clipboard and current_clipboard != self.activity_tracker.last_clipboard:
                self.activity_tracker.last_clipboard = current_clipboard
                self.activity_tracker.add_clipboard_event()
                self.logger.debug("📋 Clipboard activity detected")
        except (subprocess.TimeoutExpired, FileNotFoundError, Exception):
            pass
    
    def check_git_activity(self):
        """Check for recent git activity"""
        try:
            for path_str in self.config.get('watch_paths', []):
                path = Path(path_str).expanduser()
                if not path.exists():
                    continue
                
                git_dir = path / '.git'
                if not git_dir.exists():
                    continue
                
                head_file = git_dir / 'HEAD'
                if head_file.exists():
                    mtime = head_file.stat().st_mtime
                    if time.time() - mtime < 60:
                        self.activity_tracker.add_git_event('commit')
                        self.logger.debug(f"🔧 Git activity in {path.name}")
        except Exception as e:
            self.logger.debug(f"Git check error: {e}")
    
    def check_vs_code_activity(self):
        """Check if VS Code is active and being used"""
        try:
            for proc in psutil.process_iter(['name', 'cpu_percent']):
                proc_name = proc.info['name'].lower()
                if 'code' in proc_name or 'vscode' in proc_name:
                    cpu = proc.info['cpu_percent']
                    if cpu > 5:
                        self.activity_tracker.add_keyboard_event()
                        return True
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
        return False
    
    def start_file_watcher(self):
        """Start watching workspace for file changes"""
        watch_paths = self.config.get('watch_paths', [])
        
        event_handler = CodeFileWatcher(self.activity_tracker, self.logger)
        self.observer = Observer()
        
        watched_count = 0
        for path_str in watch_paths:
            path = Path(path_str).expanduser()
            if path.exists():
                self.observer.schedule(event_handler, str(path), recursive=True)
                self.logger.info(f"👁️  Watching: {path}")
                watched_count += 1
            else:
                self.logger.warning(f"⚠️  Path not found: {path}")
        
        if watched_count > 0:
            self.observer.start()
            self.logger.info(f"✅ Monitoring {watched_count} workspace path(s)")
        else:
            self.logger.warning("⚠️  No valid watch paths - file monitoring disabled")
    
    def monitor_terminal_activity(self):
        """Check for active terminal/build processes"""
        build_processes = ['gcc', 'g++', 'make', 'cmake', 'cargo', 'npm', 'pip', 'go', 'javac', 'platformio', 'pio']
        
        for proc in psutil.process_iter(['name']):
            try:
                proc_name = proc.info['name'].lower()
                if any(build_proc in proc_name for build_proc in build_processes):
                    self.activity_tracker.add_terminal_command()
                    return True
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass
        return False
    
    def publish_emotion(self, emotion):
        """Publish emotion to SANGI via MQTT"""
        stats = self.activity_tracker.get_stats()
        
        payload = json.dumps({
            'emotion': EMOTIONS[emotion],
            'source': 'pc',
            'timestamp': int(time.time())
        })
        
        try:
            self.mqtt_client.publish('sangi/emotion/set', payload, 1)
            
            # Get dynamic message from emotion mapper
            message = self.emotion_mapper.get_emotion_message(emotion)
            self.logger.info(f"{message} | Activity: {stats['total_activity_per_min']:.1f}/min")
            
            # Track message history
            self.message_history.append({
                'time': datetime.now().isoformat(),
                'emotion': emotion,
                'stats': stats
            })
            if len(self.message_history) > 100:
                self.message_history = self.message_history[-100:]
                
        except Exception as e:
            self.logger.error(f"Failed to publish: {e}")
    
    def publish_activity_stats(self):
        """Publish detailed activity stats"""
        stats = self.activity_tracker.get_stats()
        stats['device'] = 'pc'
        stats['hostname'] = os.uname().nodename
        stats['timestamp'] = int(time.time())
        
        payload = json.dumps(stats)
        try:
            self.mqtt_client.publish('workspace/pc/activity', payload, 0)
            self.logger.debug(f"📊 Stats published: {stats['total_activity_per_min']:.1f} events/min")
        except Exception as e:
            self.logger.error(f"Failed to publish stats: {e}")
    
    def run(self):
        """Main monitoring loop"""
        self.logger.info("🚀 SANGI Workspace Monitor started")
        self.logger.info(f"📡 Device: {self.config['client_id']}")
        
        self.start_file_watcher()
        
        loop_counter = 0
        
        try:
            while True:
                loop_counter += 1
                
                # Periodic checks
                if loop_counter % 6 == 0:  # Every 30 seconds
                    self.check_clipboard_activity()
                    self.check_git_activity()
                
                # Audio detection every 10 seconds
                if loop_counter % 2 == 0:
                    is_playing = self.audio_detector.is_audio_playing()
                    self.activity_tracker.add_audio_event(is_playing)
                
                # Terminal/build activity
                self.monitor_terminal_activity()
                
                # VS Code activity
                self.check_vs_code_activity()
                
                # Get stats and determine emotion
                stats = self.activity_tracker.get_stats()
                emotion = self.emotion_mapper.determine_emotion(stats)
                
                # Update emotion mapper state for random variations
                self.emotion_mapper.current_emotion = emotion
                
                # Publish if emotion changed
                if emotion != self.current_emotion:
                    self.logger.info(f"🔄 {self.current_emotion} → {emotion}")
                    self.publish_emotion(emotion)
                    self.current_emotion = emotion
                    self.last_publish_time = time.time()
                
                # Publish stats every 30 seconds
                if loop_counter % 6 == 0:
                    self.publish_activity_stats()
                
                time.sleep(5)
                
        except KeyboardInterrupt:
            self.logger.info("\n🛑 Stopping monitor...")
            if self.observer:
                self.observer.stop()
                self.observer.join()
            self.mqtt_client.disconnect()
            self.logger.info("✅ Disconnected")
            self.logger.info("👋 Goodbye!")


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
