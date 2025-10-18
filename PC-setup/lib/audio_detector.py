"""
Audio Detector Module

Detects audio playback through system speakers/headphones using multiple methods:
1. ALSA (/proc/asound) - Hardware pointer advancement
2. PulseAudio (pactl) - If available
3. Process detection - Media player CPU usage
"""

import time
import psutil
import subprocess
from pathlib import Path


class AudioDetector:
    """Detects active audio playback on the system"""
    
    def __init__(self, logger=None):
        """
        Initialize audio detector
        
        Args:
            logger: Optional logger for debug output
        """
        self.logger = logger
        
    def _log_debug(self, message):
        """Log debug message if logger available"""
        if self.logger:
            self.logger.debug(message)
    
    def check_alsa(self):
        """
        Check ALSA for active audio streams
        
        Returns:
            bool: True if audio is playing via ALSA
        """
        try:
            pcm_files = Path('/proc/asound').glob('card*/pcm*/sub*/status')
            
            for status_file in pcm_files:
                try:
                    content = status_file.read_text()
                    if 'state: RUNNING' not in content:
                        continue
                    
                    # Parse hw_ptr and owner_pid
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
                    
                    # Verify audio is flowing (hw_ptr advancing)
                    if hw_ptr1 is not None:
                        time.sleep(0.1)  # Wait 100ms
                        content2 = status_file.read_text()
                        
                        for line in content2.split('\n'):
                            if 'hw_ptr' in line and ':' in line:
                                hw_ptr2 = int(line.split(':')[1].strip())
                                
                                # If hw_ptr advanced significantly, audio is playing
                                # At 48kHz, 100ms = 4800 samples minimum
                                if hw_ptr2 > hw_ptr1 and (hw_ptr2 - hw_ptr1) > 1000:
                                    # Verify the owner process isn't just audio server idle
                                    if owner_pid:
                                        try:
                                            proc = psutil.Process(owner_pid)
                                            proc_name = proc.name().lower()
                                            # If it's just audio server with no clients, skip
                                            if proc_name in ['pipewire', 'pulseaudio', 'jackd']:
                                                children = proc.children()
                                                if not children:
                                                    continue
                                        except (psutil.NoSuchProcess, psutil.AccessDenied):
                                            pass
                                    
                                    self._log_debug("🎵 Audio detected via ALSA (hw_ptr advancing)")
                                    return True
                                break
                
                except Exception:
                    pass
            
            return False
            
        except (FileNotFoundError, PermissionError, ValueError):
            return False
    
    def check_pulseaudio(self):
        """
        Check PulseAudio for active audio streams
        
        Returns:
            bool: True if audio is playing via PulseAudio
        """
        try:
            result = subprocess.run(
                ['pactl', 'list', 'sink-inputs'],
                capture_output=True,
                text=True,
                timeout=1
            )
            
            if 'Sink Input #' in result.stdout:
                self._log_debug("🎵 Audio detected via PulseAudio")
                return True
            
            return False
            
        except (FileNotFoundError, subprocess.TimeoutExpired):
            return False
    
    def check_media_processes(self):
        """
        Check for media player processes with audio activity
        
        Returns:
            bool: True if active media player found
        """
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
                            self._log_debug(f"🎵 Audio detected via {player} ({cpu:.1f}% CPU)")
                            return True
                            
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass
        
        return False
    
    def is_audio_playing(self):
        """
        Check if audio is currently playing (tries all methods)
        
        Returns:
            bool: True if audio playback detected
        """
        # Method 1: ALSA (most reliable)
        if self.check_alsa():
            return True
        
        # Method 2: PulseAudio (if available)
        if self.check_pulseaudio():
            return True
        
        # Method 3: Process detection (fallback)
        if self.check_media_processes():
            return True
        
        return False
