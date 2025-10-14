# MQTT Quick Start

## 1. Enable MQTT Mode

Edit `include/config.h`:
```cpp
#define ENABLE_MQTT true
```

## 2. Configure Credentials

Copy template and edit:
```bash
cp include/secrets.h.template include/secrets.h
```

Fill in:
- WiFi SSID and password
- AWS IoT endpoint (from AWS Console Settings)
- Device certificates (from AWS IoT Thing creation)

## 3. Build and Upload

```bash
pio run --target upload
pio device monitor
```

## 4. Test Emotion Control

### Via AWS IoT Console

**Topic**: `sangi/emotion/set`

**Payload**:
```json
{"emotion": 1}
```

### Emotion Values
| Value | Emotion |
|-------|---------|
| 0 | IDLE |
| 1 | HAPPY |
| 2 | SLEEPY |
| 3 | EXCITED |
| 4 | SAD |
| 5 | ANGRY |
| 6 | CONFUSED |
| 7 | THINKING |
| 8 | LOVE |
| 9 | SURPRISED |
| 10 | DEAD |
| 12 | MUSIC |

## 5. Monitor Status

Subscribe to: `sangi/#`

You'll receive:
- `sangi/battery` - Voltage and percentage
- `sangi/emotion/current` - Current emotion
- `sangi/status` - Connection events

## Troubleshooting

**WiFi won't connect**: Check SSID/password, use 2.4GHz network

**MQTT fails**: Verify endpoint URL and certificates are activated in AWS

**See full guide**: [MQTT_SETUP.md](MQTT_SETUP.md)
