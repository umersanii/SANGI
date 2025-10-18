"""
Unit tests for ActivityTracker module
Run with: pytest tests/test_activity_tracker.py -v
"""

import pytest
from datetime import datetime, timedelta
import time
import sys
sys.path.insert(0, '..')

from lib.activity_tracker import ActivityTracker


class TestActivityTracker:
    """Test suite for activity tracking functionality"""
    
    def test_initialization(self):
        """Test tracker initializes with correct defaults"""
        tracker = ActivityTracker(window_size_minutes=5)
        
        assert len(tracker.file_changes) == 0
        assert len(tracker.keyboard_events) == 0
        assert tracker.is_audio_playing == False
        assert tracker.last_clipboard == ""
    
    def test_add_file_change(self):
        """Test file change tracking"""
        tracker = ActivityTracker()
        
        tracker.add_file_change("/path/to/file.py")
        assert len(tracker.file_changes) == 1
        assert tracker.last_activity is not None
    
    def test_add_keyboard_event(self):
        """Test keyboard event tracking"""
        tracker = ActivityTracker()
        
        tracker.add_keyboard_event()
        assert len(tracker.keyboard_events) == 1
    
    def test_add_mouse_event(self):
        """Test mouse event tracking"""
        tracker = ActivityTracker()
        
        tracker.add_mouse_event()
        assert len(tracker.mouse_events) == 1
    
    def test_old_events_cleanup(self):
        """Test that old events are removed after window expires"""
        tracker = ActivityTracker(window_size_minutes=0.1)  # 6 second window
        
        # Add event
        tracker.add_file_change("test.py")
        assert len(tracker.file_changes) == 1
        
        # Wait for window to expire
        time.sleep(7)
        
        # Add new event (triggers cleanup)
        tracker.add_file_change("test2.py")
        
        # Old event should be cleaned up
        assert len(tracker.file_changes) == 1
    
    def test_stats_calculation(self):
        """Test activity statistics calculation"""
        tracker = ActivityTracker(window_size_minutes=5)
        
        # Add various events
        for i in range(10):
            tracker.add_keyboard_event()
        
        for i in range(5):
            tracker.add_file_change(f"file{i}.py")
        
        stats = tracker.get_stats()
        
        assert stats['keyboard_events_per_min'] == 10 / 5  # 2 per minute
        assert stats['file_changes_per_min'] == 5 / 5  # 1 per minute
        assert stats['total_activity_per_min'] > 0
        assert stats['idle_minutes'] < 0.1  # Should be very recent
    
    def test_git_activity_tracking(self):
        """Test git event tracking"""
        tracker = ActivityTracker()
        
        tracker.add_git_event("commit")
        tracker.add_git_event("push")
        
        stats = tracker.get_stats()
        assert stats['git_activity'] == 2
    
    def test_audio_playback_tracking(self):
        """Test audio playback detection"""
        tracker = ActivityTracker()
        
        tracker.add_audio_event(True)
        assert tracker.is_audio_playing == True
        
        stats = tracker.get_stats()
        assert stats['is_audio_playing'] == True
        assert stats['audio_activity'] >= 1
    
    def test_clipboard_tracking(self):
        """Test clipboard event tracking"""
        tracker = ActivityTracker()
        
        tracker.last_clipboard = "old content"
        tracker.add_clipboard_event()
        
        stats = tracker.get_stats()
        assert stats['clipboard_events_per_min'] > 0
    
    def test_build_event_success(self):
        """Test build success tracking"""
        tracker = ActivityTracker()
        
        tracker.add_build_event(success=True)
        tracker.add_build_event(success=True)
        
        stats = tracker.get_stats()
        assert stats['recent_build_success'] == True
        assert stats['recent_build_failure'] == False
    
    def test_build_event_failure(self):
        """Test build failure tracking"""
        tracker = ActivityTracker()
        
        tracker.add_build_event(success=False)
        
        stats = tracker.get_stats()
        assert stats['recent_build_failure'] == True
    
    def test_terminal_command_tracking(self):
        """Test terminal command tracking"""
        tracker = ActivityTracker()
        
        tracker.add_terminal_command()
        tracker.add_terminal_command()
        
        stats = tracker.get_stats()
        assert stats['terminal_commands_per_min'] > 0
    
    def test_error_event_tracking(self):
        """Test error event tracking"""
        tracker = ActivityTracker()
        
        tracker.add_error_event()
        tracker.add_error_event()
        tracker.add_error_event()
        
        stats = tracker.get_stats()
        assert stats['total_errors'] == 3
    
    def test_mixed_activity(self):
        """Test combination of different activities"""
        tracker = ActivityTracker(window_size_minutes=1)
        
        # Simulate busy coding session
        for i in range(20):
            tracker.add_keyboard_event()
        
        for i in range(10):
            tracker.add_mouse_event()
        
        for i in range(3):
            tracker.add_file_change(f"module{i}.cpp")
        
        tracker.add_git_event("commit")
        tracker.add_build_event(success=True)
        
        stats = tracker.get_stats()
        
        # Should show high activity
        assert stats['total_activity_per_min'] > 30  # 33 events / 1 min
        assert stats['git_activity'] == 1
        assert stats['recent_build_success'] == True
    
    def test_idle_detection(self):
        """Test idle time calculation"""
        tracker = ActivityTracker()
        
        tracker.add_keyboard_event()
        
        # Wait a bit
        time.sleep(2)
        
        stats = tracker.get_stats()
        
        # Should show ~2 seconds idle (converted to minutes)
        assert 0.03 < stats['idle_minutes'] < 0.04
    
    def test_zero_division_protection(self):
        """Test that per-minute calculations don't divide by zero"""
        tracker = ActivityTracker(window_size_minutes=0)  # Edge case
        
        # Should not crash
        stats = tracker.get_stats()
        
        # All rates should be infinity or zero (handled gracefully)
        assert 'keyboard_events_per_min' in stats


class TestActivityTrackerEdgeCases:
    """Edge case testing"""
    
    def test_negative_window_size(self):
        """Test that negative window size is handled"""
        # Should not crash (datetime.timedelta will handle it)
        tracker = ActivityTracker(window_size_minutes=-1)
        
        # Cleanup should work without errors
        tracker.add_file_change("test.py")
        stats = tracker.get_stats()
    
    def test_very_large_window(self):
        """Test with very large time window"""
        tracker = ActivityTracker(window_size_minutes=10000)
        
        for i in range(100):
            tracker.add_keyboard_event()
        
        stats = tracker.get_stats()
        
        # Events per minute should be very low
        assert stats['keyboard_events_per_min'] < 1
    
    def test_rapid_events(self):
        """Test handling of many rapid events"""
        tracker = ActivityTracker()
        
        # Add 1000 events rapidly
        for i in range(1000):
            tracker.add_keyboard_event()
        
        stats = tracker.get_stats()
        assert len(tracker.keyboard_events) == 1000
    
    def test_empty_stats(self):
        """Test stats when no activity recorded"""
        tracker = ActivityTracker()
        
        stats = tracker.get_stats()
        
        assert stats['file_changes_per_min'] == 0
        assert stats['total_errors'] == 0
        assert stats['recent_build_success'] is None
        assert stats['git_activity'] == 0


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
