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
