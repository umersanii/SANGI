"""Random stats/notification trigger — periodically sends emotion commands to SANGI."""

import random
import time
from typing import Callable

from sangi_notify.monitors.base import Monitor


class RandomTriggerMonitor(Monitor):
    """Randomly trigger stats display on SANGI."""

    def __init__(self, config: dict,
                 stats_callback: Callable[[], None],
                 emotion_callback: Callable[[int], None]):
        super().__init__(config, "RandomTriggerMonitor")

        trigger_config = config.get("random_stats_trigger", {})
        self.enabled = trigger_config.get("enabled", True)
        self._interval_seconds = trigger_config.get("interval", 300)
        self.trigger_types = trigger_config.get("trigger_types", ["github_stats"])

        self.stats_callback = stats_callback
        self.emotion_callback = emotion_callback
        self.last_trigger_time: float | None = None

        if not self.enabled:
            self.logger.warning("Random stats trigger is disabled in config")
        else:
            self.logger.info(f"RandomTriggerMonitor initialized (interval: {self._interval_seconds}s)")

    @property
    def interval(self) -> float:
        return 10  # Check every 10 seconds (actual trigger governed by _interval_seconds).

    def poll(self) -> None:
        if not self.enabled:
            return

        now = time.time()

        if self.last_trigger_time is None:
            # First poll — add random delay to avoid immediate trigger on start.
            self.last_trigger_time = now + random.randint(0, 60)
            return

        if (now - self.last_trigger_time) < self._interval_seconds:
            return

        self.last_trigger_time = now
        self._trigger_random_display()

    def _trigger_random_display(self):
        available = []
        if "github_stats" in self.trigger_types and self.config.get("github_stats", {}).get("enabled", False):
            available.append("github_stats")
        if "notifications" in self.trigger_types:
            available.append("notifications")

        if not available:
            self.logger.warning("No available trigger types enabled")
            return

        selected = random.choice(available)
        self.logger.info(f"Triggering random display: {selected}")

        if selected == "github_stats":
            try:
                self.emotion_callback(15)  # EMOTION_GITHUB_STATS
                self.logger.info("Sent EMOTION_GITHUB_STATS to SANGI")
            except Exception as e:
                self.logger.error(f"Failed to trigger GitHub stats: {e}", exc_info=True)
