"""
GitHub statistics monitor using GitHub API
Fetches user statistics and contribution data
"""

import logging
import time
import requests
from datetime import datetime, timezone, timedelta


class GitHubStatsMonitor:
    """Monitor GitHub user statistics via REST API and GraphQL"""
    
    def __init__(self, config, callback):
        """
        Initialize GitHub stats monitor
        
        Args:
            config: Configuration dictionary with GitHub settings
            callback: Function to call when stats are updated (stats_dict)
        """
        self.config = config
        self.callback = callback
        self.logger = logging.getLogger('GitHubStatsMonitor')
        
        github_config = config.get('github_stats', {})
        self.enabled = github_config.get('enabled', False)
        self.token = github_config.get('token', '')
        self.username = github_config.get('username', '')
        self.poll_interval = github_config.get('poll_interval', 300)  # 5 minutes default
        self.stats_types = github_config.get('stats_types', [
            'repos', 'followers', 'contributions', 'commits', 'prs', 'issues', 'stars'
        ])
        
        self.headers = {
            'Authorization': f'token {self.token}',
            'Accept': 'application/vnd.github.v3+json'
        }
        
        self.graphql_headers = {
            'Authorization': f'bearer {self.token}',
            'Content-Type': 'application/json'
        }
        
        self.last_stats = {}
        self.last_poll_time = None
        
        if not self.enabled:
            self.logger.warning("GitHub stats monitoring is disabled in config")
        elif not self.token or not self.username:
            self.logger.error("GitHub token or username not configured")
            self.enabled = False
        else:
            self.logger.info(f"GitHubStatsMonitor initialized for user: {self.username}")
    
    def _fetch_user_data(self):
        """Fetch basic user data from REST API"""
        try:
            url = f'https://api.github.com/users/{self.username}'
            response = requests.get(url, headers=self.headers, timeout=10)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            self.logger.error(f"Failed to fetch user data: {e}")
            return None
    
    def _fetch_user_repos(self):
        """Fetch user repositories"""
        try:
            url = f'https://api.github.com/users/{self.username}/repos'
            params = {
                'per_page': 100,
                'sort': 'updated',
                'type': 'owner'  # Only repos owned by user
            }
            response = requests.get(url, headers=self.headers, params=params, timeout=10)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            self.logger.error(f"Failed to fetch repos: {e}")
            return []
    
    def _fetch_contributions_graphql(self):
        """Fetch contribution statistics using GraphQL API"""
        try:
            # Get date range for last year
            today = datetime.now(timezone.utc)
            one_year_ago = today - timedelta(days=365)
            
            query = """
            query($username: String!, $from: DateTime!, $to: DateTime!) {
              user(login: $username) {
                contributionsCollection(from: $from, to: $to) {
                  contributionCalendar {
                    totalContributions
                  }
                  totalCommitContributions
                  totalIssueContributions
                  totalPullRequestContributions
                  totalPullRequestReviewContributions
                }
              }
            }
            """
            
            variables = {
                'username': self.username,
                'from': one_year_ago.isoformat(),
                'to': today.isoformat()
            }
            
            response = requests.post(
                'https://api.github.com/graphql',
                json={'query': query, 'variables': variables},
                headers=self.graphql_headers,
                timeout=10
            )
            response.raise_for_status()
            
            data = response.json()
            if 'errors' in data:
                self.logger.error(f"GraphQL errors: {data['errors']}")
                return None
            
            return data.get('data', {}).get('user', {}).get('contributionsCollection', {})
            
        except requests.exceptions.RequestException as e:
            self.logger.error(f"Failed to fetch contributions: {e}")
            return None
    
    def _calculate_stats(self):
        """Calculate all requested statistics"""
        stats = {}
        
        # Fetch user data for basic stats
        if any(stat in self.stats_types for stat in ['followers', 'repos', 'profile']):
            user_data = self._fetch_user_data()
            if user_data:
                if 'followers' in self.stats_types:
                    stats['followers'] = user_data.get('followers', 0)
                    stats['following'] = user_data.get('following', 0)
                
                if 'repos' in self.stats_types or 'profile' in self.stats_types:
                    stats['public_repos'] = user_data.get('public_repos', 0)
        
        # Fetch detailed repo data for stars
        if 'stars' in self.stats_types:
            repos = self._fetch_user_repos()
            total_stars = sum(repo.get('stargazers_count', 0) for repo in repos)
            stats['total_stars'] = total_stars
            stats['repos_with_stars'] = sum(1 for repo in repos if repo.get('stargazers_count', 0) > 0)
        
        # Fetch contribution stats using GraphQL
        if any(stat in self.stats_types for stat in ['contributions', 'commits', 'prs', 'issues']):
            contrib_data = self._fetch_contributions_graphql()
            if contrib_data:
                calendar = contrib_data.get('contributionCalendar', {})
                
                if 'contributions' in self.stats_types:
                    stats['total_contributions'] = calendar.get('totalContributions', 0)
                
                if 'commits' in self.stats_types:
                    stats['total_commits'] = contrib_data.get('totalCommitContributions', 0)
                
                if 'prs' in self.stats_types:
                    stats['total_prs'] = contrib_data.get('totalPullRequestContributions', 0)
                    stats['total_pr_reviews'] = contrib_data.get('totalPullRequestReviewContributions', 0)
                
                if 'issues' in self.stats_types:
                    stats['total_issues'] = contrib_data.get('totalIssueContributions', 0)
        
        return stats
    
    def fetch_stats(self):
        """Fetch GitHub statistics (single check)"""
        if not self.enabled:
            return None
        
        try:
            self.logger.info("Fetching GitHub statistics...")
            stats = self._calculate_stats()
            
            if stats:
                stats['username'] = self.username
                stats['timestamp'] = datetime.now(timezone.utc).isoformat()
                stats['last_year'] = True  # Indicates contribution data is from last 365 days
                
                self.logger.info(f"GitHub stats: {stats}")
                self.last_stats = stats
                self.last_poll_time = time.time()
                
                # Call callback with stats
                self.callback(stats)
                
                return stats
            else:
                self.logger.warning("Failed to fetch complete statistics")
                return None
            
        except Exception as e:
            self.logger.error(f"Error fetching GitHub stats: {e}", exc_info=True)
            return None
    
    def get_last_stats(self):
        """Get the last fetched statistics"""
        return self.last_stats
    
    def start_polling(self):
        """Start polling GitHub API for stats (blocking loop)"""
        if not self.enabled:
            self.logger.warning("GitHub stats monitoring disabled, not starting poll")
            return
        
        self.logger.info(f"Starting GitHub stats polling (interval: {self.poll_interval}s)")
        
        try:
            # Fetch immediately on start
            self.fetch_stats()
            
            while True:
                time.sleep(self.poll_interval)
                self.fetch_stats()
                
        except KeyboardInterrupt:
            self.logger.info("GitHub stats polling stopped by user")
        except Exception as e:
            self.logger.error(f"Error in GitHub stats polling: {e}", exc_info=True)
            raise
