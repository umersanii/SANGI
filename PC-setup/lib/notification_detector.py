"""
SANGI Notification Detector
Monitors system notifications from various sources (Discord, Slack, email, etc.)
Uses D-Bus for Linux desktop notifications
"""

import logging
import subprocess
import re
from datetime import datetime
from typing import Dict, List, Optional, Tuple

try:
    import dbus
    from dbus.mainloop.glib import DBusGMainLoop
    from gi.repository import GLib
    DBUS_AVAILABLE = True
except ImportError:
    DBUS_AVAILABLE = False
    print("⚠️  D-Bus not available - notification monitoring disabled")
    print("   Install with: pip install dbus-python pygobject")


class NotificationDetector:
    """Detects and categorizes desktop notifications"""
    
    # Notification source patterns (app name matching)
    SOURCE_PATTERNS = {
        'discord': ['discord', 'Discord'],
        'slack': ['slack', 'Slack'],
        'email': ['thunderbird', 'evolution', 'outlook', 'mail', 'gmail'],
        'github': ['github', 'GitHub Desktop'],
        'calendar': ['calendar', 'gnome-calendar', 'thunderbird'],
        'system': ['notify-send', 'System', 'NetworkManager', 'Update']
    }
    
    # Keywords to filter out unimportant notifications
    IGNORE_KEYWORDS = [
        'is now available',  # App updates
        'battery low',       # Battery warnings (handled separately)
        'volume',            # Volume changes
        'brightness',        # Brightness changes
    ]
    
    def __init__(self, logger=None, callback=None):
        self.logger = logger or logging.getLogger('NotificationDetector')
        self.callback = callback  # Function to call when notification detected
        self.notification_history = []
        self.dbus_loop = None
        self.bus = None
        
        if not DBUS_AVAILABLE:
            self.logger.warning("D-Bus not available - notification detection disabled")
            return
        
        self._init_dbus()
    
    def _init_dbus(self):
        """Initialize D-Bus connection for notification monitoring"""
        try:
            DBusGMainLoop(set_as_default=True)
            self.bus = dbus.SessionBus()
            
            # Listen to notifications using the FreeDesktop notification spec
            self.bus.add_match_string_non_blocking(
                "interface='org.freedesktop.Notifications',member='Notify'"
            )
            self.bus.add_signal_receiver(
                self._notification_callback,
                dbus_interface='org.freedesktop.Notifications',
                signal_name='Notify'
            )
            
            self.logger.info("✅ D-Bus notification monitoring initialized")
            
        except Exception as e:
            self.logger.error(f"Failed to initialize D-Bus: {e}")
            DBUS_AVAILABLE = False
    
    def _notification_callback(self, app_name, replaces_id, app_icon, summary, body, 
                              actions, hints, timeout):
        """Called when a desktop notification is received"""
        try:
            # Convert DBus types to Python strings safely
            app_name = str(app_name) if app_name else ""
            summary = str(summary) if summary else ""
            body = str(body) if body else ""
            
            self.logger.debug(f"📩 Notification: {app_name} | {summary} | {body[:50]}")
            
            # Categorize notification
            notif_type = self._categorize_notification(app_name, summary, body)
            
            # Filter out unimportant notifications
            if self._should_ignore(app_name, summary, body):
                self.logger.debug(f"Ignored: {app_name} - {summary}")
                return
            
            # Store in history
            notification = {
                'timestamp': datetime.now().isoformat(),
                'type': notif_type,
                'app': app_name,
                'title': summary,
                'message': body[:60],  # Truncate long messages
            }
            self.notification_history.append(notification)
            
            # Keep only last 50 notifications
            if len(self.notification_history) > 50:
                self.notification_history = self.notification_history[-50:]
            
            # Call callback if registered
            if self.callback:
                self.callback(notif_type, summary, body[:60])
                
            self.logger.info(f"🔔 [{notif_type}] {summary}")
            
        except Exception as e:
            self.logger.error(f"Error processing notification: {e}")
    
    def _categorize_notification(self, app_name: str, summary: str, body: str) -> str:
        """Determine notification type based on app name and content"""
        app_lower = app_name.lower()
        summary_lower = summary.lower()
        body_lower = body.lower()
        
        # Check app name against known patterns
        for notif_type, patterns in self.SOURCE_PATTERNS.items():
            for pattern in patterns:
                if pattern.lower() in app_lower:
                    return notif_type
        
        # Check content for type hints
        if any(keyword in summary_lower or keyword in body_lower 
               for keyword in ['message', 'dm', 'mention', '@']):
            return 'discord'  # Likely a chat notification
        
        if any(keyword in summary_lower or keyword in body_lower 
               for keyword in ['email', 'inbox', 'from:']):
            return 'email'
        
        if any(keyword in summary_lower or keyword in body_lower 
               for keyword in ['meeting', 'reminder', 'event']):
            return 'calendar'
        
        if any(keyword in summary_lower or keyword in body_lower 
               for keyword in ['pull request', 'pr', 'commit', 'issue']):
            return 'github'
        
        # Default to system notification
        return 'system'
    
    def _should_ignore(self, app_name: str, summary: str, body: str) -> bool:
        """Check if notification should be ignored"""
        combined = f"{app_name} {summary} {body}".lower()
        
        for keyword in self.IGNORE_KEYWORDS:
            if keyword.lower() in combined:
                return True
        
        # Ignore empty notifications
        if not summary and not body:
            return True
        
        return False
    
    def get_recent_notifications(self, limit: int = 10) -> List[Dict]:
        """Get recent notification history"""
        return self.notification_history[-limit:]
    
    def get_notification_stats(self) -> Dict:
        """Get statistics about notifications"""
        if not self.notification_history:
            return {}
        
        type_counts = {}
        for notif in self.notification_history:
            notif_type = notif['type']
            type_counts[notif_type] = type_counts.get(notif_type, 0) + 1
        
        return {
            'total': len(self.notification_history),
            'by_type': type_counts,
            'last_notification': self.notification_history[-1] if self.notification_history else None
        }
    
    def check_specific_app(self, app_name: str) -> bool:
        """Check if specific app has sent notifications recently (last 60s)"""
        if not self.notification_history:
            return False
        
        from datetime import datetime, timedelta
        cutoff = datetime.now() - timedelta(seconds=60)
        
        for notif in reversed(self.notification_history):
            notif_time = datetime.fromisoformat(notif['timestamp'])
            if notif_time < cutoff:
                break
            
            if app_name.lower() in notif['app'].lower():
                return True
        
        return False
    
    def start_monitoring(self):
        """Start the D-Bus event loop for continuous monitoring"""
        if not DBUS_AVAILABLE:
            self.logger.warning("Cannot start monitoring - D-Bus not available")
            return
        
        try:
            self.logger.info("🔊 Starting notification monitoring loop...")
            self.dbus_loop = GLib.MainLoop()
            self.dbus_loop.run()
        except KeyboardInterrupt:
            self.logger.info("Stopping notification monitoring")
            if self.dbus_loop:
                self.dbus_loop.quit()


def test_notification_detector():
    """Test function for notification detector"""
    logging.basicConfig(
        level=logging.DEBUG,
        format='%(asctime)s | %(levelname)-8s | %(message)s'
    )
    logger = logging.getLogger('NotificationTest')
    
    def on_notification(notif_type, title, message):
        logger.info(f"✅ Notification received: [{notif_type}] {title}")
    
    detector = NotificationDetector(logger=logger, callback=on_notification)
    
    if DBUS_AVAILABLE:
        logger.info("Send some desktop notifications to test...")
        logger.info("Example: notify-send 'Discord' 'New message from @friend'")
        detector.start_monitoring()
    else:
        logger.error("D-Bus not available - cannot test")


if __name__ == '__main__':
    test_notification_detector()
