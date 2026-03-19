"""D-Bus desktop notification monitor for Discord and WhatsApp notifications."""

import logging
import time

from gi.repository import GLib
from dbus.mainloop.glib import DBusGMainLoop
import dbus

from sangi_notify.monitors.base import Monitor


class DBusNotificationMonitor(Monitor):
    """Monitor desktop notifications via D-Bus.

    Overrides start() because GLib.MainLoop is its own blocking loop.
    """

    def __init__(self, config: dict, callback):
        super().__init__(config, "DBusNotificationMonitor")

        DBusGMainLoop(set_as_default=True)
        self.bus = dbus.SessionBus()
        self.loop = GLib.MainLoop()

        self.callback = callback

        filters = config.get("filters", {})
        self.ignore_apps = filters.get("ignore_apps", [])
        self.min_interval = filters.get("min_notification_interval", 5)
        self.last_notification_time = 0

        self.discord_keywords = ["discord", "Discord"]
        self.whatsapp_keywords = ["whatsapp", "WhatsApp", "Signal"]

        self.logger.info("DBusNotificationMonitor initialized")

    @property
    def interval(self) -> float:
        return 0  # Not used — GLib loop is event-driven.

    def poll(self) -> None:
        pass  # Not used.

    def start(self) -> None:
        """Override to use GLib main loop."""
        try:
            self.bus.add_signal_receiver(
                self._notification_handler,
                dbus_interface="org.freedesktop.Notifications",
                signal_name="Notify",
            )
            self.logger.info("D-Bus notification monitoring started")
            self.loop.run()
        except KeyboardInterrupt:
            self.logger.info("Notification monitoring stopped by user")
            self.stop()
        except Exception as e:
            self.logger.error(f"Error in notification monitoring: {e}", exc_info=True)
            raise

    def stop(self) -> None:
        self.loop.quit()
        super().stop()

    def _notification_handler(self, app_name, replaces_id, app_icon,
                              summary, body, actions, hints, expire_timeout):
        if app_name in self.ignore_apps:
            return

        current_time = time.time()
        if current_time - self.last_notification_time < self.min_interval:
            self.logger.debug(f"Notification from {app_name} rate-limited")
            return

        notif_cfg = self.config.get("notifications", {})

        if (notif_cfg.get("discord", {}).get("enabled", False)
                and self._is_discord(app_name, summary)):
            username = summary.split(" - ")[0].strip() if " - " in summary else summary.strip()
            self.callback("discord", username, "new message")
            self.last_notification_time = current_time
            return

        if (notif_cfg.get("whatsapp", {}).get("enabled", False)
                and self._is_whatsapp(app_name, summary)):
            contact = summary.strip()
            message = body[:50] + "..." if len(body) > 50 else body
            self.callback("whatsapp", contact, message)
            self.last_notification_time = current_time
            return

    def _is_discord(self, app_name, summary):
        return any(kw in app_name or kw in summary for kw in self.discord_keywords)

    def _is_whatsapp(self, app_name, summary):
        return any(kw in app_name or kw in summary for kw in self.whatsapp_keywords)
