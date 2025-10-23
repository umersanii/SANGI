"""
GitHub notifications monitor using GitHub API
"""

import logging
import time
import requests
from datetime import datetime, timezone


class GitHubMonitor:
    """Monitor GitHub notifications via REST API"""
    
    def __init__(self, config, callback):
        """
        Initialize GitHub monitor
        
        Args:
            config: Configuration dictionary with GitHub settings
            callback: Function to call when notification received (type, title, message)
        """
        self.config = config
        self.callback = callback
        self.logger = logging.getLogger('GitHubMonitor')
        
        github_config = config.get('notifications', {}).get('github', {})
        self.enabled = github_config.get('enabled', False)
        self.token = github_config.get('token', '')
        self.username = github_config.get('username', '')
        self.check_interval = github_config.get('check_interval', 60)
        self.monitor_types = github_config.get('monitor_types', ['pull_requests', 'issues'])
        
        self.headers = {
            'Authorization': f'token {self.token}',
            'Accept': 'application/vnd.github.v3+json'
        }
        
        self.last_check_time = None
        self.processed_notifications = set()
        
        if not self.enabled:
            self.logger.warning("GitHub monitoring is disabled in config")
        elif not self.token or not self.username:
            self.logger.error("GitHub token or username not configured")
            self.enabled = False
        else:
            self.logger.info(f"GitHubMonitor initialized for user: {self.username}")
    
    def _fetch_notifications(self):
        """Fetch unread notifications from GitHub API"""
        try:
            url = 'https://api.github.com/notifications'
            params = {
                'participating': 'true',  # Only notifications where user is directly involved
                'per_page': 20
            }
            
            response = requests.get(url, headers=self.headers, params=params, timeout=10)
            response.raise_for_status()
            
            return response.json()
            
        except requests.exceptions.RequestException as e:
            self.logger.error(f"Failed to fetch GitHub notifications: {e}")
            return []
    
    def _parse_notification(self, notif):
        """
        Parse GitHub notification into SANGI format
        
        Returns:
            (type, title, message) tuple or None if should be ignored
        """
        notif_id = notif.get('id')
        
        # Skip if already processed
        if notif_id in self.processed_notifications:
            return None
        
        subject = notif.get('subject', {})
        subject_type = subject.get('type', '').lower()
        subject_title = subject.get('title', 'Unknown')
        
        # Filter by monitor types
        if 'pull_requests' not in self.monitor_types and subject_type == 'pullrequest':
            return None
        if 'issues' not in self.monitor_types and subject_type == 'issue':
            return None
        
        repo = notif.get('repository', {}).get('full_name', 'Unknown repo')
        reason = notif.get('reason', 'notification')
        
        # Format title and message
        if subject_type == 'pullrequest':
            title = f"PR: {repo}"
            message = subject_title[:40]
        elif subject_type == 'issue':
            title = f"Issue: {repo}"
            message = subject_title[:40]
        elif reason == 'mention':
            title = f"Mentioned in {repo}"
            message = subject_title[:40]
        else:
            title = f"GitHub: {repo}"
            message = subject_title[:40]
        
        # Mark as processed
        self.processed_notifications.add(notif_id)
        
        # Clean up old processed IDs (keep last 100)
        if len(self.processed_notifications) > 100:
            self.processed_notifications = set(list(self.processed_notifications)[-100:])
        
        return ('github', title, message)
    
    def check_notifications(self):
        """Check for new GitHub notifications (single check)"""
        if not self.enabled:
            return
        
        try:
            notifications = self._fetch_notifications()
            
            for notif in notifications:
                parsed = self._parse_notification(notif)
                if parsed:
                    notif_type, title, message = parsed
                    self.logger.info(f"GitHub notification: {title} - {message}")
                    self.callback(notif_type, title, message)
            
            self.last_check_time = time.time()
            
        except Exception as e:
            self.logger.error(f"Error checking GitHub notifications: {e}", exc_info=True)
    
    def start_polling(self):
        """Start polling GitHub API (blocking loop)"""
        if not self.enabled:
            self.logger.warning("GitHub monitoring disabled, not starting poll")
            return
        
        self.logger.info(f"Starting GitHub polling (interval: {self.check_interval}s)")
        
        try:
            while True:
                self.check_notifications()
                time.sleep(self.check_interval)
                
        except KeyboardInterrupt:
            self.logger.info("GitHub polling stopped by user")
        except Exception as e:
            self.logger.error(f"Error in GitHub polling: {e}", exc_info=True)
            raise
