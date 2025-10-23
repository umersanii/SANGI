"""
D-Bus based notification monitor for Discord and WhatsApp desktop notifications
"""

import logging
from gi.repository import GLib
from dbus.mainloop.glib import DBusGMainLoop
import dbus
import json


class NotificationMonitor:
    """Monitor desktop notifications via D-Bus"""
    
    def __init__(self, config, callback):
        """
        Initialize notification monitor
        
        Args:
            config: Configuration dictionary
            callback: Function to call when notification received (type, title, message)
        """
        self.config = config
        self.callback = callback
        self.logger = logging.getLogger('NotificationMonitor')
        
        # Initialize D-Bus
        DBusGMainLoop(set_as_default=True)
        self.bus = dbus.SessionBus()
        self.loop = GLib.MainLoop()
        
        # Notification filters
        self.filters = config.get('filters', {})
        self.ignore_apps = self.filters.get('ignore_apps', [])
        self.min_interval = self.filters.get('min_notification_interval', 5)
        self.last_notification_time = 0
        
        # Discord/WhatsApp patterns
        self.discord_keywords = ['discord', 'Discord']
        self.whatsapp_keywords = ['whatsapp', 'WhatsApp', 'Signal']
        
        self.logger.info("NotificationMonitor initialized")
    
    def _is_discord_notification(self, app_name, summary):
        """Check if notification is from Discord"""
        return any(keyword in app_name or keyword in summary 
                   for keyword in self.discord_keywords)
    
    def _is_whatsapp_notification(self, app_name, summary):
        """Check if notification is from WhatsApp"""
        return any(keyword in app_name or keyword in summary 
                   for keyword in self.whatsapp_keywords)
    
    def _parse_discord_notification(self, summary, body):
        """
        Parse Discord notification
        Returns (username, message) tuple
        """
        # Discord format: "username - #channel" or just "username"
        username = summary.split(' - ')[0].strip() if ' - ' in summary else summary.strip()
        
        # Simplify message to "new message"
        message = "new message"
        
        return username, message
    
    def _parse_whatsapp_notification(self, summary, body):
        """
        Parse WhatsApp notification
        Returns (contact, message) tuple
        """
        # WhatsApp format: "Contact Name" in summary, message in body
        contact = summary.strip()
        
        # Truncate long messages
        message = body[:50] + "..." if len(body) > 50 else body
        
        return contact, message
    
    def _notification_handler(self, app_name, replaces_id, app_icon, 
                             summary, body, actions, hints, expire_timeout):
        """Handle incoming D-Bus notifications"""
        
        # Ignore filtered apps
        if app_name in self.ignore_apps:
            return
        
        # Rate limiting
        import time
        current_time = time.time()
        if current_time - self.last_notification_time < self.min_interval:
            self.logger.debug(f"Notification from {app_name} rate-limited")
            return
        
        # Process Discord notifications
        if (self.config.get('notifications', {}).get('discord', {}).get('enabled', False) and
            self._is_discord_notification(app_name, summary)):
            
            username, message = self._parse_discord_notification(summary, body)
            self.logger.info(f"Discord notification: {username} - {message}")
            self.callback('discord', username, message)
            self.last_notification_time = current_time
            return
        
        # Process WhatsApp notifications
        if (self.config.get('notifications', {}).get('whatsapp', {}).get('enabled', False) and
            self._is_whatsapp_notification(app_name, summary)):
            
            contact, message = self._parse_whatsapp_notification(summary, body)
            self.logger.info(f"WhatsApp notification: {contact} - {message}")
            self.callback('whatsapp', contact, message)
            self.last_notification_time = current_time
            return
    
    def start(self):
        """Start monitoring notifications"""
        try:
            # Subscribe to D-Bus notification signals
            self.bus.add_signal_receiver(
                self._notification_handler,
                dbus_interface="org.freedesktop.Notifications",
                signal_name="Notify"
            )
            
            self.logger.info("Notification monitoring started")
            self.loop.run()
            
        except KeyboardInterrupt:
            self.logger.info("Notification monitoring stopped by user")
            self.stop()
        except Exception as e:
            self.logger.error(f"Error in notification monitoring: {e}", exc_info=True)
            raise
    
    def stop(self):
        """Stop monitoring"""
        self.loop.quit()
        self.logger.info("Notification monitoring stopped")
