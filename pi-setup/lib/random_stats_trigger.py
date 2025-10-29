"""
Random stats/notification trigger for SANGI
Randomly sends GitHub stats or notifications to display on SANGI
"""

import logging
import time
import random


class RandomStatsTrigger:
    """Randomly trigger stats display on SANGI"""
    
    def __init__(self, config, stats_callback, emotion_callback):
        """
        Initialize random stats trigger
        
        Args:
            config: Configuration dictionary
            stats_callback: Function to call to trigger stats display
            emotion_callback: Function to call to send emotion change
        """
        self.config = config
        self.stats_callback = stats_callback
        self.emotion_callback = emotion_callback
        self.logger = logging.getLogger('RandomStatsTrigger')
        
        trigger_config = config.get('random_stats_trigger', {})
        self.enabled = trigger_config.get('enabled', True)
        self.interval = trigger_config.get('interval', 300)  # 5 minutes default
        self.trigger_types = trigger_config.get('trigger_types', ['github_stats'])
        
        self.last_trigger_time = None
        
        if not self.enabled:
            self.logger.warning("Random stats trigger is disabled in config")
        else:
            self.logger.info(f"RandomStatsTrigger initialized (interval: {self.interval}s)")
    
    def should_trigger(self):
        """
        Check if it's time to trigger a random display
        
        Returns:
            bool: True if should trigger now
        """
        if not self.enabled:
            return False
        
        current_time = time.time()
        
        if self.last_trigger_time is None:
            # First trigger - add random delay 0-60s to avoid immediate trigger on start
            self.last_trigger_time = current_time + random.randint(0, 60)
            return False
        
        if (current_time - self.last_trigger_time) >= self.interval:
            return True
        
        return False
    
    def trigger_random_display(self):
        """
        Trigger a random stats/notification display
        Chooses between available types (github_stats, notifications, etc.)
        """
        if not self.enabled:
            return
        
        # Update last trigger time
        self.last_trigger_time = time.time()
        
        # Filter available trigger types
        available_types = []
        
        if 'github_stats' in self.trigger_types:
            # Check if GitHub stats are enabled
            if self.config.get('github_stats', {}).get('enabled', False):
                available_types.append('github_stats')
        
        if 'notifications' in self.trigger_types:
            # Could check for queued notifications here
            available_types.append('notifications')
        
        if not available_types:
            self.logger.warning("No available trigger types enabled")
            return
        
        # Randomly select a type
        selected_type = random.choice(available_types)
        
        self.logger.info(f"Triggering random display: {selected_type}")
        
        if selected_type == 'github_stats':
            # Trigger GitHub stats display
            self._trigger_github_stats()
        elif selected_type == 'notifications':
            # Could trigger notification review
            pass
    
    def _trigger_github_stats(self):
        """Trigger GitHub stats emotion"""
        try:
            # Send EMOTION_GITHUB_STATS (15) to SANGI
            self.emotion_callback(15)
            self.logger.info("Sent EMOTION_GITHUB_STATS to SANGI")
            
        except Exception as e:
            self.logger.error(f"Failed to trigger GitHub stats: {e}", exc_info=True)
    
    def start_monitoring(self):
        """
        Start monitoring for trigger intervals (blocking loop)
        Should be run in a separate thread
        """
        if not self.enabled:
            self.logger.warning("Random stats trigger disabled, not starting monitor")
            return
        
        self.logger.info(f"Starting random stats trigger (interval: {self.interval}s)")
        
        try:
            while True:
                if self.should_trigger():
                    self.trigger_random_display()
                
                # Check every 10 seconds
                time.sleep(10)
                
        except KeyboardInterrupt:
            self.logger.info("Random stats trigger stopped by user")
        except Exception as e:
            self.logger.error(f"Error in random stats trigger: {e}", exc_info=True)
            raise
