#!/bin/bash
# SANGI PC Workspace Monitor Setup Script

set -e  # Exit on error

echo "🤖 SANGI PC Workspace Monitor Setup"
echo "===================================="
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check Python version
echo "✅ Checking Python version..."
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is not installed. Please install Python 3.7 or higher."
    exit 1
fi

PYTHON_VERSION=$(python3 --version | cut -d' ' -f2)
echo "   Found Python $PYTHON_VERSION"

# Install Python dependencies
echo ""
echo "📦 Installing Python dependencies..."
pip3 install --user AWSIoTPythonSDK psutil watchdog

# Create config file if it doesn't exist
if [ ! -f "config.json" ]; then
    echo ""
    echo "⚙️  Creating config file..."
    cp config.json.template config.json
    echo "   Created config.json from template"
    echo ""
    echo "⚠️  IMPORTANT: Edit config.json with your AWS IoT credentials!"
    echo "   Required fields:"
    echo "   - endpoint: Your AWS IoT endpoint"
    echo "   - certificate_path: Path to device certificate"
    echo "   - private_key_path: Path to private key"
    echo "   - root_ca_path: Path to Amazon Root CA"
    echo "   - watch_paths: Directories to monitor"
    echo ""
else
    echo "   ℹ️  config.json already exists, skipping creation"
fi

# Make workspace_monitor.py executable
echo ""
echo "🔧 Setting permissions..."
chmod +x workspace_monitor.py
echo "   Made workspace_monitor.py executable"

# Optionally setup systemd service
echo ""
read -p "📋 Do you want to setup systemd service for auto-start? (y/n) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    SERVICE_FILE="/etc/systemd/system/sangi-pc-monitor.service"
    
    # Create service file content
    cat > /tmp/sangi-pc-monitor.service << EOF
[Unit]
Description=SANGI Workspace Monitor (PC)
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$SCRIPT_DIR
ExecStart=$(which python3) $SCRIPT_DIR/workspace_monitor.py
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

    # Install service file
    echo "   Installing systemd service..."
    sudo mv /tmp/sangi-pc-monitor.service "$SERVICE_FILE"
    sudo systemctl daemon-reload
    
    echo ""
    read -p "   Enable service to start on boot? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo systemctl enable sangi-pc-monitor.service
        echo "   ✅ Service enabled for auto-start"
    fi
    
    echo ""
    read -p "   Start service now? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo systemctl start sangi-pc-monitor.service
        echo "   ✅ Service started"
        echo ""
        echo "   Check status with: sudo systemctl status sangi-pc-monitor"
        echo "   View logs with: journalctl -u sangi-pc-monitor -f"
    fi
fi

# Create test script
echo ""
echo "🧪 Creating test script..."
cat > test_monitor.sh << 'EOF'
#!/bin/bash
# Test the workspace monitor with your current config

echo "🧪 Testing SANGI Workspace Monitor"
echo "=================================="
echo ""
echo "This will run the monitor for 30 seconds to verify setup."
echo "Watch for connection messages and emotion updates."
echo ""
read -p "Press Enter to start test..."

timeout 30s python3 workspace_monitor.py || true

echo ""
echo "✅ Test complete!"
echo "If you saw connection and activity messages, setup is working!"
EOF

chmod +x test_monitor.sh
echo "   Created test_monitor.sh"

echo ""
echo "✅ Setup complete!"
echo ""
echo "📝 Next steps:"
echo "   1. Edit config.json with your AWS IoT credentials"
echo "   2. Test with: ./test_monitor.sh"
echo "   3. Run manually: python3 workspace_monitor.py"
if [[ -f "$SERVICE_FILE" ]]; then
    echo "   4. Manage service:"
    echo "      - Status: sudo systemctl status sangi-pc-monitor"
    echo "      - Logs:   journalctl -u sangi-pc-monitor -f"
    echo "      - Stop:   sudo systemctl stop sangi-pc-monitor"
    echo "      - Start:  sudo systemctl start sangi-pc-monitor"
fi
echo ""
echo "🤖 Your SANGI will now reflect your PC activity in real-time!"
