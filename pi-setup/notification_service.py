#!/usr/bin/env python3
"""
SANGI Notification Service
Monitors Discord, GitHub, and WhatsApp notifications and forwards them to SANGI robot via MQTT
"""

import json
import logging
import os
import sys
import threading
import signal
from pathlib import Path

# Add lib to path
sys.path.insert(0, str(Path(__file__).parent / 'lib'))

from lib import NotificationMonitor, MQTTPublisher, GitHubMonitor


class NotificationService:
    """Main notification service orchestrator"""
    
    def __init__(self, config_path='config.json'):
        """Initialize notification service"""
        # Load configuration
        self.config = self._load_config(config_path)
        
        # Setup logging
        self._setup_logging()
        self.logger = logging.getLogger('NotificationService')
        
        # Initialize components
        self.mqtt_publisher = MQTTPublisher(self.config)
        self.notification_monitor = NotificationMonitor(self.config, self._handle_notification)
        self.github_monitor = GitHubMonitor(self.config, self._handle_notification)
        
        # Shutdown flag
        self.shutdown_flag = threading.Event()
        
        self.logger.info("SANGI Notification Service initialized")
    
    def _load_config(self, config_path):
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            print(f"ERROR: Config file not found: {config_path}")
            print("Please copy config.json.template to config.json and configure it")
            sys.exit(1)
        except json.JSONDecodeError as e:
            print(f"ERROR: Invalid JSON in config file: {e}")
            sys.exit(1)
    
    def _setup_logging(self):
        """Setup logging configuration"""
        log_config = self.config.get('logging', {})
        log_level = getattr(logging, log_config.get('level', 'INFO'))
        log_file = log_config.get('file', './logs/notification_monitor.log')
        
        # Create logs directory if needed
        log_dir = Path(log_file).parent
        log_dir.mkdir(parents=True, exist_ok=True)
        
        # Configure logging
        logging.basicConfig(
            level=log_level,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file),
                logging.StreamHandler(sys.stdout)
            ]
        )
    
    def _handle_notification(self, notif_type, title, message):
        """
        Handle incoming notification from any monitor
        
        Args:
            notif_type: Type of notification (discord, github, whatsapp, etc.)
            title: Notification title
            message: Notification message
        """
        self.logger.info(f"Received {notif_type} notification: {title} - {message}")
        
        # Publish to MQTT
        success = self.mqtt_publisher.publish_notification(notif_type, title, message)
        
        if success:
            self.logger.debug(f"Successfully forwarded {notif_type} notification to SANGI")
        else:
            self.logger.warning(f"Failed to forward {notif_type} notification to SANGI")
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        self.logger.info(f"Received signal {signum}, shutting down...")
        self.shutdown_flag.set()
    
    def start(self):
        """Start the notification service"""
        # Register signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
        try:
            # Connect to MQTT
            self.logger.info("Connecting to MQTT...")
            if not self.mqtt_publisher.connect():
                self.logger.error("Failed to connect to MQTT, exiting")
                return
            
            # Test connection
            self.mqtt_publisher.test_connection()
            
            # Start GitHub monitor in separate thread
            github_thread = None
            if self.config.get('notifications', {}).get('github', {}).get('enabled', False):
                self.logger.info("Starting GitHub monitor...")
                github_thread = threading.Thread(target=self.github_monitor.start_polling, daemon=True)
                github_thread.start()
            
            # Start D-Bus notification monitor (blocking)
            self.logger.info("Starting D-Bus notification monitor...")
            self.logger.info("Notification service is running. Press Ctrl+C to stop.")
            
            # Run notification monitor in main thread
            monitor_thread = threading.Thread(target=self.notification_monitor.start, daemon=True)
            monitor_thread.start()
            
            # Wait for shutdown signal
            self.shutdown_flag.wait()
            
        except KeyboardInterrupt:
            self.logger.info("Keyboard interrupt received")
        except Exception as e:
            self.logger.error(f"Error in notification service: {e}", exc_info=True)
        finally:
            self.shutdown()
    
    def shutdown(self):
        """Shutdown the service gracefully"""
        self.logger.info("Shutting down notification service...")
        
        # Stop monitors
        self.notification_monitor.stop()
        
        # Disconnect MQTT
        self.mqtt_publisher.disconnect()
        
        self.logger.info("Notification service stopped")


def main():
    """Main entry point"""
    # Check for config file
    config_file = 'config.json'
    
    if not os.path.exists(config_file):
        print("ERROR: config.json not found!")
        print("\nPlease copy config.json.template to config.json and configure:")
        print("1. AWS IoT endpoint and certificates")
        print("2. GitHub token (optional)")
        print("3. Enable/disable notification sources")
        print("\nSee README.md for detailed setup instructions")
        sys.exit(1)
    
    # Start service
    service = NotificationService(config_file)
    service.start()


if __name__ == '__main__':
    main()
