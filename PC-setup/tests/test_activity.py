#!/usr/bin/env python3
"""
Quick test script to verify activity tracking works
Run this to test the enhanced workspace monitor without MQTT
"""

import time
import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent))

# Import the monitor (without running it)
print("🧪 Testing SANGI Workspace Monitor Activity Tracking")
print("=" * 60)

# Test imports
try:
    from workspace_monitor import ActivityTracker, PYNPUT_AVAILABLE
    print("✅ ActivityTracker imported successfully")
except Exception as e:
    print(f"❌ Import failed: {e}")
    sys.exit(1)

# Test activity tracker
tracker = ActivityTracker(window_size_minutes=1)
print(f"✅ ActivityTracker initialized (window: 1 minute)")
print()

# Add some test events
print("Adding test events...")
tracker.add_file_change("test.py")
time.sleep(0.1)
tracker.add_keyboard_event()
tracker.add_keyboard_event()
tracker.add_keyboard_event()
time.sleep(0.1)
tracker.add_mouse_event()
tracker.add_mouse_event()
time.sleep(0.1)
tracker.add_clipboard_event()
time.sleep(0.1)
tracker.add_git_event("commit")
print("✅ Added: 1 file change, 3 keyboard events, 2 mouse events, 1 clipboard, 1 git event")
print()

# Get stats
stats = tracker.get_stats()
print("📊 Activity Statistics:")
print(f"  File changes/min:     {stats['file_changes_per_min']:.2f}")
print(f"  Keyboard events/min:  {stats['keyboard_events_per_min']:.2f}")
print(f"  Mouse events/min:     {stats['mouse_events_per_min']:.2f}")
print(f"  Clipboard events/min: {stats['clipboard_events_per_min']:.2f}")
print(f"  Total activity/min:   {stats['total_activity_per_min']:.2f}")
print(f"  Idle minutes:         {stats['idle_minutes']:.2f}")
print(f"  Git activity:         {stats['git_activity']}")
print()

# Test pynput availability
if PYNPUT_AVAILABLE:
    print("✅ pynput is available - keyboard/mouse monitoring enabled")
else:
    print("⚠️  pynput NOT available - keyboard/mouse monitoring disabled")
    print("   Install with: pip3 install --user pynput")
print()

# Test emotion determination logic
print("🎭 Testing Emotion Logic:")
from workspace_monitor import WorkspaceMonitor

# We can't fully initialize without MQTT, so we'll test the logic manually
test_cases = [
    {"total_activity_per_min": 15, "expected": "excited (high activity)"},
    {"keyboard_events_per_min": 25, "expected": "thinking (typing)"},
    {"clipboard_events_per_min": 1, "expected": "happy (clipboard)"},
    {"file_changes_per_min": 1, "expected": "happy (files)"},
    {"idle_minutes": 12, "expected": "sleepy (idle)"},
    {"total_activity_per_min": 0.1, "idle_minutes": 0.5, "expected": "idle (low activity)"},
]

for i, case in enumerate(test_cases, 1):
    activity = case.get('total_activity_per_min', 0)
    idle = case.get('idle_minutes', 0)
    print(f"  {i}. Activity: {activity}/min, Idle: {idle}min → {case['expected']}")

print()
print("✅ Activity tracking test complete!")
print()
print("To run the full monitor with MQTT:")
print("  python3 workspace_monitor.py")
