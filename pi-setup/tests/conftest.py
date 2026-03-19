"""Pytest fixtures for sangi_notify tests."""

import pytest


@pytest.fixture
def sample_config():
    """Minimal config dict for testing."""
    return {
        "mqtt": {
            "endpoint": "test.iot.us-east-1.amazonaws.com",
            "certificate_path": "/tmp/cert.pem",
            "private_key_path": "/tmp/key.pem",
            "root_ca_path": "/tmp/ca.pem",
            "client_id": "test-client",
        },
        "notifications": {
            "github": {
                "enabled": False,
                "token": "ghp_test",
                "username": "testuser",
                "check_interval": 60,
                "monitor_types": ["pull_requests", "issues"],
            },
        },
        "github_stats": {
            "enabled": False,
            "token": "ghp_test",
            "username": "testuser",
            "poll_interval": 300,
        },
        "discord_messages": {
            "enabled": False,
        },
        "random_stats_trigger": {
            "enabled": False,
            "interval": 300,
            "trigger_types": ["github_stats"],
        },
        "logging": {
            "level": "DEBUG",
            "file": "/tmp/sangi_test.log",
        },
    }
