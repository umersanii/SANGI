#!/usr/bin/env python3
"""
SANGI Workspace Activity Monitor - Enhanced & Robust Edition
Monitors PC workspace activity and sends emotion updates to SANGI robot via MQTT

Features:
- File system watching (code changes)
- Keyboard & mouse activity tracking
- VS Code / editor process detection
- Clipboard monitoring
- Git activity detection
- CPU usage tracking
- Time-based emotion variations
"""

import os
import sys
import json
import time
import psutil
import logging
import random
import subprocess
from pathlib import Path
from datetime import datetime, timedelta
from collections import deque
from logging.handlers import RotatingFileHandler
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

# Try to import pynput for keyboard/mouse tracking (optional)
try:
    from pynput import keyboard, mouse
    PYNPUT_AVAILABLE = True
except ImportError:
    PYNPUT_AVAILABLE = False
    print("⚠️  pynput not available - keyboard/mouse tracking disabled")
    print("   Install with: pip install pynput")

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
    'love': 9,
    'dead': 10,
    'music': 11,
    'blink': 12
}

class ActivityTracker:
    """Enhanced activity tracker with multiple input sources"""
    def __init__(self, window_size_minutes=5):
        self.window_size = timedelta(minutes=window_size_minutes)
        self.file_changes = deque()
        self.terminal_commands = deque()
        self.build_events = deque()
        self.error_events = deque()
        self.keyboard_events = deque()
        self.mouse_events = deque()
        self.clipboard_events = deque()
        self.git_events = deque()
        self.audio_events = deque()
        self.last_activity = datetime.now()
        self.last_clipboard = ""
        self.is_audio_playing = False
        
    def _clean_old_events(self, event_queue):
        """Remove events older than window_size"""
        cutoff = datetime.now() - self.window_size
        while event_queue and isinstance(event_queue[0], datetime) and event_queue[0] < cutoff:
            event_queue.popleft()
        # Also handle tuples (timestamp, data)
        while event_queue and isinstance(event_queue[0], tuple) and event_queue[0][0] < cutoff:
            event_queue.popleft()
    
    def add_file_change(self, filepath):
        """Record file modification"""
        self.file_changes.append(datetime.now())
        self._clean_old_events(self.file_changes)
        self.last_activity = datetime.now()
        
    def add_keyboard_event(self):
        """Record keyboard activity"""
        self.keyboard_events.append(datetime.now())
        self._clean_old_events(self.keyboard_events)
        self.last_activity = datetime.now()
        
    def add_mouse_event(self):
        """Record mouse activity"""
        self.mouse_events.append(datetime.now())
        self._clean_old_events(self.mouse_events)
        self.last_activity = datetime.now()
        
    def add_clipboard_event(self):
        """Record clipboard activity"""
        self.clipboard_events.append(datetime.now())
        self._clean_old_events(self.clipboard_events)
        self.last_activity = datetime.now()
        
    def add_git_event(self, event_type):
        """Record git activity"""
        self.git_events.append((datetime.now(), event_type))
        self._clean_old_events(self.git_events)
        self.last_activity = datetime.now()
    
    def add_audio_event(self, is_playing):
        """Record audio playback activity"""
        self.audio_events.append((datetime.now(), is_playing))
        self._clean_old_events(self.audio_events)
        self.is_audio_playing = is_playing
        if is_playing:
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
        """Get comprehensive activity statistics"""
        self._clean_old_events(self.file_changes)
        self._clean_old_events(self.terminal_commands)
        self._clean_old_events(self.error_events)
        self._clean_old_events(self.keyboard_events)
        self._clean_old_events(self.mouse_events)
        self._clean_old_events(self.clipboard_events)
        self._clean_old_events(self.git_events)
        self._clean_old_events(self.audio_events)
        
        idle_minutes = (datetime.now() - self.last_activity).total_seconds() / 60
        window_minutes = self.window_size.total_seconds() / 60
        
        # Recent build results
        recent_builds = [success for ts, success in self.build_events if ts > datetime.now() - timedelta(minutes=2)]
        
        # Git activity
        recent_git = [(ts, evt) for ts, evt in self.git_events if ts > datetime.now() - timedelta(minutes=2)]
        
        # Audio playback
        recent_audio = [(ts, playing) for ts, playing in self.audio_events if ts > datetime.now() - timedelta(minutes=1)]
        
        return {
            'file_changes_per_min': len(self.file_changes) / window_minutes,
            'keyboard_events_per_min': len(self.keyboard_events) / window_minutes,
            'mouse_events_per_min': len(self.mouse_events) / window_minutes,
            'clipboard_events_per_min': len(self.clipboard_events) / window_minutes,
            'terminal_commands_per_min': len(self.terminal_commands) / window_minutes,
            'total_errors': len(self.error_events),
            'idle_minutes': idle_minutes,
            'recent_build_success': all(recent_builds) if recent_builds else None,
            'recent_build_failure': any(not s for s in recent_builds) if recent_builds else None,
            'git_activity': len(recent_git),
            'is_audio_playing': self.is_audio_playing,
            'audio_activity': len(recent_audio),
            'total_activity_per_min': (
                len(self.file_changes) + len(self.keyboard_events) + 
                len(self.mouse_events) + len(self.clipboard_events)
            ) / window_minutes
        }

class CodeFileWatcher(FileSystemEventHandler):
    """Watches for code file changes"""
    def __init__(self, tracker, logger, extensions=None):
        self.tracker = tracker
        self.logger = logger
        self.extensions = extensions or {'.py', '.cpp', '.c', '.h', '.js', '.ts', '.java', '.go', '.rs'}
        self.last_event_time = {}
        self.debounce_seconds = 0.5  # Reduced for more responsive tracking
        
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
    """Enhanced workspace monitor with multi-source activity tracking"""
    
    def __init__(self, config_path='config.json'):
        self.config = self._load_config(config_path)
        self.activity_tracker = ActivityTracker()
        self.current_emotion = 'idle'
        self.last_publish_time = time.time()
        self.last_clipboard_check = time.time()
        self.last_git_check = time.time()
        self.last_audio_check = time.time()
        self.last_random_change = time.time()
        self.message_history = []
        self.observer = None
        
        # Setup logging
        self.logger = self._setup_logging()
        self.logger.info("="*60)
        self.logger.info("SANGI Workspace Monitor - Enhanced Edition")
        self.logger.info("="*60)
        
        # MQTT client
        self.mqtt_client = self._init_mqtt()
        
        # Input monitoring
        if PYNPUT_AVAILABLE:
            self._setup_input_monitoring()
        else:
            self.logger.warning("⚠️  Input monitoring disabled (pynput not installed)")
        
        # Emotion mappings
        self.emotions = EMOTIONS
        
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
    
    def check_clipboard_activity(self):
        """Check for clipboard changes (copy/paste activity)"""
        try:
            # Use xclip on Linux
            result = subprocess.run(['xclip', '-o', '-selection', 'clipboard'], 
                                  capture_output=True, text=True, timeout=0.5)
            current_clipboard = result.stdout
            
            if current_clipboard and current_clipboard != self.activity_tracker.last_clipboard:
                # Clipboard changed - user copied something
                self.activity_tracker.last_clipboard = current_clipboard
                self.activity_tracker.add_clipboard_event()
                self.logger.debug("📋 Clipboard activity detected")
        except (subprocess.TimeoutExpired, FileNotFoundError, Exception):
            pass  # Clipboard check failed, skip silently
    
    def check_git_activity(self):
        """Check for recent git activity"""
        try:
            # Check watch paths for git repositories
            for path_str in self.config.get('watch_paths', []):
                path = Path(path_str).expanduser()
                if not path.exists():
                    continue
                
                git_dir = path / '.git'
                if not git_dir.exists():
                    continue
                
                # Check if git HEAD changed recently (commits, checkouts, etc.)
                head_file = git_dir / 'HEAD'
                if head_file.exists():
                    mtime = head_file.stat().st_mtime
                    if time.time() - mtime < 60:  # Changed in last minute
                        self.activity_tracker.add_git_event('commit')
                        self.logger.debug(f"🔧 Git activity in {path.name}")
        except Exception as e:
            self.logger.debug(f"Git check error: {e}")
    
    def check_audio_playback(self):
        """Check if audio is currently playing through system speakers/headphones"""
        is_playing = False
        
        try:
            # Method 1: Check /proc/asound (ALSA - works on most Linux systems)
            # But validate that audio is actually flowing, not just device open
            try:
                pcm_files = Path('/proc/asound').glob('card*/pcm*/sub*/status')
                for status_file in pcm_files:
                    content = status_file.read_text()
                    if 'state: RUNNING' in content:
                        # Get hw_ptr to verify audio is flowing
                        lines = content.split('\n')
                        hw_ptr1 = None
                        owner_pid = None
                        
                        for line in lines:
                            if 'hw_ptr' in line and ':' in line:
                                hw_ptr1 = int(line.split(':')[1].strip())
                            if 'owner_pid' in line and ':' in line:
                                try:
                                    owner_pid = int(line.split(':')[1].strip())
                                except ValueError:
                                    pass
                        
                        # Check if audio is flowing by seeing if hw_ptr advances
                        if hw_ptr1 is not None:
                            time.sleep(0.1)  # Wait 100ms
                            content2 = status_file.read_text()
                            for line in content2.split('\n'):
                                if 'hw_ptr' in line and ':' in line:
                                    hw_ptr2 = int(line.split(':')[1].strip())
                                    # If hw_ptr advanced significantly, audio is playing
                                    # At 48kHz, 100ms = 4800 samples minimum
                                    if hw_ptr2 > hw_ptr1 and (hw_ptr2 - hw_ptr1) > 1000:
                                        # Verify the owner process isn't just PipeWire/PulseAudio idle
                                        if owner_pid:
                                            try:
                                                proc = psutil.Process(owner_pid)
                                                proc_name = proc.name().lower()
                                                # If it's just audio server with no clients, skip
                                                if proc_name in ['pipewire', 'pulseaudio', 'jackd']:
                                                    # Check if audio server has active clients
                                                    children = proc.children()
                                                    if not children:
                                                        continue
                                            except (psutil.NoSuchProcess, psutil.AccessDenied):
                                                pass
                                        
                                        is_playing = True
                                        self.logger.debug("🎵 Audio detected via ALSA (hw_ptr advancing)")
                                        break
                        
                        if is_playing:
                            break
            except (FileNotFoundError, PermissionError, ValueError):
                pass
            
            # Method 2: Check PulseAudio if available
            if not is_playing:
                try:
                    result = subprocess.run(
                        ['pactl', 'list', 'sink-inputs'],
                        capture_output=True,
                        text=True,
                        timeout=1
                    )
                    if 'Sink Input #' in result.stdout:
                        is_playing = True
                        self.logger.debug("🎵 Audio detected via PulseAudio")
                except FileNotFoundError:
                    pass  # pactl not available
            
            # Method 3: Check for media player processes with audio activity
            if not is_playing:
                media_players = {
                    'spotify': 1.0,      # Spotify uses CPU when playing
                    'vlc': 0.5,          # VLC minimal CPU
                    'mpv': 0.5,
                    'mplayer': 0.5,
                    'rhythmbox': 1.0,
                    'clementine': 1.0,
                    'audacious': 0.8,
                    'firefox': 2.0,      # Browser video/audio
                    'chrome': 2.0,
                    'chromium': 2.0,
                }
                
                for proc in psutil.process_iter(['name', 'cpu_percent', 'cmdline']):
                    try:
                        proc_name = proc.info['name'].lower() if proc.info['name'] else ''
                        cmdline = ' '.join(proc.info['cmdline'] or []).lower()
                        
                        # Check process name and command line for media players
                        for player, cpu_threshold in media_players.items():
                            if player in proc_name or player in cmdline:
                                # Check if process is actively using CPU
                                cpu = proc.cpu_percent(interval=0.1)
                                if cpu > cpu_threshold:
                                    is_playing = True
                                    self.logger.debug(f"🎵 Audio detected via {player} ({cpu:.1f}% CPU)")
                                    break
                        
                        if is_playing:
                            break
                            
                    except (psutil.NoSuchProcess, psutil.AccessDenied):
                        pass
            
            # Update tracker
            self.activity_tracker.add_audio_event(is_playing)
            return is_playing
            
        except Exception as e:
            self.logger.debug(f"Audio check error: {e}")
            return False
    
    def check_vs_code_activity(self):
        """Check if VS Code is active and being used"""
        try:
            for proc in psutil.process_iter(['name', 'cpu_percent']):
                proc_name = proc.info['name'].lower()
                if 'code' in proc_name or 'vscode' in proc_name:
                    cpu = proc.info['cpu_percent']
                    if cpu > 5:  # VS Code is actively being used
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
        # Look for common build/compile processes
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
    
    def determine_emotion(self):
        """Enhanced emotion determination with multiple activity sources"""
        stats = self.activity_tracker.get_stats()
        
        # Time-based random variations (prevent stagnation)
        current_time = time.time()
        if current_time - self.last_random_change > 120:  # Every 2 minutes
            if random.random() < 0.3:  # 30% chance of random emotion
                self.last_random_change = current_time
                random_emotions = ['thinking', 'happy', 'excited', 'music']
                emotion = random.choice([e for e in random_emotions if e != self.current_emotion])
                self.logger.debug(f"🎲 Random emotion variation: {emotion}")
                return emotion
        
        # Audio playback detected → Music (high priority)
        if stats['is_audio_playing']:
            return 'music'
        
        # Git activity → Excited or Thinking
        if stats['git_activity'] > 0:
            return 'excited' if stats['total_activity_per_min'] > 5 else 'thinking'
        
        # Build/compile activity → Thinking
        if stats['terminal_commands_per_min'] > 0.5:
            return 'thinking'
        
        # Recent build failure → Confused/Sad
        if stats['recent_build_failure']:
            return 'confused' if stats['total_errors'] > 3 else 'sad'
        
        # Recent build success → Love/Happy
        if stats['recent_build_success']:
            return 'love' if stats['total_activity_per_min'] > 3 else 'happy'
        
        # High total activity (keyboard+mouse+files+clipboard) → Excited
        if stats['total_activity_per_min'] > 10:
            return 'excited'
        
        # High keyboard activity → Thinking (typing code)
        if stats['keyboard_events_per_min'] > 20:
            return 'thinking'
        
        # Clipboard activity → Happy (copy/pasting code)
        if stats['clipboard_events_per_min'] > 0.5:
            return 'happy'
        
        # Moderate file changes → Happy
        if stats['file_changes_per_min'] > 0.3:
            return 'happy'
        
        # High error rate → Confused
        if stats['total_errors'] > 5:
            return 'confused'
        
        # Long idle time → Sleepy
        if stats['idle_minutes'] > 10:
            return 'sleepy'
        
        # Low but present activity → Content/Happy
        if stats['total_activity_per_min'] > 1:
            return 'happy'
        
        # Moderate idle (5-10 min) → Thinking
        if stats['idle_minutes'] > 5:
            return 'thinking'
        
        # Default → Idle
        return 'idle'
    
    def publish_emotion(self, emotion):
        """Publish emotion to SANGI via MQTT"""
        stats = self.activity_tracker.get_stats()
        
        # Simplified payload to reduce size (ESP32 memory constraint)
        payload = json.dumps({
            'emotion': EMOTIONS[emotion],
            'source': 'pc',
            'timestamp': int(time.time())
        })
        
        # Dynamic emotion messages for variety
        emotion_messages = {
            'thinking': [
                "🤔 Deep in thought...",
                "💭 Pondering solutions...",
                "🧠 Brain processing...",
                "💡 Ideas brewing..."
            ],
            'happy': [
                "😊 Feeling productive!",
                "✨ Code flowing nicely!",
                "🎉 Great progress!",
                "😄 Things are clicking!"
            ],
            'excited': [
                "⚡ High energy mode!",
                "🚀 Maximum productivity!",
                "🔥 On fire!",
                "💫 Super active!"
            ],
            'sleepy': [
                "😴 Time for a break?",
                "💤 Winding down...",
                "🌙 Getting quiet...",
                "😪 Low activity detected..."
            ],
            'confused': [
                "❓ Detecting struggles...",
                "🤷 Many errors found...",
                "😵 Debug mode active...",
                "🔍 Investigating issues..."
            ],
            'idle': [
                "😐 Waiting for action...",
                "⏸️  Standby mode...",
                "🔇 All quiet...",
                "💤 Idle state..."
            ],
            'sad': [
                "😢 Build issues detected...",
                "😔 Errors encountered...",
                "💔 Something went wrong...",
                "😞 Debugging needed..."
            ],
            'angry': [
                "😡 Frustration detected...",
                "💢 Multiple errors!",
                "🔴 Build failures!",
                "😤 Critical issues..."
            ],
            'surprised': [
                "😲 Unexpected activity!",
                "🤯 Sudden spike!",
                "😮 What just happened?",
                "⚡ Rapid changes!"
            ],
            'love': [
                "❤️ Clean build success!",
                "💝 Everything working!",
                "😍 Perfect code!",
                "💖 All tests passing!"
            ],
            'music': [
                "🎵 In the zone!",
                "🎶 Flow state active!",
                "🎧 Deep focus mode!",
                "🎼 Coding rhythm!"
            ],
            'dead': [
                "💀 System exhausted...",
                "😵 No activity...",
                "⚰️ Completely idle...",
                "👻 Ghosting..."
            ]
        }
        
        try:
            self.mqtt_client.publish('sangi/emotion/set', payload, 1)
            
            # Pick a random message variant for this emotion
            messages = emotion_messages.get(emotion, [f"📤 {emotion.upper()}"])
            message = random.choice(messages) if messages else f"📤 {emotion.upper()}"
            
            self.logger.info(f"{message} | Activity: {stats['total_activity_per_min']:.1f}/min")
            
            # Track message
            self.message_history.append({
                'time': datetime.now().isoformat(),
                'emotion': emotion,
                'stats': stats
            })
            # Keep last 100
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
        """Main monitoring loop with enhanced activity tracking"""
        self.logger.info("🚀 SANGI Workspace Monitor started")
        self.logger.info(f"📡 Device: {self.config['client_id']}")
        
        self.start_file_watcher()
        
        loop_counter = 0
        
        try:
            while True:
                loop_counter += 1
                
                # Check various activity sources
                if loop_counter % 6 == 0:  # Every 30 seconds
                    self.check_clipboard_activity()
                    self.check_git_activity()
                
                # Check audio playback every 10 seconds
                if loop_counter % 2 == 0:  # Every 10 seconds
                    self.check_audio_playback()
                
                # Check terminal/build processes
                self.monitor_terminal_activity()
                
                # Check VS Code activity
                self.check_vs_code_activity()
                
                # Determine and publish emotion
                emotion = self.determine_emotion()
                if emotion != self.current_emotion:
                    # Dynamic transition messages
                    transition_messages = {
                        ('thinking', 'happy'): "💡→😊 Eureka moment!",
                        ('thinking', 'excited'): "💭→⚡ Idea breakthrough!",
                        ('happy', 'thinking'): "😊→🤔 New challenge ahead...",
                        ('excited', 'thinking'): "⚡→💭 Focusing down...",
                        ('sleepy', 'thinking'): "😴→💭 Waking up...",
                        ('sleepy', 'excited'): "😴→⚡ Energy spike!",
                        ('confused', 'thinking'): "❓→💭 Analyzing the problem...",
                        ('confused', 'happy'): "❓→😊 Issue resolved!",
                        ('sad', 'thinking'): "😢→💭 Debugging mode...",
                        ('sad', 'happy'): "😢→😊 Build fixed!",
                        ('idle', 'thinking'): "😐→💭 Starting work...",
                        ('idle', 'excited'): "😐→⚡ Burst of activity!",
                        ('thinking', 'sleepy'): "💭→😴 Slowing down...",
                        ('excited', 'sleepy'): "⚡→� Energy depleted...",
                    }
                    
                    # Check if we have a specific transition message
                    transition_key = (self.current_emotion, emotion)
                    if transition_key in transition_messages:
                        transition_msg = transition_messages[transition_key]
                    else:
                        # Generic transition
                        transition_msg = f"🔄 {self.current_emotion} → {emotion}"
                    
                    self.logger.info(transition_msg)
                    self.publish_emotion(emotion)
                    self.current_emotion = emotion
                    self.last_publish_time = time.time()
                
                # Publish detailed stats every 30 seconds
                if loop_counter % 6 == 0:
                    self.publish_activity_stats()
                
                time.sleep(5)  # Check every 5 seconds
                
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
