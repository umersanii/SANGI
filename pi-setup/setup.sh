#!/bin/bash
#
# SANGI Notification Service Setup Script
# Sets up notification monitoring for Discord, GitHub, and WhatsApp
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVICE_NAME="sangi-notification-monitor@$(whoami).service"

echo "======================================"
echo "SANGI Notification Service Setup"
echo "======================================"
echo ""

# Check if running on Raspberry Pi or Linux
if [[ ! -f /etc/os-release ]]; then
    echo "ERROR: This script requires a Linux system"
    exit 1
fi

# Check Python version
echo "[1/6] Checking Python installation..."
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is not installed"
    exit 1
fi

PYTHON_VERSION=$(python3 --version | cut -d' ' -f2)
echo "✓ Python $PYTHON_VERSION found"

# Install system dependencies
echo ""
echo "[2/6] Installing system dependencies..."
if command -v apt &> /dev/null; then
    sudo apt update
    sudo apt install -y python3-pip python3-gi python3-dbus libcairo2-dev libgirepository1.0-dev
elif command -v dnf &> /dev/null; then
    sudo dnf install -y python3-pip python3-gobject python3-dbus cairo-devel gobject-introspection-devel
else
    echo "WARNING: Unknown package manager, please install manually:"
    echo "  - python3-pip"
    echo "  - python3-gi (PyGObject)"
    echo "  - python3-dbus"
fi

# Install Python dependencies
echo ""
echo "[3/6] Installing Python dependencies..."
pip3 install -r "$SCRIPT_DIR/requirements.txt"
echo "✓ Python dependencies installed"

# Setup configuration
echo ""
echo "[4/6] Setting up configuration..."
if [[ ! -f "$SCRIPT_DIR/config.json" ]]; then
    cp "$SCRIPT_DIR/config.json.template" "$SCRIPT_DIR/config.json"
    echo "✓ Created config.json from template"
    echo ""
    echo "⚠️  IMPORTANT: Edit config.json and configure:"
    echo "   1. AWS IoT endpoint and certificate paths"
    echo "   2. GitHub personal access token (optional)"
    echo "   3. Enable/disable notification sources"
    echo ""
    read -p "Press Enter to continue after configuring config.json..."
else
    echo "✓ config.json already exists"
fi

# Setup certificates directory
echo ""
echo "[5/6] Setting up certificates..."
mkdir -p "$SCRIPT_DIR/certs"
mkdir -p "$SCRIPT_DIR/logs"

if [[ -z "$(ls -A "$SCRIPT_DIR/certs" 2>/dev/null)" ]]; then
    echo "⚠️  Certificates directory is empty"
    echo "Please copy your AWS IoT certificates to: $SCRIPT_DIR/certs/"
    echo "Required files:"
    echo "  - AmazonRootCA1.pem"
    echo "  - cert.pem (device certificate)"
    echo "  - private.key (private key)"
    echo ""
    read -p "Press Enter after copying certificates..."
else
    echo "✓ Certificates found"
fi

# Install systemd service
echo ""
echo "[6/6] Installing systemd service..."

# Copy service file
sudo cp "$SCRIPT_DIR/sangi-notification-monitor@.service" /etc/systemd/system/

# Update service file with correct paths
sudo sed -i "s|/home/%i/SANGI|$(dirname $(dirname "$SCRIPT_DIR"))|g" /etc/systemd/system/sangi-notification-monitor@.service

# Reload systemd
sudo systemctl daemon-reload

# Enable service
sudo systemctl enable "$SERVICE_NAME"

echo "✓ Systemd service installed and enabled"

# Test connection
echo ""
echo "======================================"
echo "Testing configuration..."
echo "======================================"
echo ""
echo "Starting a test run (Ctrl+C to stop)..."
python3 "$SCRIPT_DIR/notification_service.py"

echo ""
echo "======================================"
echo "Setup Complete!"
echo "======================================"
echo ""
echo "Service installed as: $SERVICE_NAME"
echo ""
echo "Commands:"
echo "  Start:   sudo systemctl start $SERVICE_NAME"
echo "  Stop:    sudo systemctl stop $SERVICE_NAME"
echo "  Status:  sudo systemctl status $SERVICE_NAME"
echo "  Logs:    journalctl -u $SERVICE_NAME -f"
echo ""
echo "Configuration: $SCRIPT_DIR/config.json"
echo "Logs: $SCRIPT_DIR/logs/"
echo ""
echo "The service will start automatically on boot."
echo ""
