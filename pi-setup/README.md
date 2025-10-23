# SANGI Notification Service

Standalone 24/7 notification monitoring service for Raspberry Pi that forwards Discord, GitHub, and WhatsApp notifications to SANGI robot via MQTT.

## Features

- **Discord Notifications**: Monitors desktop Discord notifications via D-Bus
- **GitHub Notifications**: Polls GitHub API for pull requests, issues, and mentions
- **WhatsApp Notifications**: Monitors desktop WhatsApp notifications via D-Bus (requires desktop app)
- **Auto-Start**: Runs as systemd service, starts on boot
- **MQTT Integration**: Forwards notifications to SANGI via AWS IoT Core
- **Rate Limiting**: Prevents notification spam
- **Logging**: Comprehensive logging to file and console

## Requirements

- **Hardware**: Raspberry Pi (or any Linux system with D-Bus)
- **OS**: Linux (Raspberry Pi OS, Ubuntu, etc.)
- **Python**: 3.7+
- **Network**: WiFi/Ethernet connection to AWS IoT Core
- **GitHub**: Personal access token (optional, for GitHub monitoring)
- **AWS IoT**: Device certificates and endpoint

## Quick Start

### 1. Installation

```bash
cd /media/sani/work/Coding/SANGI/PC-setup/notification-service
./setup.sh
```

The setup script will:
- Install system dependencies (Python, D-Bus, etc.)
- Install Python packages
- Create `config.json` from template
- Setup certificates directory
- Install systemd service
- Test the connection

### 2. Configuration

Edit `config.json`:

```json
{
  "mqtt": {
    "endpoint": "xxxxx-ats.iot.us-east-1.amazonaws.com",
    "certificate_path": "./certs/cert.pem",
    "private_key_path": "./certs/private.key",
    "root_ca_path": "./certs/AmazonRootCA1.pem"
  },
  "notifications": {
    "discord": {
      "enabled": true
    },
    "github": {
      "enabled": true,
      "token": "ghp_your_token_here",
      "username": "your_github_username"
    }
  }
}
```

### 3. Add Certificates

Copy your AWS IoT certificates to `certs/` directory:

```bash
cp ~/path/to/AmazonRootCA1.pem ./certs/
cp ~/path/to/certificate.pem ./certs/cert.pem
cp ~/path/to/private.key ./certs/private.key
```

### 4. Start Service

```bash
# Start service
sudo systemctl start sangi-notification-monitor@$(whoami).service

# Enable auto-start on boot
sudo systemctl enable sangi-notification-monitor@$(whoami).service

# Check status
sudo systemctl status sangi-notification-monitor@$(whoami).service

# View logs
journalctl -u sangi-notification-monitor@$(whoami).service -f
```

## Configuration Details

### Discord Notifications

Discord notifications are captured via D-Bus from the Discord desktop app.

**Requirements**:
- Discord desktop app installed and running
- Notifications enabled in Discord settings

**Format**:
- Username extracted from notification
- Message simplified to "new message"

**Example**:
```
Discord notification: friend_username - new message
```

### GitHub Notifications

GitHub notifications are fetched via GitHub REST API.

**Setup**:
1. Generate personal access token at https://github.com/settings/tokens
2. Required scopes: `notifications`, `repo` (for private repos)
3. Add token to `config.json`

**Monitored Types**:
- Pull requests
- Issues
- Mentions
- Review requests

**Polling Interval**: 60 seconds (configurable)

### WhatsApp Notifications

WhatsApp notifications are captured via D-Bus from WhatsApp desktop app.

**Requirements**:
- WhatsApp desktop app installed and running
- Notifications enabled

**Note**: WhatsApp Business API is also supported but requires business account setup.

## Service Management

```bash
# Start service
sudo systemctl start sangi-notification-monitor@$(whoami).service

# Stop service
sudo systemctl stop sangi-notification-monitor@$(whoami).service

# Restart service
sudo systemctl restart sangi-notification-monitor@$(whoami).service

# Check status
sudo systemctl status sangi-notification-monitor@$(whoami).service

# View real-time logs
journalctl -u sangi-notification-monitor@$(whoami).service -f

# View last 50 lines
journalctl -u sangi-notification-monitor@$(whoami).service -n 50

# Disable auto-start
sudo systemctl disable sangi-notification-monitor@$(whoami).service
```

## Testing

### Manual Test Run

```bash
cd /media/sani/work/Coding/SANGI/PC-setup/notification-service
python3 notification_service.py
```

Press `Ctrl+C` to stop.

### Test GitHub API

```python
import requests

headers = {
    'Authorization': 'token ghp_your_token',
    'Accept': 'application/vnd.github.v3+json'
}

response = requests.get('https://api.github.com/notifications', headers=headers)
print(response.json())
```

### Test D-Bus Notifications

```bash
# Send test notification
notify-send "Test Title" "Test Message"
```

Watch the service logs to see if it's captured.

## Troubleshooting

### Service Won't Start

```bash
# Check service status
sudo systemctl status sangi-notification-monitor@$(whoami).service

# View detailed logs
journalctl -u sangi-notification-monitor@$(whoami).service -n 100
```

### MQTT Connection Failed

- Verify AWS IoT endpoint in `config.json`
- Check certificate paths are correct
- Ensure certificates have correct permissions (readable)
- Test network connectivity to AWS

### Discord Notifications Not Working

- Ensure Discord desktop app is running
- Check notifications are enabled in Discord settings
- Verify D-Bus session is available
- Test with: `dbus-monitor "interface='org.freedesktop.Notifications'"`

### GitHub Notifications Not Working

- Verify token has correct scopes
- Check token hasn't expired
- Test API manually (see Testing section)
- Check rate limiting (5000 requests/hour for authenticated)

## File Structure

```
notification-service/
├── notification_service.py          # Main service orchestrator
├── config.json                      # Configuration (gitignored)
├── config.json.template             # Configuration template
├── requirements.txt                 # Python dependencies
├── setup.sh                         # Installation script
├── sangi-notification-monitor@.service  # Systemd service file
├── lib/
│   ├── __init__.py
│   ├── notification_monitor.py      # D-Bus notification capture
│   ├── github_monitor.py            # GitHub API polling
│   └── mqtt_publisher.py            # AWS IoT MQTT publisher
├── certs/
│   ├── AmazonRootCA1.pem
│   ├── cert.pem
│   └── private.key
└── logs/
    └── notification_monitor.log
```

## Rate Limiting

Default configuration:
- Minimum 5 seconds between notifications from same source
- Maximum 10 notifications per minute
- GitHub API: 60 second polling interval

Adjust in `config.json`:
```json
"filters": {
  "min_notification_interval": 5,
  "max_notifications_per_minute": 10
}
```

## Security

- AWS IoT certificates should be kept secure (600 permissions)
- GitHub token should never be committed to git
- `config.json` is gitignored by default
- Service runs as non-root user

## Logs

Logs are written to:
- `./logs/notification_monitor.log` (rotating, max 10MB)
- System journal (via systemd)

View logs:
```bash
# Service logs
journalctl -u sangi-notification-monitor@$(whoami).service -f

# File logs
tail -f logs/notification_monitor.log
```

## Uninstallation

```bash
# Stop and disable service
sudo systemctl stop sangi-notification-monitor@$(whoami).service
sudo systemctl disable sangi-notification-monitor@$(whoami).service

# Remove service file
sudo rm /etc/systemd/system/sangi-notification-monitor@.service
sudo systemctl daemon-reload

# Remove installation directory
rm -rf /media/sani/work/Coding/SANGI/PC-setup/notification-service
```

## Support

For issues or questions, see:
- Main project README: `../../README.md`
- Code reference: `../../docs/Code_Reference.md`
- GitHub issues: https://github.com/umersanii/SANGI/issues
