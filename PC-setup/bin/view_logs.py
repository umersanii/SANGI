#!/usr/bin/env python3
"""
SANGI Log Viewer
View and analyze workspace monitor logs and MQTT communication history
"""

import json
import sys
from pathlib import Path
from datetime import datetime
from collections import Counter

def view_mqtt_log(log_file='logs/mqtt_messages.log', lines=50):
    """Display recent MQTT messages"""
    if not Path(log_file).exists():
        print(f"❌ Log file not found: {log_file}")
        return
    
    print(f"\n{'='*70}")
    print(f"📡 MQTT Communication Log (Last {lines} messages)")
    print(f"{'='*70}\n")
    
    with open(log_file, 'r') as f:
        all_lines = f.readlines()
        recent_lines = all_lines[-lines:]
        
        for line in recent_lines:
            # Parse timestamp and message
            parts = line.strip().split(' | ')
            if len(parts) >= 2:
                timestamp = parts[0]
                message = ' | '.join(parts[1:])
                
                # Color code by type
                if 'PUBLISH' in message:
                    print(f"📤 [{timestamp}] {message}")
                elif 'CONNECTED' in message:
                    print(f"✅ [{timestamp}] {message}")
                elif 'STATUS' in message:
                    print(f"📊 [{timestamp}] {message}")
                else:
                    print(f"   [{timestamp}] {message}")
    
    print(f"\n{'='*70}\n")

def view_main_log(log_file='logs/workspace_monitor.log', lines=50):
    """Display recent main log entries"""
    if not Path(log_file).exists():
        print(f"❌ Log file not found: {log_file}")
        return
    
    print(f"\n{'='*70}")
    print(f"📄 Main Monitor Log (Last {lines} entries)")
    print(f"{'='*70}\n")
    
    with open(log_file, 'r') as f:
        all_lines = f.readlines()
        recent_lines = all_lines[-lines:]
        
        for line in recent_lines:
            parts = line.strip().split(' | ')
            if len(parts) >= 3:
                timestamp = parts[0]
                level = parts[1].strip()
                message = ' | '.join(parts[2:])
                
                # Color indicators
                if 'ERROR' in level:
                    print(f"❌ [{timestamp}] {message}")
                elif 'WARNING' in level:
                    print(f"⚠️  [{timestamp}] {message}")
                elif 'INFO' in level:
                    print(f"ℹ️  [{timestamp}] {message}")
                else:
                    print(f"   [{timestamp}] {message}")
    
    print(f"\n{'='*70}\n")

def view_history(history_file='logs/message_history.json'):
    """Display message history with statistics"""
    if not Path(history_file).exists():
        print(f"❌ History file not found: {history_file}")
        return
    
    with open(history_file, 'r') as f:
        history = json.load(f)
    
    if not history:
        print("📭 No message history found")
        return
    
    print(f"\n{'='*70}")
    print(f"📊 Communication Statistics ({len(history)} total messages)")
    print(f"{'='*70}\n")
    
    # Emotion distribution
    emotions = [msg['payload']['emotion_name'] for msg in history if 'emotion_name' in msg['payload']]
    emotion_counts = Counter(emotions)
    
    print("Emotion Distribution:")
    max_count = max(emotion_counts.values()) if emotion_counts else 1
    for emotion, count in sorted(emotion_counts.items(), key=lambda x: x[1], reverse=True):
        bar = '█' * int(count * 40 / max_count)
        percentage = count * 100 / len(emotions)
        print(f"  {emotion:12} {bar:40} {count:3} ({percentage:5.1f}%)")
    
    # Time range
    if history:
        first_msg = datetime.fromisoformat(history[0]['timestamp'])
        last_msg = datetime.fromisoformat(history[-1]['timestamp'])
        duration = (last_msg - first_msg).total_seconds() / 3600
        
        print(f"\nTime Range:")
        print(f"  First message: {first_msg.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"  Last message:  {last_msg.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"  Duration:      {duration:.1f} hours")
        print(f"  Avg rate:      {len(history)/max(duration, 0.1):.1f} msg/hour")
    
    print(f"\n{'='*70}")
    print(f"📜 Recent Messages (Last 20)")
    print(f"{'='*70}\n")
    
    for msg in history[-20:]:
        timestamp = datetime.fromisoformat(msg['timestamp'])
        payload = msg['payload']
        emotion = payload.get('emotion_name', 'unknown')
        stats = payload.get('stats', {})
        
        print(f"[{timestamp.strftime('%Y-%m-%d %H:%M:%S')}]")
        print(f"  Emotion: {emotion.upper():10}", end='')
        
        if stats:
            changes = stats.get('changes_per_min', 0)
            print(f" | Activity: {changes:.1f} changes/min", end='')
            
            if stats.get('idle_minutes', 0) > 5:
                print(f" | Idle: {stats['idle_minutes']:.0f}m", end='')
        
        print()
    
    print(f"\n{'='*70}\n")

def tail_log(log_file='logs/mqtt_messages.log'):
    """Follow log file in real-time"""
    import subprocess
    
    if not Path(log_file).exists():
        print(f"❌ Log file not found: {log_file}")
        return
    
    print(f"📡 Following {log_file} (Ctrl+C to stop)...\n")
    
    try:
        subprocess.run(['tail', '-f', log_file])
    except KeyboardInterrupt:
        print("\n⏹️  Stopped following log")

def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='View SANGI workspace monitor logs')
    parser.add_argument('--mqtt', action='store_true', help='Show MQTT message log')
    parser.add_argument('--main', action='store_true', help='Show main monitor log')
    parser.add_argument('--history', action='store_true', help='Show message history with stats')
    parser.add_argument('--tail', action='store_true', help='Follow MQTT log in real-time')
    parser.add_argument('--lines', type=int, default=50, help='Number of lines to show (default: 50)')
    parser.add_argument('--all', action='store_true', help='Show all logs')
    
    args = parser.parse_args()
    
    # Change to script directory
    script_dir = Path(__file__).parent
    import os
    os.chdir(script_dir)
    
    # Default: show all if no specific option selected
    if not any([args.mqtt, args.main, args.history, args.tail]):
        args.all = True
    
    if args.tail:
        tail_log()
    elif args.all:
        view_history()
        view_mqtt_log(lines=args.lines)
        view_main_log(lines=args.lines)
    else:
        if args.history:
            view_history()
        if args.mqtt:
            view_mqtt_log(lines=args.lines)
        if args.main:
            view_main_log(lines=args.lines)

if __name__ == '__main__':
    main()
