# SANGI Control Interface

A production-ready web control panel for SANGI, an ESP32-C3 companion robot with emotion states, MQTT communication, and AWS IoT integration.

## Features

- **Real-time MQTT Dashboard**: Connect to AWS IoT Core via WebSocket
- **Emotion Control**: 12 emotion buttons with visual feedback and timestamps
- **System Monitoring**: Battery level, WiFi signal strength, uptime, and heap memory
- **Sound Control**: Publish sound commands with volume control
- **Message Log**: View all MQTT publish/subscribe activity
- **Mock Mode**: Development mode with simulated data
- **Dark Mode**: Glassmorphism design with backdrop blur effects

## Quick Start

### Installation

\`\`\`bash
npm install
npm run dev
\`\`\`

The app will start at `http://localhost:5173`

### Environment Variables

Create a `.env.local` file in the project root:

\`\`\`bash
# AWS IoT Core WebSocket endpoint
VITE_MQTT_ENDPOINT=wss://xxxxx-ats.iot.us-east-1.amazonaws.com/mqtt

# AWS Region
VITE_AWS_REGION=us-east-1

# Cognito Identity Pool ID (for authentication)
VITE_COGNITO_IDENTITY_POOL_ID=us-east-1:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

# MQTT Client ID (auto-generated if not set)
VITE_MQTT_CLIENT_ID=sangi-web-client

# Enable mock mode for development (default: true)
VITE_MOCK_MODE=true
\`\`\`

## Development

### Mock Mode (Default)

The app starts in **Mock Mode** by default, simulating SANGI device data without requiring AWS IoT Core setup. This is perfect for UI development and testing.

**Mock data includes:**
- Battery status updates every 5 seconds
- System status (uptime, heap, WiFi signal)
- Random emotion changes
- Realistic MQTT message flow

Toggle between Mock and Real modes in the Settings panel (gear icon).

### Real MQTT Mode

To connect to a real SANGI device:

1. **Set up AWS IoT Core** (if not already done):
   - Create an IoT Thing for your ESP32-C3
   - Generate certificates and attach policies
   - Note the WebSocket endpoint

2. **Configure Cognito Identity Pool**:
   - Create an Identity Pool in AWS Cognito
   - Enable unauthenticated access
   - Attach an IoT policy allowing `iot:Connect`, `iot:Subscribe`, `iot:Receive`, `iot:Publish` on `sangi/*` topics

3. **Add environment variables** to `.env.local`:
   \`\`\`bash
   VITE_MQTT_ENDPOINT=wss://your-endpoint-ats.iot.us-east-1.amazonaws.com/mqtt
   VITE_COGNITO_IDENTITY_POOL_ID=us-east-1:your-pool-id
   VITE_MOCK_MODE=false
   \`\`\`

4. **Toggle to Real Mode** in the Settings panel

## MQTT Topics

### Published by SANGI Device (Subscribe)

\`\`\`
sangi/status/battery     → {"level": 85, "voltage": 4.15, "charging": false}
sangi/status/emotion     → {"current": "HAPPY", "timestamp": 1729512345}
sangi/status/system      → {"uptime": 3600, "heap": 180000, "rssi": -45}
\`\`\`

### Published by Web Interface (Subscribe on Device)

\`\`\`
sangi/command/emotion    → "HAPPY" (plain text, uppercase)
sangi/command/sound      → {"type": "BEEP", "volume": 75, "duration": 200}
\`\`\`

## Emotion States

The interface supports 12 emotion states:

- **HAPPY** 😊
- **SAD** 😢
- **ANGRY** 😠
- **SURPRISED** 😲
- **SLEEPY** 😴
- **CONFUSED** 😕
- **LOVING** 😍
- **EXCITED** 🤩
- **BORED** 😑
- **CURIOUS** 🤔
- **SCARED** 😨
- **PROUD** 😌

Click any emotion button to send a command to the device. The button highlights when that emotion is active on the device.

## Architecture

### State Management

Uses **Zustand** for global MQTT state management:
- Connection status and error handling
- Message history (last 50 messages)
- Device status (battery, system, emotion)
- Last triggered emotion with timestamp

### MQTT Client

Two implementations:

1. **MockMQTTClient** (`src/lib/mock-mqtt.ts`):
   - Simulates ESP32 publishing status updates
   - Perfect for development without real hardware
   - Includes realistic timing and data ranges

2. **RealMQTTClient** (`src/lib/mqtt-client.ts`):
   - Uses `mqtt.js` for WebSocket connections
   - Connects to AWS IoT Core
   - Handles reconnection with exponential backoff

### Components

- **ConnectionStatus**: Real-time connection indicator
- **EmotionGrid**: 12 emotion buttons with visual feedback
- **EmotionButton**: Individual emotion with icon, label, and timestamp
- **SystemStats**: Battery, WiFi, uptime, and heap display
- **MessageLog**: Collapsible MQTT message history
- **PublishPanel**: Custom message publishing interface

## Deployment

### Vercel

1. Push code to GitHub
2. Connect repository to Vercel
3. Add environment variables in Vercel dashboard:
   - `VITE_MQTT_ENDPOINT`
   - `VITE_COGNITO_IDENTITY_POOL_ID`
   - `VITE_AWS_REGION`
4. Deploy

### Self-Hosted

\`\`\`bash
npm run build
npm run preview
\`\`\`

Serve the `dist/` folder with any static hosting service.

## Troubleshooting

### Connection Issues

- **"Connection failed"**: Check MQTT endpoint and Cognito credentials
- **"Cannot subscribe to topic"**: Verify IAM policy allows `iot:Subscribe` on `sangi/*`
- **Mock mode not working**: Ensure `VITE_MOCK_MODE=true` in `.env.local`

### Message Not Received

- Check that device is subscribed to the correct topic
- Verify QoS setting (0 = fire and forget, 1 = at least once)
- Check MQTT message log for publish confirmation

### Battery/System Stats Not Updating

- In Mock Mode: Stats update every 5 seconds automatically
- In Real Mode: Ensure device is publishing to `sangi/status/*` topics
- Check message log to see incoming messages

## Development Notes

### Adding New Emotions

1. Add emotion to `EmotionState` type in `src/types/sangi.d.ts`
2. Add emoji to `EMOTION_ICONS` in `src/components/emotion/emotion-button.tsx`
3. Add to `EMOTIONS` array in `src/components/emotion/emotion-grid.tsx`

### Adding New MQTT Topics

1. Define message type in `src/types/sangi.d.ts`
2. Add subscription in `useMQTTStore.connect()` method
3. Create component to display data
4. Add to main layout

### Customizing Styling

All colors use Tailwind CSS with a dark slate theme. Modify colors in:
- `src/app.tsx` (main layout)
- Individual component files
- `src/globals.css` (if using custom CSS)

## License

MIT

## Support

For issues or questions about SANGI, visit the project repository or contact the development team.
