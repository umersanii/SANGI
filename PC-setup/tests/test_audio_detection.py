#!/usr/bin/env python3
"""
Test script for audio playback detection
Run this while playing music to verify detection works
"""

import subprocess
import psutil
import time
from pathlib import Path

def check_alsa():
    """Check ALSA for active audio playback"""
    try:
        pcm_files = Path('/proc/asound').glob('card*/pcm*/sub*/status')
        active_streams = []
        
        for status_file in pcm_files:
            try:
                content = status_file.read_text()
                if 'state: RUNNING' in content:
                    # Parse status to get hw_ptr and owner_pid
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
                    
                    # Verify audio is actually flowing (hw_ptr advancing)
                    if hw_ptr1 is not None:
                        time.sleep(0.1)  # Wait 100ms
                        content2 = status_file.read_text()
                        for line in content2.split('\n'):
                            if 'hw_ptr' in line and ':' in line:
                                hw_ptr2 = int(line.split(':')[1].strip())
                                diff = hw_ptr2 - hw_ptr1
                                
                                # If hw_ptr advanced significantly, audio is flowing
                                if hw_ptr2 > hw_ptr1 and diff > 1000:
                                    parts = str(status_file).split('/')
                                    card = parts[3] if len(parts) > 3 else 'unknown'
                                    pcm = parts[4] if len(parts) > 4 else 'unknown'
                                    
                                    # Get owner process name
                                    owner_name = 'unknown'
                                    if owner_pid:
                                        try:
                                            proc = psutil.Process(owner_pid)
                                            owner_name = proc.name()
                                        except:
                                            pass
                                    
                                    active_streams.append((f"{card}/{pcm}", diff, owner_name))
                                    break
            except Exception:
                pass
        
        if active_streams:
            print("✅ ALSA: Audio is playing")
            for stream, diff, owner in active_streams:
                print(f"   Active stream: {stream}")
                print(f"   hw_ptr advanced: {diff} samples")
                print(f"   Owner process: {owner}")
            return True
        else:
            print("⭕ ALSA: No audio playing (device may be idle)")
            return False
            
    except Exception as e:
        print(f"⚠️  ALSA check skipped: {e}")
        return False

def check_pulseaudio():
    """Check PulseAudio for active audio streams"""
    try:
        result = subprocess.run(
            ['pactl', 'list', 'sink-inputs'],
            capture_output=True,
            text=True,
            timeout=1
        )
        
        is_playing = 'Sink Input #' in result.stdout
        
        if is_playing:
            print("✅ PulseAudio: Audio is playing")
            # Count active streams
            stream_count = result.stdout.count('Sink Input #')
            print(f"   Active audio streams: {stream_count}")
            
            # Extract application names
            lines = result.stdout.split('\n')
            for i, line in enumerate(lines):
                if 'application.name' in line:
                    print(f"   App: {line.split('=')[1].strip().strip('\"')}")
        else:
            print("⭕ PulseAudio: No audio playing")
        
        return is_playing
    except Exception as e:
        print(f"⚠️  PulseAudio check skipped: {e}")
        return False

def check_media_processes():
    """Check for media player processes"""
    media_players = {
        'spotify': 1.0,
        'vlc': 0.5,
        'mpv': 0.5,
        'mplayer': 0.5,
        'rhythmbox': 1.0,
        'clementine': 1.0,
        'audacious': 0.8,
        'firefox': 2.0,
        'chrome': 2.0,
        'chromium': 2.0,
    }
    
    found_players = []
    
    for proc in psutil.process_iter(['name', 'cpu_percent', 'cmdline']):
        try:
            proc_name = proc.info['name'].lower() if proc.info['name'] else ''
            cmdline = ' '.join(proc.info['cmdline'] or []).lower()
            
            for player, cpu_threshold in media_players.items():
                if player in proc_name or player in cmdline:
                    cpu = proc.cpu_percent(interval=0.1)
                    if cpu > cpu_threshold:
                        found_players.append((proc_name or cmdline[:30], cpu, cpu_threshold))
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    
    if found_players:
        print("\n✅ Media processes detected:")
        for name, cpu, threshold in found_players:
            print(f"   {name}: {cpu:.1f}% CPU (threshold: {threshold}%)")
        return True
    else:
        print("\n⭕ No active media processes found")
        return False

def main():
    print("=" * 60)
    print("SANGI Audio Detection Test")
    print("=" * 60)
    print("\n🎵 Play some music and watch the detection...\n")
    print("Detection methods:")
    print("  1. ALSA (/proc/asound) - Most reliable")
    print("  2. PulseAudio (pactl) - If available")
    print("  3. Media processes - Fallback detection\n")
    
    try:
        while True:
            print(f"\n[{time.strftime('%H:%M:%S')}] Checking audio...")
            
            alsa_detected = check_alsa()
            pulse_detected = check_pulseaudio()
            process_detected = check_media_processes()
            
            if alsa_detected or pulse_detected or process_detected:
                print("\n🎶 MUSIC EMOTION WOULD BE TRIGGERED! 🎶")
            else:
                print("\n😐 No audio detected - Normal emotion")
            
            print("\n" + "-" * 60)
            time.sleep(5)  # Check every 5 seconds
            
    except KeyboardInterrupt:
        print("\n\n✅ Test stopped")
        print("👋 Audio detection test complete!")

if __name__ == '__main__':
    main()
