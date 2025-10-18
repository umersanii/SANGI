#!/bin/bash
# Install dependencies for enhanced workspace monitor

echo "📦 Installing Python dependencies for SANGI Workspace Monitor..."

# Check if pip is available
if ! command -v pip3 &> /dev/null; then
    echo "❌ pip3 not found. Please install Python 3 and pip first."
    exit 1
fi

# Install required packages
echo "Installing required packages..."
pip3 install --user \
    AWSIoTPythonSDK \
    psutil \
    watchdog \
    pynput

echo ""
echo "✅ Installation complete!"
echo ""
echo "📝 Note: If you get permission errors with pynput input monitoring,"
echo "   you may need to add your user to the 'input' group:"
echo "   sudo usermod -a -G input $USER"
echo "   (then log out and back in)"
echo ""
echo "🚀 You can now run: python3 workspace_monitor.py"
