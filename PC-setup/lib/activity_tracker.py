"""
Activity Tracker Module

Tracks various types of workspace activity:
- File changes (code edits)
- Keyboard and mouse events
- Terminal/build commands
- Git activity
- Clipboard usage
- Audio playback
"""

from datetime import datetime, timedelta
from collections import deque


class ActivityTracker:
    """Enhanced activity tracker with multiple input sources"""
    
    def __init__(self, window_size_minutes=5):
        """
        Initialize activity tracker
        
        Args:
            window_size_minutes: Time window for activity aggregation
        """
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
