"""GitHub notification monitor — polls the notifications API."""

import time
from typing import Callable, Optional, Tuple

from sangi_notify.monitors.base import Monitor
from sangi_notify.monitors.github_api import GitHubAPI


class GitHubNotificationMonitor(Monitor):
    """Monitor GitHub notifications via REST API."""

    def __init__(self, config: dict, github_api: GitHubAPI,
                 callback: Callable[[str, str, str], None]):
        super().__init__(config, "GitHubNotificationMonitor")

        gh_config = config.get("notifications", {}).get("github", {})
        self.enabled = gh_config.get("enabled", False)
        self.username = gh_config.get("username", "")
        self._interval = gh_config.get("check_interval", 60)
        self.monitor_types = gh_config.get("monitor_types", ["pull_requests", "issues"])

        self.api = github_api
        self.callback = callback
        self.processed_notifications: set = set()

        if not self.enabled:
            self.logger.warning("GitHub monitoring is disabled in config")
        elif not self.username:
            self.logger.error("GitHub username not configured")
            self.enabled = False
        else:
            self.logger.info(f"GitHubNotificationMonitor initialized for user: {self.username}")

    @property
    def interval(self) -> float:
        return self._interval

    def poll(self) -> None:
        if not self.enabled:
            return

        notifications = self._fetch_notifications()
        for notif in notifications:
            parsed = self._parse_notification(notif)
            if parsed:
                notif_type, title, message = parsed
                self.logger.info(f"GitHub notification: {title} - {message}")
                self.callback(notif_type, title, message)

    def _fetch_notifications(self) -> list:
        try:
            resp = self.api.get("/notifications", params={
                "participating": "true",
                "per_page": 20,
            })
            return resp.json()
        except Exception as e:
            self.logger.error(f"Failed to fetch GitHub notifications: {e}")
            return []

    def _parse_notification(self, notif: dict) -> Optional[Tuple[str, str, str]]:
        notif_id = notif.get("id")
        if notif_id in self.processed_notifications:
            return None

        subject = notif.get("subject", {})
        subject_type = subject.get("type", "").lower()
        subject_title = subject.get("title", "Unknown")

        if "pull_requests" not in self.monitor_types and subject_type == "pullrequest":
            return None
        if "issues" not in self.monitor_types and subject_type == "issue":
            return None

        repo = notif.get("repository", {}).get("full_name", "Unknown repo")
        reason = notif.get("reason", "notification")

        if subject_type == "pullrequest":
            title = f"PR: {repo}"
        elif subject_type == "issue":
            title = f"Issue: {repo}"
        elif reason == "mention":
            title = f"Mentioned in {repo}"
        else:
            title = f"GitHub: {repo}"

        message = subject_title[:40]

        self.processed_notifications.add(notif_id)
        # Keep last 100
        if len(self.processed_notifications) > 100:
            self.processed_notifications = set(list(self.processed_notifications)[-100:])

        return ("github", title, message)
