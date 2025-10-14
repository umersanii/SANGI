# MQTT Setup Guide for SANGI Robot

## Overview

SANGI now supports MQTT communication via AWS IoT Core for remote emotion control and status monitoring. This enables you to control the robot's emotional expressions and receive telemetry data through the cloud.

## Features

- **Remote Emotion Control**: Change SANGI's emotion via MQTT messages
- **Telemetry Publishing**: Receive battery voltage, uptime, and current emotion state
- **Automatic Fallback**: Continues autonomous operation when network unavailable
- **Secure Communication**: Uses AWS IoT Core with certificate-based authentication

## MQTT Topics

### Subscribe Topics (SANGI receives)
- `sangi/emotion/set` - Set emotion state remotely

### Publish Topics (SANGI sends)
- `sangi/status` - General status updates
- `sangi/battery` - Battery voltage and percentage
- `sangi/uptime` - Device uptime
- `sangi/emotion/current` - Current emotion state

## AWS IoT Core Setup

### 1. Create AWS IoT Thing

1. Log in to [AWS Console](https://console.aws.amazon.com/)
2. Navigate to **IoT Core** service
3. Go to **Manage** → **Things** → **Create things**
4. Select **Create single thing**
5. Name it `SANGI_Robot` (or match the `THINGNAME` in your config)
6. Click **Next**

### 2. Generate Certificates

1. Choose **Auto-generate a new certificate**
2. Download the following files:
   - **Device certificate** (xxxxx-certificate.pem.crt)
   - **Private key** (xxxxx-private.pem.key)
   - **Amazon Root CA 1** (Download from [here](https://www.amazontrust.com/repository/AmazonRootCA1.pem))
3. Click **Activate** to enable the certificate
4. Click **Attach policies** (we'll create the policy next)

### 3. Create IoT Policy

1. Go to **Secure** → **Policies** → **Create policy**
2. Name it `SANGI_Policy`
3. Add the following policy document:

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:Connect",
      "Resource": "arn:aws:iot:REGION:ACCOUNT_ID:client/SANGI_Robot"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Subscribe",
      "Resource": "arn:aws:iot:REGION:ACCOUNT_ID:topicfilter/sangi/*"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Receive",
      "Resource": "arn:aws:iot:REGION:ACCOUNT_ID:topic/sangi/*"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Publish",
      "Resource": "arn:aws:iot:REGION:ACCOUNT_ID:topic/sangi/*"
    }
  ]
}
```

**Replace**:
- `REGION` with your AWS region (e.g., `us-east-1`)
- `ACCOUNT_ID` with your AWS account ID

4. Click **Create**
5. Go back to **Secure** → **Certificates**
6. Find your certificate and click **Attach policy**
7. Select `SANGI_Policy` and attach

### 4. Get Your AWS IoT Endpoint

1. Go to **Settings** in AWS IoT Core
2. Copy the **Endpoint** URL (format: `xxxxx-ats.iot.region.amazonaws.com`)
3. Save this for the next step

## Code Configuration

### 1. Create secrets.h File

1. Copy the template file:
   ```bash
   cp include/secrets.h.template include/secrets.h
   ```

2. Edit `include/secrets.h` with your credentials:

```cpp
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"

#define AWS_IOT_ENDPOINT "xxxxx-ats.iot.region.amazonaws.com"
#define THINGNAME "SANGI_Robot"
```

3. Paste your certificates (open the `.pem` files in a text editor):

```cpp
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
[Paste Amazon Root CA 1 certificate content here]
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
[Paste device certificate content here]
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
[Paste private key content here]
-----END RSA PRIVATE KEY-----
)EOF";
```

### 2. Enable MQTT in config.h

Edit `include/config.h`:

```cpp
#define ENABLE_MQTT true  // Change from false to true
```

### 3. Adjust Timezone (Optional)

In `include/config.h`, set your timezone:

```cpp
#define GMT_OFFSET_SEC -28800      // PST = -8 hours
#define DAYLIGHT_OFFSET_SEC 3600   // DST = +1 hour
```

Common timezones:
- **UTC**: `0`
- **EST**: `-18000` (GMT-5)
- **PST**: `-28800` (GMT-8)
- **CET**: `3600` (GMT+1)

## Building and Uploading

1. **Install dependencies** (PlatformIO will auto-download):
   ```bash
   pio lib install
   ```

2. **Build the project**:
   ```bash
   pio run
   ```

3. **Upload to ESP32**:
   ```bash
   pio run --target upload
   ```

4. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

## Testing MQTT Communication

### 1. Monitor Serial Output

After uploading, watch the serial monitor for:

```
=== Network Manager Initializing ===
Connecting to WiFi: YourWiFiName
.....
WiFi connected! IP: 192.168.1.xxx
Synchronizing time with NTP...
Time synchronized: Mon Oct 14 12:34:56 2025
Connecting to AWS IoT Core as: SANGI_Robot
MQTT connected!
Subscribed to: sangi/emotion/set
Published status: connected
```

### 2. Send Emotion Commands via AWS Console

1. Go to **AWS IoT Core** → **Test** → **MQTT test client**
2. **Subscribe** to `sangi/#` to see all messages from SANGI
3. **Publish** to `sangi/emotion/set` with this payload:

```json
{
  "emotion": 1
}
```

**Emotion Values**:
- `0` - EMOTION_IDLE
- `1` - EMOTION_HAPPY
- `2` - EMOTION_SLEEPY
- `3` - EMOTION_EXCITED
- `4` - EMOTION_SAD
- `5` - EMOTION_ANGRY
- `6` - EMOTION_CONFUSED
- `7` - EMOTION_THINKING
- `8` - EMOTION_LOVE
- `9` - EMOTION_SURPRISED
- `10` - EMOTION_DEAD
- `12` - EMOTION_MUSIC

### 3. Monitor Published Data

You should see messages published to:
- `sangi/battery` (every 30 seconds)
- `sangi/emotion/current` (every 30 seconds)
- `sangi/status` (on connection/events)

Example battery message:
```json
{
  "voltage": 3.75,
  "percentage": 62.5,
  "timestamp": 123456
}
```

### 4. Using Python Script for Testing

Create a test script `test_mqtt.py`:

```python
import paho.mqtt.client as mqtt
import json
import ssl

# Configure your AWS IoT endpoint
IOT_ENDPOINT = "xxxxx-ats.iot.region.amazonaws.com"
CERT_PATH = "./certs/device-certificate.pem.crt"
KEY_PATH = "./certs/device-private.pem.key"
ROOT_CA = "./certs/AmazonRootCA1.pem"

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe("sangi/#")

def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}")
    print(f"Payload: {msg.payload.decode()}\n")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.tls_set(ca_certs=ROOT_CA, certfile=CERT_PATH, keyfile=KEY_PATH,
               tls_version=ssl.PROTOCOL_TLSv1_2)

client.connect(IOT_ENDPOINT, 8883, 60)

# Send emotion command
emotion_msg = json.dumps({"emotion": 1})  # HAPPY
client.publish("sangi/emotion/set", emotion_msg)

client.loop_forever()
```

## Troubleshooting

### WiFi Connection Issues

**Problem**: `WiFi connection timeout`

**Solutions**:
- Verify SSID and password in `secrets.h`
- Check WiFi signal strength
- Ensure ESP32 is in range of router
- Try 2.4GHz network (ESP32-C3 doesn't support 5GHz)

### MQTT Connection Failures

**Problem**: `MQTT connection failed, rc=-2`

**Solutions**:
- Verify endpoint URL in `secrets.h`
- Check certificate/key formatting (no extra spaces/lines)
- Ensure certificates are activated in AWS Console
- Verify policy is attached to certificate

**Problem**: `MQTT connection failed, rc=-4`

**Solutions**:
- Time synchronization failed
- Wait for NTP sync (up to 15 seconds)
- Check internet connectivity

### Certificate Issues

**Problem**: `SSL handshake failed`

**Solutions**:
- Ensure you downloaded **Amazon Root CA 1** (not CA 3)
- Check certificate PEM format includes BEGIN/END markers
- Verify no extra characters or line breaks in certificates
- Make sure private key matches certificate

### Autonomous Fallback

If MQTT is enabled but connection fails, SANGI will:
1. Attempt reconnection every 5 seconds
2. Fall back to autonomous emotion cycling
3. Continue normal display operation
4. Log status to serial monitor

## Performance Notes

- **Power consumption**: MQTT adds ~30-50mA to baseline consumption
- **Memory**: ~15KB additional RAM usage for network stack
- **Latency**: Emotion commands execute within 50-100ms of reception
- **Reconnection**: Automatic with exponential backoff (5s intervals)

## Security Best Practices

1. **Never commit secrets.h to version control**
   - Add to `.gitignore`
   - Use environment variables for CI/CD

2. **Rotate certificates periodically**
   - AWS IoT allows multiple active certificates
   - Update and test before deactivating old ones

3. **Use least-privilege IAM policies**
   - Limit topics to `sangi/*` prefix only
   - Don't use wildcard (`*`) in production

4. **Monitor AWS CloudWatch**
   - Track connection attempts
   - Set alarms for authentication failures

## Advanced Configuration

### Custom Topics

Edit `include/config.h`:

```cpp
#define MQTT_TOPIC_EMOTION_SET "custom/emotion/control"
#define MQTT_TOPIC_STATUS "custom/status"
```

### Publishing Interval

Change status update frequency:

```cpp
#define STATUS_PUBLISH_INTERVAL 60000  // 60 seconds
```

### Network Timeout

Adjust connection timeout:

```cpp
#define NETWORK_TIMEOUT 20000  // 20 seconds
```

## Integration Examples

### Node-RED Flow

```json
[
  {
    "id": "mqtt_in",
    "type": "mqtt in",
    "topic": "sangi/battery",
    "broker": "aws_iot_broker"
  },
  {
    "id": "emotion_trigger",
    "type": "inject",
    "payload": "{\"emotion\": 1}",
    "topic": "sangi/emotion/set"
  }
]
```

### Home Assistant

```yaml
mqtt:
  sensor:
    - name: "SANGI Battery"
      state_topic: "sangi/battery"
      value_template: "{{ value_json.voltage }}"
      unit_of_measurement: "V"
  
  button:
    - name: "SANGI Happy"
      command_topic: "sangi/emotion/set"
      payload: '{"emotion": 1}'
```

## Support

For issues or questions:
- Check serial output for error codes
- Review AWS CloudWatch logs
- Verify certificate expiration dates
- Test with AWS IoT MQTT test client first

## License

MQTT integration maintains the same MIT License as the main project.
