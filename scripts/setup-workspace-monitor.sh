#!/bin/bash
# SANGI Workspace Monitor Setup Script
# This script sets up the workspace activity monitor on PC or Raspberry Pi

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}"
echo "╔════════════════════════════════════════╗"
echo "║  SANGI Workspace Monitor Setup         ║"
echo "╚════════════════════════════════════════╝"
echo -e "${NC}"

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
   echo -e "${RED}✗ Don't run this script as root${NC}"
   exit 1
fi

# Detect device type
read -p "Is this a PC or Raspberry Pi? (pc/pi): " DEVICE_TYPE
DEVICE_TYPE=$(echo "$DEVICE_TYPE" | tr '[:upper:]' '[:lower:]')

if [[ "$DEVICE_TYPE" != "pc" && "$DEVICE_TYPE" != "pi" ]]; then
    echo -e "${RED}✗ Invalid device type. Use 'pc' or 'pi'${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Device type: $DEVICE_TYPE${NC}"

# Create working directory
INSTALL_DIR="$HOME/sangi-workspace-monitor"
echo -e "\n${BLUE}Creating installation directory: $INSTALL_DIR${NC}"
mkdir -p "$INSTALL_DIR"

# Copy Python script
SCRIPT_SOURCE="$(dirname "$0")/sangi-workspace-monitor.py"
if [ ! -f "$SCRIPT_SOURCE" ]; then
    echo -e "${RED}✗ Monitor script not found: $SCRIPT_SOURCE${NC}"
    exit 1
fi

cp "$SCRIPT_SOURCE" "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/sangi-workspace-monitor.py"
echo -e "${GREEN}✓ Copied monitor script${NC}"

# Install Python dependencies
echo -e "\n${BLUE}Installing Python dependencies...${NC}"
pip3 install --user psutil paho-mqtt pytz || {
    echo -e "${YELLOW}⚠ Installing with sudo...${NC}"
    sudo pip3 install psutil paho-mqtt pytz
}

# Install Xlib for input tracking (Linux only)
if [ "$DEVICE_TYPE" == "pc" ] || [ "$DEVICE_TYPE" == "pi" ]; then
    echo -e "\n${BLUE}Installing X11 input tracking...${NC}"
    pip3 install --user python-xlib || {
        sudo apt-get install -y python3-xlib || echo -e "${YELLOW}⚠ Could not install python-xlib. Input tracking will be limited.${NC}"
    }
fi

# Install xdotool for window detection
echo -e "\n${BLUE}Installing xdotool...${NC}"
sudo apt-get install -y xdotool || echo -e "${YELLOW}⚠ Could not install xdotool. Window detection will be limited.${NC}"

# Setup certificates
echo -e "\n${BLUE}Certificate Setup${NC}"
echo -e "You need AWS IoT Core certificates from your SANGI project."
echo -e "Copy them to: ${GREEN}$INSTALL_DIR${NC}"
echo ""
echo -e "Required files:"
echo -e "  - AmazonRootCA1.pem"
echo -e "  - SANGI.cert.pem"
echo -e "  - SANGI.private.key"
echo ""

read -p "Have you copied the certificates? (y/n): " CERTS_READY
if [[ "$CERTS_READY" != "y" ]]; then
    echo -e "${YELLOW}⚠ Copy certificates and re-run this script${NC}"
    exit 0
fi

# Check certificates exist
if [ ! -f "$INSTALL_DIR/AmazonRootCA1.pem" ] || \
   [ ! -f "$INSTALL_DIR/SANGI.cert.pem" ] || \
   [ ! -f "$INSTALL_DIR/SANGI.private.key" ]; then
    echo -e "${RED}✗ Missing certificate files in $INSTALL_DIR${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Certificates found${NC}"

# Configure AWS endpoint
echo -e "\n${BLUE}AWS IoT Configuration${NC}"
read -p "Enter your AWS IoT endpoint (e.g., xxxxx-ats.iot.us-east-1.amazonaws.com): " AWS_ENDPOINT

# Update Python script with configuration
sed -i "s|DEVICE_NAME = \"pc\"|DEVICE_NAME = \"$DEVICE_TYPE\"|g" "$INSTALL_DIR/sangi-workspace-monitor.py"
sed -i "s|AWS_IOT_ENDPOINT = \".*\"|AWS_IOT_ENDPOINT = \"$AWS_ENDPOINT\"|g" "$INSTALL_DIR/sangi-workspace-monitor.py"
sed -i "s|CERT_DIR = \".*\"|CERT_DIR = \"$INSTALL_DIR\"|g" "$INSTALL_DIR/sangi-workspace-monitor.py"

echo -e "${GREEN}✓ Configuration updated${NC}"

# Test the monitor
echo -e "\n${BLUE}Testing monitor (5 seconds)...${NC}"
timeout 5 python3 "$INSTALL_DIR/sangi-workspace-monitor.py" --debug || {
    echo -e "${YELLOW}⚠ Test failed. Check configuration and try again.${NC}"
    exit 1
}

echo -e "${GREEN}✓ Monitor test successful${NC}"

# Setup systemd service
echo -e "\n${BLUE}Setting up systemd service...${NC}"
SERVICE_FILE="$(dirname "$0")/sangi-workspace-monitor@.service"

if [ -f "$SERVICE_FILE" ]; then
    # Update service file with device type
    SERVICE_TEMP="/tmp/sangi-workspace-monitor-$DEVICE_TYPE.service"
    cp "$SERVICE_FILE" "$SERVICE_TEMP"
    sed -i "s|--device pc|--device $DEVICE_TYPE|g" "$SERVICE_TEMP"
    
    sudo cp "$SERVICE_TEMP" "/etc/systemd/system/sangi-workspace-monitor-$DEVICE_TYPE.service"
    sudo systemctl daemon-reload
    
    echo -e "${GREEN}✓ Service file installed${NC}"
    
    read -p "Enable service to start on boot? (y/n): " ENABLE_SERVICE
    if [[ "$ENABLE_SERVICE" == "y" ]]; then
        sudo systemctl enable "sangi-workspace-monitor-$DEVICE_TYPE.service"
        sudo systemctl start "sangi-workspace-monitor-$DEVICE_TYPE.service"
        echo -e "${GREEN}✓ Service enabled and started${NC}"
        
        # Show status
        echo -e "\n${BLUE}Service Status:${NC}"
        sudo systemctl status "sangi-workspace-monitor-$DEVICE_TYPE.service" --no-pager
    fi
else
    echo -e "${YELLOW}⚠ Service file not found. Skipping systemd setup.${NC}"
fi

# Final instructions
echo -e "\n${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║  Setup Complete!                       ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
echo ""
echo -e "${BLUE}Installation Directory:${NC} $INSTALL_DIR"
echo -e "${BLUE}Device Type:${NC} $DEVICE_TYPE"
echo -e "${BLUE}AWS Endpoint:${NC} $AWS_ENDPOINT"
echo ""
echo -e "${BLUE}Manual Commands:${NC}"
echo -e "  Test monitor:     python3 $INSTALL_DIR/sangi-workspace-monitor.py --debug"
echo -e "  Start service:    sudo systemctl start sangi-workspace-monitor-$DEVICE_TYPE"
echo -e "  Stop service:     sudo systemctl stop sangi-workspace-monitor-$DEVICE_TYPE"
echo -e "  View logs:        sudo journalctl -u sangi-workspace-monitor-$DEVICE_TYPE -f"
echo ""
echo -e "${BLUE}Next Steps:${NC}"
echo -e "1. Upload updated firmware to SANGI robot"
echo -e "2. Monitor AWS IoT Core MQTT test client for 'workspace/$DEVICE_TYPE/activity'"
echo -e "3. Watch SANGI's emotions change based on your activity!"
echo ""
