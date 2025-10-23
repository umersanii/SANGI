"""
PC-setup Library Modules

Reusable components for SANGI workspace monitoring.
"""

__version__ = "2.0.0"
__author__ = "SANGI Project"

from .activity_tracker import ActivityTracker
from .audio_detector import AudioDetector
from .emotion_mapper import EmotionMapper, EMOTIONS
# NotificationDetector has been moved to a standalone Pi notification service.
# Keep the module for legacy/testing, but do not re-export it here to avoid
# importing desktop-only notification code in the workspace monitor.

__all__ = [
    'ActivityTracker',
    'AudioDetector',
    'EmotionMapper',
    'EMOTIONS'
]
__all__ = [
    'ActivityTracker',
    'AudioDetector',
    'EmotionMapper',
    'EMOTIONS'
]
