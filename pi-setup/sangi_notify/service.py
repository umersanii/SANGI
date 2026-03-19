"""Main notification service orchestrator."""

import logging
import signal
import threading

from sangi_notify.util.logging import setup_logging
from sangi_notify.mqtt.client import MQTTClient
from sangi_notify.monitors.github_api import GitHubAPI
from sangi_notify.monitors.github_notifications import GitHubNotificationMonitor
from sangi_notify.monitors.github_stats import GitHubStatsMonitor
from sangi_notify.monitors.random_trigger import RandomTriggerMonitor


class NotificationService:
    """Wires up MQTT client, monitors, and callbacks."""

    def __init__(self, config: dict):
        self.config = config
        setup_logging(config)
        self.logger = logging.getLogger("NotificationService")

        self.mqtt = MQTTClient(config)

        # Shared GitHub API client (if token is available)
        gh_token = (config.get("notifications", {}).get("github", {}).get("token")
                    or config.get("github_stats", {}).get("token", ""))
        self.github_api = GitHubAPI(gh_token) if gh_token else None

        # Monitors
        self.github_notif = GitHubNotificationMonitor(
            config, self.github_api, self._handle_notification) if self.github_api else None
        self.github_stats = GitHubStatsMonitor(
            config, self.github_api, self._handle_stats) if self.github_api else None
        self.random_trigger = RandomTriggerMonitor(
            config, self._trigger_stats_display, self._send_emotion)

        # Optional monitors (imported lazily to avoid hard deps)
        self.discord_monitor = None
        self.dbus_monitor = None

        self.shutdown_flag = threading.Event()
        self.logger.info("NotificationService initialized")

    # ---- callbacks ----

    def _handle_notification(self, notif_type, title, message):
        self.logger.info(f"Received {notif_type} notification: {title} - {message}")
        ok = self.mqtt.publish_notification(notif_type, title, message)
        if not ok:
            self.logger.warning(f"Failed to forward {notif_type} notification to SANGI")

    def _handle_stats(self, stats):
        self.logger.info(f"Received GitHub stats update: {stats}")
        ok = self.mqtt.publish_github_stats(stats)
        if not ok:
            self.logger.warning("Failed to forward GitHub stats to SANGI")

    def _handle_discord_stats(self, stats):
        self.logger.info(f"Received Discord stats update: {stats}")
        ok = self.mqtt.publish_discord_stats(stats)
        if not ok:
            self.logger.warning("Failed to forward Discord stats to SANGI")

    def _trigger_stats_display(self):
        pass  # Placeholder for future expansion

    def _send_emotion(self, emotion_id):
        ssid = self.config.get("mqtt", {}).get("ssid")
        ok = self.mqtt.publish_emotion(emotion_id, ssid)
        if not ok:
            self.logger.warning(f"Failed to send emotion {emotion_id} to SANGI")

    # ---- lifecycle ----

    def _signal_handler(self, signum, frame):
        self.logger.info(f"Received signal {signum}, shutting down...")
        self.shutdown_flag.set()

    def start(self):
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)

        try:
            self.logger.info("Connecting to MQTT...")
            if not self.mqtt.connect():
                self.logger.error("Failed to connect to MQTT, exiting")
                return

            self.mqtt.test_connection()

            # Start GitHub notification monitor
            if (self.github_notif
                    and self.config.get("notifications", {}).get("github", {}).get("enabled", False)):
                self.logger.info("Starting GitHub notification monitor...")
                threading.Thread(target=self.github_notif.start, daemon=True).start()

            # Start GitHub stats monitor
            if (self.github_stats
                    and self.config.get("github_stats", {}).get("enabled", False)):
                self.logger.info("Starting GitHub stats monitor...")
                threading.Thread(target=self.github_stats.start, daemon=True).start()

            # Start Discord monitor (lazy import)
            if self.config.get("discord_messages", {}).get("enabled", False):
                try:
                    from sangi_notify.monitors.discord import DiscordMonitor
                    self.discord_monitor = DiscordMonitor(self.config, self._handle_discord_stats)
                    self.logger.info("Starting Discord monitor...")
                    threading.Thread(target=self.discord_monitor.start, daemon=True).start()
                except ImportError:
                    self.logger.warning("discord.py-self not installed, skipping Discord monitor")

            # Start random stats trigger
            if self.config.get("random_stats_trigger", {}).get("enabled", True):
                self.logger.info("Starting random stats trigger...")
                threading.Thread(target=self.random_trigger.start, daemon=True).start()

            self.logger.info("SANGI Notification Service is running. Press Ctrl+C to stop.")
            self.shutdown_flag.wait()

        except KeyboardInterrupt:
            self.logger.info("Keyboard interrupt received")
        except Exception as e:
            self.logger.error(f"Error in notification service: {e}", exc_info=True)
        finally:
            self.shutdown()

    def shutdown(self):
        self.logger.info("Shutting down notification service...")
        if self.github_notif:
            self.github_notif.stop()
        if self.github_stats:
            self.github_stats.stop()
        self.random_trigger.stop()
        self.mqtt.disconnect()
        self.logger.info("Notification service stopped")
