"""
Emotion Mapper Module

Maps workspace activity statistics to SANGI robot emotions.
Uses priority-based logic to determine the most appropriate emotion.
"""

import time
import random


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


class EmotionMapper:
    """Maps activity statistics to appropriate emotions"""
    
    def __init__(self, logger=None):
        """
        Initialize emotion mapper
        
        Args:
            logger: Optional logger for debug output
        """
        self.logger = logger
        self.current_emotion = 'idle'
        self.last_random_change = time.time()
        
    def _log_debug(self, message):
        """Log debug message if logger available"""
        if self.logger:
            self.logger.debug(message)
    
    def determine_emotion(self, stats):
        """
        Determine appropriate emotion based on activity statistics
        
        Args:
            stats: Dictionary of activity statistics from ActivityTracker
            
        Returns:
            str: Emotion name (key from EMOTIONS dict)
        """
        # Time-based random variations (prevent stagnation)
        current_time = time.time()
        if current_time - self.last_random_change > 120:  # Every 2 minutes
            if random.random() < 0.3:  # 30% chance of random emotion
                self.last_random_change = current_time
                random_emotions = ['thinking', 'happy', 'excited', 'music']
                emotion = random.choice([e for e in random_emotions if e != self.current_emotion])
                self._log_debug(f"🎲 Random emotion variation: {emotion}")
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
    
    def get_emotion_id(self, emotion_name):
        """
        Get numeric emotion ID for MQTT message
        
        Args:
            emotion_name: Emotion name (key from EMOTIONS dict)
            
        Returns:
            int: Emotion ID (0-12)
        """
        return EMOTIONS.get(emotion_name, 0)
    
    def get_emotion_message(self, emotion):
        """
        Get display message for emotion
        
        Args:
            emotion: Emotion name
            
        Returns:
            str: Random message variant for this emotion
        """
        emotion_messages = {
            'thinking': ["🤔 Deep in thought...", "💭 Pondering solutions...", "🧠 Brain processing...", "💡 Ideas brewing..."],
            'happy': ["😊 Feeling productive!", "✨ Code flowing nicely!", "🎉 Great progress!", "😄 Things are clicking!"],
            'excited': ["⚡ High energy mode!", "🚀 Maximum productivity!", "🔥 On fire!", "💫 Super active!"],
            'sleepy': ["😴 Time for a break?", "💤 Winding down...", "🌙 Getting quiet...", "😪 Low activity detected..."],
            'confused': ["❓ Detecting struggles...", "🤷 Many errors found...", "😵 Debug mode active...", "🔍 Investigating issues..."],
            'idle': ["😐 Waiting for action...", "⏸️  Standby mode...", "🔇 All quiet...", "💤 Idle state..."],
            'sad': ["😢 Build issues detected...", "😔 Errors encountered...", "💔 Something went wrong...", "😞 Debugging needed..."],
            'angry': ["😡 Frustration detected...", "💢 Multiple errors!", "🔴 Build failures!", "😤 Critical issues..."],
            'surprised': ["😲 Unexpected activity!", "🤯 Sudden spike!", "😮 What just happened?", "⚡ Rapid changes!"],
            'love': ["❤️ Clean build success!", "💝 Everything working!", "😍 Perfect code!", "💖 All tests passing!"],
            'music': ["🎵 In the zone!", "🎶 Flow state active!", "🎧 Deep focus mode!", "🎼 Coding rhythm!"],
            'dead': ["💀 System exhausted...", "😵 No activity...", "⚰️ Completely idle...", "👻 Ghosting..."]
        }
        
        messages = emotion_messages.get(emotion, [f"📤 {emotion.upper()}"])
        return random.choice(messages) if messages else f"📤 {emotion.upper()}"
