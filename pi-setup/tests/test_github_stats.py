#!/usr/bin/env python3
"""
Test script for GitHub stats monitoring
Run this to verify GitHub API integration before deploying as service
"""

import json
import sys
from pathlib import Path

# Add lib to path
sys.path.insert(0, str(Path(__file__).parent / 'lib'))

from lib.github_stats import GitHubStatsMonitor


def print_stats(stats):
    """Pretty print stats callback"""
    print("\n" + "="*60)
    print("GitHub Statistics Updated")
    print("="*60)
    print(json.dumps(stats, indent=2))
    print("="*60 + "\n")


def main():
    """Test GitHub stats fetching"""
    
    # Load config
    config_file = Path(__file__).parent / 'config.json'
    
    if not config_file.exists():
        print("ERROR: config.json not found!")
        print("Please copy config.json.template to config.json and configure it")
        sys.exit(1)
    
    with open(config_file, 'r') as f:
        config = json.load(f)
    
    # Check if GitHub stats is enabled
    github_config = config.get('github_stats', {})
    if not github_config.get('enabled', False):
        print("ERROR: GitHub stats monitoring is disabled in config.json")
        print("Set 'github_stats.enabled' to true")
        sys.exit(1)
    
    if not github_config.get('token') or not github_config.get('username'):
        print("ERROR: GitHub token or username not configured in config.json")
        sys.exit(1)
    
    print(f"Testing GitHub stats for user: {github_config['username']}")
    print(f"Stats types: {github_config.get('stats_types', [])}")
    print("\nFetching statistics from GitHub API...")
    
    # Create monitor
    monitor = GitHubStatsMonitor(config, print_stats)
    
    # Fetch stats once
    stats = monitor.fetch_stats()
    
    if stats:
        print("\n✓ GitHub stats test successful!")
        print(f"\nTo run continuous monitoring, use:")
        print(f"  ./venv/bin/python notification_service.py")
    else:
        print("\n✗ GitHub stats test failed!")
        print("Check the error messages above and verify:")
        print("  1. Token has correct scopes (repo, read:user)")
        print("  2. Username matches your GitHub login")
        print("  3. Network connectivity to api.github.com")
        sys.exit(1)


if __name__ == '__main__':
    main()
