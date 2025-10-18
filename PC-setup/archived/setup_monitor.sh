#!/bin/bash
# Quick start script for SANGI PC workspace monitor with logging

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=================================="
echo "SANGI PC Workspace Monitor Setup"
echo "=================================="
echo ""

# Check Python
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 not found. Please install Python 3."
    exit 1
fi
echo "✅ Python 3 found"

# Check dependencies
echo "📦 Checking Python dependencies..."
python3 -c "import AWSIoTPythonSDK" 2>/dev/null || {
    echo "⚠️  AWSIoTPythonSDK not found. Installing..."
    pip3 install --user AWSIoTPythonSDK
}

python3 -c "import watchdog" 2>/dev/null || {
    echo "⚠️  watchdog not found. Installing..."
    pip3 install --user watchdog
}

python3 -c "import psutil" 2>/dev/null || {
    echo "⚠️  psutil not found. Installing..."
    pip3 install --user psutil
}

echo "✅ All dependencies installed"

# Check config
if [ ! -f "config.json" ]; then
    echo "❌ config.json not found!"
    echo "Please create config.json from config.json.template"
    exit 1
fi
echo "✅ Configuration file found"

# Check certificates
CERT_PATH=$(python3 -c "import json; print(json.load(open('config.json'))['certificate_path'])")
if [ ! -f "$CERT_PATH" ]; then
    echo "❌ Certificate not found: $CERT_PATH"
    echo "Please check your config.json certificate paths"
    exit 1
fi
echo "✅ Certificates found"

# Create logs directory
mkdir -p logs
echo "✅ Logs directory ready: $SCRIPT_DIR/logs"

# Test connection
echo ""
echo "🔧 Testing AWS IoT connection..."
if python3 test_connection.py; then
    echo ""
    echo "=================================="
    echo "✅ Setup Complete!"
    echo "=================================="
    echo ""
    echo "Start monitoring:"
    echo "  python3 workspace_monitor_v2.py"
    echo ""
    echo "View logs:"
    echo "  python3 view_logs.py"
    echo ""
    echo "Follow live:"
    echo "  python3 view_logs.py --tail"
    echo ""
    echo "Install as service:"
    echo "  sudo cp sangi-workspace-monitor.service /etc/systemd/system/sangi-workspace-monitor@$USER.service"
    echo "  sudo systemctl daemon-reload"
    echo "  sudo systemctl enable sangi-workspace-monitor@$USER"
    echo "  sudo systemctl start sangi-workspace-monitor@$USER"
    echo ""
else
    echo ""
    echo "=================================="
    echo "⚠️  Connection Test Failed"
    echo "=================================="
    echo ""
    echo "Please check:"
    echo "  1. AWS IoT thing is created (Sani-PC)"
    echo "  2. Policy allows client_id: Sani-PC"
    echo "  3. Certificate is attached to thing"
    echo "  4. Endpoint is correct in config.json"
    echo ""
    echo "See AWS_IOT_SETUP.md for detailed setup instructions"
    echo ""
fi
