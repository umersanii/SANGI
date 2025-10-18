#!/usr/bin/env python3
"""
Test script to verify modular library functionality
Tests all three modules without requiring MQTT connection
"""

import sys
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent))

from lib import ActivityTracker, AudioDetector, EmotionMapper, EMOTIONS

def test_activity_tracker():
    """Test ActivityTracker module"""
    print("🧪 Testing ActivityTracker...")
    
    tracker = ActivityTracker(window_size_minutes=5)
    
    # Simulate some events
    tracker.add_file_change("/test/file.py")
    tracker.add_keyboard_event()
    tracker.add_mouse_event()
    tracker.add_clipboard_event()
    tracker.add_git_event("commit")
    
    # Get stats
    stats = tracker.get_stats()
    
    assert 'file_changes_per_min' in stats
    assert 'keyboard_events_per_min' in stats
    assert 'total_activity_per_min' in stats
    assert stats['file_changes_per_min'] > 0
    
    print("✅ ActivityTracker: All tests passed")
    return True

def test_audio_detector():
    """Test AudioDetector module"""
    print("\n🧪 Testing AudioDetector...")
    
    detector = AudioDetector()
    
    # Test detection methods (may return False if no audio)
    alsa_result = detector.check_alsa()
    pulse_result = detector.check_pulseaudio()
    process_result = detector.check_media_processes()
    is_playing = detector.is_audio_playing()
    
    # Just verify they return boolean
    assert isinstance(alsa_result, bool)
    assert isinstance(pulse_result, bool)
    assert isinstance(process_result, bool)
    assert isinstance(is_playing, bool)
    
    print(f"  - ALSA check: {'🎵 Detected' if alsa_result else '🔇 Silent'}")
    print(f"  - PulseAudio check: {'🎵 Detected' if pulse_result else '🔇 Silent'}")
    print(f"  - Process check: {'🎵 Detected' if process_result else '🔇 Silent'}")
    print(f"  - Overall: {'🎵 Audio playing' if is_playing else '🔇 No audio'}")
    
    print("✅ AudioDetector: All tests passed")
    return True

def test_emotion_mapper():
    """Test EmotionMapper module"""
    print("\n🧪 Testing EmotionMapper...")
    
    mapper = EmotionMapper()
    
    # Test with idle stats
    idle_stats = {
        'file_changes_per_min': 0,
        'keyboard_events_per_min': 0,
        'total_activity_per_min': 0,
        'idle_minutes': 1,
        'is_audio_playing': False,
        'git_activity': 0,
        'terminal_commands_per_min': 0,
        'recent_build_failure': False,
        'recent_build_success': False,
        'clipboard_events_per_min': 0,
        'total_errors': 0
    }
    
    emotion = mapper.determine_emotion(idle_stats)
    assert emotion in EMOTIONS
    print(f"  - Idle activity → {emotion}")
    
    # Test with high activity
    active_stats = idle_stats.copy()
    active_stats['total_activity_per_min'] = 15
    active_stats['keyboard_events_per_min'] = 25
    active_stats['idle_minutes'] = 0
    
    emotion = mapper.determine_emotion(active_stats)
    print(f"  - High activity → {emotion}")
    
    # Test with audio
    audio_stats = idle_stats.copy()
    audio_stats['is_audio_playing'] = True
    
    emotion = mapper.determine_emotion(audio_stats)
    assert emotion == 'music', f"Expected 'music' for audio playback, got '{emotion}'"
    print(f"  - Audio playing → {emotion}")
    
    # Test emotion messages
    message = mapper.get_emotion_message('happy')
    assert isinstance(message, str)
    assert len(message) > 0
    print(f"  - Happy message: {message}")
    
    # Test emotion ID lookup
    emotion_id = mapper.get_emotion_id('thinking')
    assert emotion_id == EMOTIONS['thinking']
    print(f"  - Thinking ID: {emotion_id}")
    
    print("✅ EmotionMapper: All tests passed")
    return True

def main():
    """Run all tests"""
    print("="*60)
    print("SANGI Library Module Tests")
    print("="*60)
    
    try:
        test_activity_tracker()
        test_audio_detector()
        test_emotion_mapper()
        
        print("\n" + "="*60)
        print("✅ All module tests passed!")
        print("="*60)
        print("\n💡 You can now use these modules in workspace_monitor.py:")
        print("   from lib import ActivityTracker, AudioDetector, EmotionMapper")
        
        return 0
        
    except AssertionError as e:
        print(f"\n❌ Test failed: {e}")
        return 1
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        return 1

if __name__ == '__main__':
    sys.exit(main())
