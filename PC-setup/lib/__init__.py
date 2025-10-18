"""
PC-setup Library Modules

Reusable components for SANGI workspace monitoring.
"""

__version__ = "2.0.0"
__author__ = "SANGI Project"

from .activity_tracker import ActivityTracker
from .audio_detector import AudioDetector
from .emotion_mapper import EmotionMapper, EMOTIONS
from .notification_detector import NotificationDetector

__all__ = [
    'ActivityTracker',
    'AudioDetector',
    'EmotionMapper',
    'NotificationDetector',
    'EMOTIONS'
]
