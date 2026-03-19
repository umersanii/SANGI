"""Monitor subpackage — all notification source monitors."""

from sangi_notify.monitors.base import Monitor
from sangi_notify.monitors.github_notifications import GitHubNotificationMonitor
from sangi_notify.monitors.github_stats import GitHubStatsMonitor
from sangi_notify.monitors.random_trigger import RandomTriggerMonitor

__all__ = [
    "Monitor",
    "GitHubNotificationMonitor",
    "GitHubStatsMonitor",
    "RandomTriggerMonitor",
]
