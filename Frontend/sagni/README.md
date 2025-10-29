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

```bash
npm install --legacy-peer-deps
npm run dev
```

The app will start at `http://localhost:3000`

### Environment Variables

Create a `.env.local` file in the project root:

```bash
# AWS IoT Core WebSocket endpoint
# Get from: AWS Console → IoT Core → Settings → Device data endpoint
NEXT_PUBLIC_AWS_IOT_ENDPOINT=wss://xxxxxxxxxxxxx.iot.us-east-1.amazonaws.com/mqtt
NEXT_PUBLIC_AWS_REGION=us-east-1

# AWS Credentials (Optional - for SigV4 signing)
# For production, use AWS Cognito Identity Pool or temporary credentials
NEXT_PUBLIC_AWS_ACCESS_KEY_ID=
NEXT_PUBLIC_AWS_SECRET_ACCESS_KEY=
NEXT_PUBLIC_AWS_SESSION_TOKEN=

# MQTT Client Configuration
NEXT_PUBLIC_MQTT_CLIENT_PREFIX=sangi-web

# Mock Mode (set to 'false' for production with AWS IoT)
NEXT_PUBLIC_USE_MOCK_MQTT=true
```

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

To connect to a real SANGI device via AWS IoT Core:

#### Option 1: Certificate-Based Authentication (Recommended)

1. **AWS IoT Policy**: Ensure your SANGI device has an IoT policy allowing:
   ```json
   {
     "Version": "2012-10-17",
     "Statement": [{
       "Effect": "Allow",
       "Action": ["iot:Connect", "iot:Publish", "iot:Subscribe", "iot:Receive"],
       "Resource": ["arn:aws:iot:REGION:ACCOUNT:topic/sangi/*", "arn:aws:iot:REGION:ACCOUNT:client/sangi-*"]
     }]
   }
   ```

2. **Configure CORS on AWS IoT**: Enable WebSocket access from browser
   - AWS IoT automatically handles CORS for certificate-based connections

3. **Add environment variables** to `.env.local`:
   ```bash
   NEXT_PUBLIC_AWS_IOT_ENDPOINT=wss://xxxxxxxxxxxxx.iot.us-east-1.amazonaws.com/mqtt
   NEXT_PUBLIC_AWS_REGION=us-east-1
   NEXT_PUBLIC_USE_MOCK_MQTT=false
   ```

#### Option 2: SigV4 Authentication (AWS Credentials)

For temporary access using AWS credentials:

1. **Create Cognito Identity Pool**:
   - AWS Console → Cognito → Create Identity Pool
   - Enable unauthenticated access
   - Attach IAM role with IoT permissions

2. **Get temporary credentials** from Cognito and add to `.env.local`:
   ```bash
   NEXT_PUBLIC_AWS_ACCESS_KEY_ID=ASIA...
   NEXT_PUBLIC_AWS_SECRET_ACCESS_KEY=...
   NEXT_PUBLIC_AWS_SESSION_TOKEN=...
   ```

3. **Toggle to Real Mode** in the Settings panel

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

Two implementations for flexible deployment:

1. **MockMQTTClient** (`src/lib/mock-mqtt.ts`):
   - Simulates ESP32 publishing status updates
   - Perfect for development without real hardware
   - Includes realistic timing and data ranges

2. **RealMQTTClient** (`src/lib/mqtt-client.ts`):
   - Uses `mqtt.js` for WebSocket connections to AWS IoT Core
   - **Direct browser-to-AWS connection** (no server proxy)
   - Supports AWS SigV4 signing with `@smithy/signature-v4`
   - Handles reconnection with exponential backoff
   - **Vercel-compatible** (serverless-friendly architecture)

### Components

- **ConnectionStatus**: Real-time connection indicator
- **EmotionGrid**: 12 emotion buttons with visual feedback
- **EmotionButton**: Individual emotion with icon, label, and timestamp
- **SystemStats**: Battery, WiFi, uptime, and heap display
- **MessageLog**: Collapsible MQTT message history
- **PublishPanel**: Custom message publishing interface

## Deployment

### Vercel (Recommended for Serverless)

The app now connects **directly to AWS IoT Core via WebSocket**, making it fully compatible with Vercel's serverless architecture.

#### Prerequisites

1. **AWS IoT Core Setup**:
   - Device endpoint: `AWS Console → IoT Core → Settings`
   - IoT Policy with permissions for `sangi/*` topics
   - (Optional) Cognito Identity Pool for credential management

2. **Vercel Project Setup**:
   ```bash
   # Install Vercel CLI
   npm i -g vercel
   
   # Deploy from project directory
   cd Frontend/sagni
   vercel
   ```

3. **Configure Environment Variables** in Vercel Dashboard:
   - `NEXT_PUBLIC_AWS_IOT_ENDPOINT` → `wss://xxxxxxxxxxxxx.iot.us-east-1.amazonaws.com/mqtt`
   - `NEXT_PUBLIC_AWS_REGION` → `us-east-1`
   - `NEXT_PUBLIC_MQTT_CLIENT_PREFIX` → `sangi-web`
   - `NEXT_PUBLIC_USE_MOCK_MQTT` → `false`

4. **Optional: Add AWS Credentials** (for SigV4 signing):
   - `NEXT_PUBLIC_AWS_ACCESS_KEY_ID`
   - `NEXT_PUBLIC_AWS_SECRET_ACCESS_KEY`
   - `NEXT_PUBLIC_AWS_SESSION_TOKEN`

   **Note**: For production, use Cognito Identity Pool to fetch temporary credentials client-side instead of hardcoding.

5. **Deploy**:
   ```bash
   vercel --prod
   ```

#### How It Works on Vercel

- **Direct WebSocket Connection**: Browser → AWS IoT Core (no server proxy needed)
- **Serverless-Compatible**: No long-lived connections on Vercel functions
- **Real-time MQTT**: Persistent WebSocket maintained by browser, not server
- **AWS SigV4 Signing**: Optional authentication using Smithy SDK

### Self-Hosted

For traditional server deployment:

```bash
npm run build
npm run start
```

Serve with PM2, Docker, or any Node.js hosting platform.

### Docker

```dockerfile
FROM node:20-alpine
WORKDIR /app
COPY package*.json ./
RUN npm install --legacy-peer-deps
COPY . .
RUN npm run build
EXPOSE 3000
CMD ["npm", "start"]
```

```bash
docker build -t sangi-frontend .
docker run -p 3000:3000 --env-file .env.local sangi-frontend
```

## Troubleshooting

### Connection Issues

- **"Connection failed"**: Check AWS IoT endpoint URL format (`wss://xxxxx.iot.region.amazonaws.com/mqtt`)
- **"WebSocket error"**: Verify IoT policy allows `iot:Connect` for your client ID pattern
- **"Cannot subscribe to topic"**: Verify IAM policy allows `iot:Subscribe` on `sangi/*`
- **"Unauthorized"**: Check AWS credentials or Cognito Identity Pool configuration
- **Certificate errors**: AWS IoT certificate-based auth requires proper CORS setup
- **Mock mode not working**: Ensure `NEXT_PUBLIC_USE_MOCK_MQTT=true` in `.env.local`

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
