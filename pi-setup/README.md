# SANGI Notification Service (Raspberry Pi)

Standalone 24/7 notification monitoring service for Raspberry Pi that forwards Discord, GitHub, and WhatsApp notifications to SANGI robot via MQTT.

## Features

- **Discord Notifications**: Monitors desktop Discord notifications via D-Bus
- **GitHub Notifications**: Polls GitHub API for pull requests, issues, and mentions
- **GitHub Statistics**: Fetches real-time GitHub profile stats (contributions, repos, stars, followers)
- **WhatsApp Notifications**: Monitors desktop WhatsApp notifications via D-Bus (requires desktop app)
- **Auto-Start**: Runs as systemd service, starts on boot
- **MQTT Integration**: Forwards notifications and stats to SANGI via AWS IoT Core
- **Rate Limiting**: Prevents notification spam
- **Logging**: Comprehensive logging to file and console
- **Virtual Environment**: Isolated dependencies with system PyGObject/D-Bus access

## Requirements

- **Hardware**: Raspberry Pi (or any Linux system with D-Bus)
- **OS**: Linux (Raspberry Pi OS, Ubuntu, Debian, etc.)
- **Python**: 3.7+ (3.11+ recommended)
- **Network**: WiFi/Ethernet connection to AWS IoT Core
- **GitHub**: Personal access token (optional, for GitHub monitoring)
- **AWS IoT**: Device certificates and endpoint

## Quick Start

### 1. Installation

```bash
cd ~/SANGI/pi-setup
chmod +x setup.sh
./setup.sh
```

The setup script will:
1. ✅ Check Python 3 installation
2. ✅ Install system dependencies (python3-gi, python3-dbus, build tools)
3. ✅ Create virtual environment with `--system-site-packages` (to access system PyGObject)
4. ✅ Install AWS IoT SDK (`awscrt`, `awsiotsdk`) and `requests`
5. ✅ Create `config.json` from template
6. ✅ Setup `certs/` and `logs/` directories
7. ✅ Install systemd service file
8. ✅ Enable service for auto-start

**Note**: The setup uses a virtual environment with `--system-site-packages` to access system-installed PyGObject and D-Bus libraries while keeping AWS packages isolated. This avoids PEP 668 restrictions and complex builds.

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
  },
  "github_stats": {
    "enabled": true,
    "token": "ghp_your_token_here",
    "username": "your_github_username",
    "poll_interval": 300
  }
}
```

### 3. Add Certificates

Copy your AWS IoT certificates to the `certs/` directory:

```bash
cd ~/SANGI/pi-setup
cp ~/path/to/AmazonRootCA1.pem ./certs/
cp ~/path/to/certificate.pem ./certs/cert.pem
cp ~/path/to/private.key ./certs/private.key

# Set proper permissions
chmod 600 ./certs/private.key
chmod 644 ./certs/*.pem
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

### GitHub Statistics

GitHub statistics are fetched periodically via GitHub REST API and GraphQL API.

**Setup**:
1. Use the same GitHub token as notifications (or generate a new one)
2. Required scopes: `repo`, `read:user` (for private repo stats and user data)
3. Add token and username to `config.json` under `github_stats`

**Statistics Collected**:
- **Profile Stats**: Public repos, followers, following
- **Contributions**: Total contributions in last 365 days
- **Commits**: Total commit contributions in last year
- **Pull Requests**: PRs opened and reviews submitted
- **Issues**: Issues created/commented on
- **Stars**: Total stars received across all repos

**Polling Interval**: 300 seconds (5 minutes) - configurable

**Customization**:
Choose which stats to fetch by modifying `stats_types` in config:
```json
"github_stats": {
  "stats_types": ["repos", "followers", "contributions", "commits", "prs", "issues", "stars"]
}
```

**Example Stats Payload**:
```json
{
  "username": "your_username",
  "followers": 42,
  "following": 15,
  "public_repos": 25,
  "total_stars": 150,
  "total_contributions": 1247,
  "total_commits": 856,
  "total_prs": 45,
  "total_issues": 23,
  "timestamp": "2025-10-29T12:00:00Z"
}
```

**MQTT Topic**: Stats are published to `sangi/github/stats` (configurable in `mqtt.stats_topic`)

### Random Stats Display Trigger

The service can automatically trigger SANGI to display stats randomly during workspace mode.

**How it Works**:
1. Every 5 minutes (configurable), randomly sends emotion command to SANGI
2. Triggers `EMOTION_GITHUB_STATS` to display the latest fetched stats
3. Can be expanded to show notifications, Discord messages, etc.

**Configuration**:
```json
"random_stats_trigger": {
  "enabled": true,
  "interval": 300,
  "trigger_types": ["github_stats"]
}
```

**Trigger Types**:
- `github_stats`: Display GitHub statistics animation
- `notifications`: (Planned) Review recent notifications
- `discord`: (Planned) Show Discord message summary
- `whatsapp`: (Planned) Show WhatsApp message summary

**Benefits**:
- Keeps SANGI engaging during long coding sessions
- Provides motivational stats updates
- Non-intrusive (respects workspace mode timing)

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

Before starting the systemd service, test manually to verify configuration:

```bash
cd ~/SANGI/pi-setup
./venv/bin/python notification_service.py
```

Press `Ctrl+C` to stop. You should see:
- MQTT connection attempt
- Service initialization
- D-Bus monitor starting
- GitHub poller starting (if enabled)

### Verify Dependencies

```bash
cd ~/SANGI/pi-setup
./venv/bin/python -c "import gi; import dbus; import awsiotsdk; print('✓ All imports OK')"
```

Expected output: `✓ All imports OK`

### Test Certificates

```bash
ls -l ~/SANGI/pi-setup/certs/
# Should show: AmazonRootCA1.pem, cert.pem, private.key
```

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

### Test GitHub Stats API

```python
import requests

# Test REST API
headers = {
    'Authorization': 'token ghp_your_token',
    'Accept': 'application/vnd.github.v3+json'
}
response = requests.get('https://api.github.com/users/your_username', headers=headers)
print("User data:", response.json())

# Test GraphQL API
graphql_headers = {
    'Authorization': 'bearer ghp_your_token',
    'Content-Type': 'application/json'
}
query = """
{
  viewer {
    login
    contributionsCollection {
      contributionCalendar {
        totalContributions
      }
    }
  }
}
"""
response = requests.post(
    'https://api.github.com/graphql',
    json={'query': query},
    headers=graphql_headers
)
print("GraphQL data:", response.json())
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
journalctl -u sangi-notification-monitor@$(whoami).service -n 100 --no-pager
```

### Python Import Errors

The setup uses a virtual environment with `--system-site-packages` to access system PyGObject and D-Bus.

**Verify system packages are installed**:
```bash
dpkg -l | grep -E 'python3-gi|python3-dbus'
# Should show: python3-gi, python3-dbus
```

**Test imports**:
```bash
cd ~/SANGI/pi-setup
./venv/bin/python -c "import gi; print('✓ PyGObject OK')"
./venv/bin/python -c "import dbus; print('✓ D-Bus OK')"
./venv/bin/python -c "import awsiotsdk; print('✓ AWS IoT SDK OK')"
```

**If PyGObject or D-Bus imports fail**, ensure system packages are installed:
```bash
sudo apt update
sudo apt install -y python3-gi python3-dbus
```

### MQTT Connection Failed

- Verify AWS IoT endpoint in `config.json` (format: `xxxxx-ats.iot.region.amazonaws.com`)
- Check certificate paths are correct (relative to `pi-setup/` directory)
- Ensure certificates have correct permissions:
  ```bash
  chmod 600 ./certs/private.key
  chmod 644 ./certs/*.pem
  ```
- Test network connectivity:
  ```bash
  ping xxxxx-ats.iot.us-east-1.amazonaws.com
  ```

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

### GitHub Statistics Not Working

- Verify token has `repo` and `read:user` scopes
- Check username matches your GitHub login
- Test GraphQL endpoint (requires `bearer` token format)
- Review logs for API errors:
  ```bash
  journalctl -u sangi-notification-monitor@$(whoami).service | grep GitHubStats
  ```
- Verify polling interval isn't too aggressive (minimum 60s recommended)
- Check for API rate limits (5000 requests/hour, GraphQL has separate quota)

## File Structure

```
pi-setup/
├── notification_service.py          # Main service orchestrator
├── config.json                      # Configuration (gitignored)
├── config.json.template             # Configuration template
├── requirements.txt                 # Python dependencies
├── setup.sh                         # Installation script
├── sangi-notification-monitor@.service  # Systemd service file
├── test_service.py                  # Service test script
├── venv/                            # Python virtual environment (created by setup.sh)
│   ├── bin/
│   │   └── python                   # Venv Python interpreter
│   └── lib/
│       └── python3.11/
│           └── site-packages/       # AWS IoT packages installed here
├── lib/
│   ├── __init__.py
│   ├── notification_monitor.py      # D-Bus notification capture
│   ├── github_monitor.py            # GitHub API polling
│   ├── github_stats.py              # GitHub statistics fetcher
│   └── mqtt_publisher.py            # AWS IoT MQTT publisher
├── certs/                           # AWS IoT certificates (created by setup.sh)
│   ├── AmazonRootCA1.pem           # (copy manually)
│   ├── cert.pem                     # (copy manually)
│   └── private.key                  # (copy manually)
└── logs/                            # Log files (created by setup.sh)
    └── notification_monitor.log
```

**Note**: The `venv/` uses `--system-site-packages` to access system PyGObject (`python3-gi`) and D-Bus (`python3-dbus`) while keeping AWS packages isolated.

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

# Remove virtual environment and config (optional)
cd ~/SANGI/pi-setup
rm -rf venv/ config.json logs/

# Keep certs if you plan to reinstall
# rm -rf certs/
```

## Technical Notes

### Virtual Environment Strategy

The setup creates a virtual environment with `--system-site-packages` enabled. This approach:

✅ **Avoids PEP 668 restrictions** - No `--break-system-packages` needed  
✅ **Prevents build issues** - Uses system-compiled PyGObject (requires girepository-2.0 otherwise)  
✅ **Isolates AWS packages** - Keeps `awscrt` and `awsiotsdk` in venv  
✅ **Maintains compatibility** - Works across Debian, Ubuntu, Raspberry Pi OS  

The venv includes:
- System packages: `PyGObject` (python3-gi), `dbus-python` (python3-dbus)
- Venv packages: `awscrt>=0.16.0`, `awsiotsdk>=1.11.0`, `requests>=2.28.0`

### Systemd Service

The service file uses template units (`@.service`) to support multiple users:
- `%i` = username (from `sangi-notification-monitor@umersani.service`)
- `%U` = user UID (for D-Bus session bus path)
- Runs as the specified user (not root)
- Includes D-Bus session bus environment variable

## Support

For issues or questions, see:
- Main project README: `../../README.md`
- Code reference: `../../docs/Code_Reference.md`
- GitHub issues: https://github.com/umersanii/SANGI/issues
