"""GitHub statistics monitor — fetches user stats and contribution data."""

import time
from datetime import datetime, timezone, timedelta
from typing import Callable, Optional

from sangi_notify.monitors.base import Monitor
from sangi_notify.monitors.github_api import GitHubAPI


class GitHubStatsMonitor(Monitor):
    """Monitor GitHub user statistics via REST API and GraphQL."""

    def __init__(self, config: dict, github_api: GitHubAPI,
                 callback: Callable[[dict], None]):
        super().__init__(config, "GitHubStatsMonitor")

        gh_config = config.get("github_stats", {})
        self.enabled = gh_config.get("enabled", False)
        self.username = gh_config.get("username", "")
        self._interval = gh_config.get("poll_interval", 300)
        self.stats_types = gh_config.get("stats_types", [
            "repos", "followers", "contributions", "commits", "prs", "issues", "stars",
        ])

        self.api = github_api
        self.callback = callback
        self.last_stats: dict = {}

        if not self.enabled:
            self.logger.warning("GitHub stats monitoring is disabled in config")
        elif not self.username:
            self.logger.error("GitHub username not configured")
            self.enabled = False
        else:
            self.logger.info(f"GitHubStatsMonitor initialized for user: {self.username}")

    @property
    def interval(self) -> float:
        return self._interval

    def poll(self) -> None:
        if not self.enabled:
            return
        self._fetch_and_report()

    def _fetch_and_report(self) -> Optional[dict]:
        self.logger.info("Fetching GitHub statistics...")
        stats = self._calculate_stats()

        if stats:
            stats["username"] = self.username
            stats["timestamp"] = datetime.now(timezone.utc).isoformat()
            stats["last_year"] = True
            self.logger.info(f"GitHub stats: {stats}")
            self.last_stats = stats
            self.callback(stats)
            return stats

        self.logger.warning("Failed to fetch complete statistics")
        return None

    def _calculate_stats(self) -> dict:
        stats: dict = {}

        if any(s in self.stats_types for s in ("followers", "repos", "profile")):
            user_data = self._fetch_user_data()
            if user_data:
                if "followers" in self.stats_types:
                    stats["followers"] = user_data.get("followers", 0)
                    stats["following"] = user_data.get("following", 0)
                if "repos" in self.stats_types or "profile" in self.stats_types:
                    stats["public_repos"] = user_data.get("public_repos", 0)

        if "stars" in self.stats_types:
            repos = self._fetch_user_repos()
            stats["total_stars"] = sum(r.get("stargazers_count", 0) for r in repos)
            stats["repos_with_stars"] = sum(1 for r in repos if r.get("stargazers_count", 0) > 0)

        if any(s in self.stats_types for s in ("contributions", "commits", "prs", "issues")):
            contrib = self._fetch_contributions_graphql()
            if contrib:
                cal = contrib.get("contributionCalendar", {})
                if "contributions" in self.stats_types:
                    stats["total_contributions"] = cal.get("totalContributions", 0)
                if "commits" in self.stats_types:
                    stats["total_commits"] = contrib.get("totalCommitContributions", 0)
                if "prs" in self.stats_types:
                    stats["total_prs"] = contrib.get("totalPullRequestContributions", 0)
                    stats["total_pr_reviews"] = contrib.get("totalPullRequestReviewContributions", 0)
                if "issues" in self.stats_types:
                    stats["total_issues"] = contrib.get("totalIssueContributions", 0)

        return stats

    def _fetch_user_data(self) -> Optional[dict]:
        try:
            return self.api.get(f"/users/{self.username}").json()
        except Exception as e:
            self.logger.error(f"Failed to fetch user data: {e}")
            return None

    def _fetch_user_repos(self) -> list:
        try:
            return self.api.get(f"/users/{self.username}/repos", params={
                "per_page": 100, "sort": "updated", "type": "owner",
            }).json()
        except Exception as e:
            self.logger.error(f"Failed to fetch repos: {e}")
            return []

    def _fetch_contributions_graphql(self) -> Optional[dict]:
        try:
            today = datetime.now(timezone.utc)
            one_year_ago = today - timedelta(days=365)

            query = """
            query($username: String!, $from: DateTime!, $to: DateTime!) {
              user(login: $username) {
                contributionsCollection(from: $from, to: $to) {
                  contributionCalendar { totalContributions }
                  totalCommitContributions
                  totalIssueContributions
                  totalPullRequestContributions
                  totalPullRequestReviewContributions
                }
              }
            }
            """
            variables = {
                "username": self.username,
                "from": one_year_ago.isoformat(),
                "to": today.isoformat(),
            }

            data = self.api.graphql(query, variables)
            if "errors" in data:
                self.logger.error(f"GraphQL errors: {data['errors']}")
                return None
            return data.get("data", {}).get("user", {}).get("contributionsCollection", {})

        except Exception as e:
            self.logger.error(f"Failed to fetch contributions: {e}")
            return None
