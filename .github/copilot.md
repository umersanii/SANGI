# SANGI Robot: Technical Specification & Development Context

## Project Overview

**Hardware Platform:** ESP32-C3 microcontroller with OLED display  
**Project Goal:** Companion robot with expressive animations and workspace integration  
**Development Philosophy:** Modular, personality-first design with progressive feature integration

---

## Core Design Principles

- **Personality without complexity:** Simple state machines create believable emotional responses
- **Autonomous baseline:** Device functions meaningfully without external dependencies
- **Progressive integration:** Each feature tier is independently complete
- **Low-power awareness:** Battery life considerations influence animation and communication strategies

---

## 🧩 Feature Tier 1: Standalone "Pocket Pet" Mode

### Implementation Requirements

**Core emotion engine:**
- State machine: idle → sleepy → happy → bored → excited
- Trigger sources: RTC time, battery voltage reading, button/capacitive touch events
- Animation system: Frame-based transitions (2-3 frame minimum for smooth state changes)

**Input mechanisms:**
- Physical button or capacitive touch → immediate reaction animations
- Ambient light sensor (optional) → context-aware responses (pickup detection, cover detection)

**Display feedback:**
- Battery status mapped to facial expressions (voltage thresholds → emotion states)
- Daily mood randomization on boot (PRNG-seeded personality variation)
- Optional buzzer/piezo for auditory feedback

**Example behavior mapping:**
```
06:00-12:00 → energetic/happy expressions
12:00-18:00 → calm/focused expressions  
18:00-24:00 → sleepy/"goodnight" expressions
<3.3V battery → forced sleepy state
```

**Hardware considerations:**
- Li-ion battery via USB-C charging
- Deep sleep mode implementation for idle periods
- Wake-on-interrupt for button/sensor events

**Animation implementation note:**
Avoid instant state switches. Implement transition frames (blink-to-sleepy, excited-to-calm) for organic feel. 2-3 frame transitions significantly improve perceived responsiveness and personality.

---

## 💻 Feature Tier 2: Workspace Integration

### Communication Architecture

**Supported protocols:**
- **Serial (USB):** Direct connection, useful for initial debugging/prototyping
- **Wi-Fi (HTTP REST / WebSocket / MQTT):** Primary wireless integration method
- **Bluetooth LE:** Alternative for mobile app control (limited bandwidth)

**Recommended approach based on use case:**
- **MQTT:** Best for pub/sub model, scales well with multiple devices
- **HTTP REST:** Simplest for one-way Pi → SANGI commands
- **Serial:** Development/testing before wireless implementation

### Integration Patterns

#### 1. Time-Based Emotional Cycles
**Data source:** System time from Pi/laptop  
**Update frequency:** Every 60 seconds (balance between responsiveness and power)  
**Implementation:** Simple HTTP GET or MQTT publish with timestamp payload

#### 2. Development Environment Awareness
**Trigger events:**
- Process monitoring → VS Code launch detection → "focused" face
- Idle detection (no keyboard/mouse input threshold) → "bored" face
- Build system exit codes → success (excited) vs. failure (angry/confused)
- Git commit events → satisfaction animation

**Implementation considerations:**
- VS Code integration: Monitor active window process OR use custom VS Code extension with webhook
- Build monitoring: Parse compiler output or monitor build script exit codes
- Process detection may require platform-specific implementation (Linux proc vs macOS ps)

#### 3. System Metrics Visualization
**Data sources:**
- CPU usage percentage → expression intensity mapping
- Memory usage → secondary indicator (color shift, blink rate)
- Temperature (especially Pi) → progressive "stress" expressions
- Network activity → brief attention animations

**Expression mapping example:**
```
CPU < 30%: calm/idle
CPU 30-60%: focused
CPU 60-80%: concerned
CPU > 80%: stressed/angry
```

**Why this works:** Provides ambient awareness of system health without requiring active monitoring. Temperature-based reactions are both functional (alert mechanism) and entertaining.

#### 4. Notification & Event System
**Event types:**
- New message/email → surprised expression
- Calendar reminder → thinking/alert face
- Timer/alarm → attention-getting animation
- Break reminder → relaxed/sleepy suggestion

#### 5. Focus Mode / Pomodoro Integration
**State mapping:**
- Focus period start → concentrated expression
- Break period start → happy/relaxed expression
- Session complete → celebration animation

**Practical value:** Turns SANGI into functional peripheral for productivity tracking, not just decorative element.

### Communication Implementation

**ESP32-C3 side:**
- HTTP server listening on local network
- Endpoint structure: `/api/emotion/{state}` or `/api/system/{metric}`
- JSON payload parsing for complex state data
- Fallback to standalone mode if no messages received within timeout

**Pi/Laptop side (Python reference):**
```python
# Pseudo-code structure
import requests
import psutil

def update_sangi(emotion, intensity=1.0):
    requests.post('http://sangi.local/api/emotion', 
                  json={'state': emotion, 'intensity': intensity})

# Monitoring loop
while True:
    cpu_temp = psutil.sensors_temperatures()['cpu_thermal'][0].current
    if cpu_temp > 70:
        update_sangi('stressed', intensity=0.8)
    # ... other checks
    time.sleep(60)
```

---

## 🌐 Feature Tier 3: IoT Companion Mode

### Integration Points

**Existing infrastructure:**
- Tailscale network connectivity
- Existing Pi monitoring application (leverage existing data sources)
- Local network services status

**Communication patterns:**
- MQTT broker on Pi for pub/sub event distribution
- REST API endpoints for direct queries
- WebSocket for real-time bidirectional communication

**Use cases:**
- Pi status display (temperature, uptime, service health) via expressions
- Network-connected status indicator sitting adjacent to Pi
- Alert visualization for monitoring app events
- Future voice assistant integration (Rhasspy) → visual feedback layer

**Voice assistant integration concept:**
SANGI displays "listening" animation during wake word detection, then reacts based on command result (success/failure/thinking states). Provides visual feedback layer for headless voice assistant setup.

---

## 🚀 Expansion Capabilities

### Hardware Extensions
- **Servo motors:** Head tilt mechanism (2 DOF), ear movement (1-2 servos)
- **Microphone:** Sound level detection, clap pattern recognition, voice activity detection (not recognition)
- **Speaker/Buzzer:** Emotional sound generation (beeps, chirps, sighs mapped to states)
- **BLE mobile app:** Remote expression control, configuration, battery monitoring

### Software Extensions
- Advanced animation interpolation (ease-in/ease-out curves)
- Multiple personality profiles (user-selectable behavior patterns)
- Learning mode (frequency tracking of user interactions → preference adaptation)
- Animation scripting language for custom sequences

---

## 💭 Reference System Architecture

### Complete Integration Scenario

**Network topology:**
```
[Laptop] ←→ Tailscale ←→ [Raspberry Pi]
                              ↓
                         Local WiFi
                              ↓
                          [SANGI]
```

**Data flow:**
1. Python monitoring script on Pi collects metrics (CPU, time, events)
2. Script publishes to MQTT topic OR sends HTTP POST every 60s
3. SANGI subscribes/polls for updates
4. Expression engine translates data → emotion state
5. Animation controller renders appropriate face
6. When disconnected from network → automatic fallback to standalone pet mode

**Behavior characteristics:**
- Morning (06:00): Cheerful boot animation
- Work hours: Reflects laptop/Pi activity status
- Build failure: Immediate reactive animation
- System overheating: Progressive stress indicators
- Evening (22:00): Gradual transition to sleepy states
- Portable mode: Time-based emotions only, no network dependency

**Key insight:** Continuity of character across contexts. SANGI maintains personality whether networked or standalone, creating consistent interaction model.

---

## 🎯 Development Implementation Order

### Phase 1: Foundation (Standalone Functionality)
1. Animation engine with frame-based transitions
2. Time-based emotion state machine
3. Button/touch interaction handling
4. Battery level display integration
5. Deep sleep power management

**Completion criteria:** SANGI operates independently with engaging personality

### Phase 2: Network Layer
1. Wi-Fi connection management with fallback
2. HTTP server implementation on ESP32-C3
3. Basic command reception (/api/emotion endpoint)
4. Connection status indication
5. Automatic standalone fallback on timeout

**Completion criteria:** SANGI accepts remote commands while maintaining standalone capability

### Phase 3: Pi Integration
1. Python monitoring script (system metrics collection)
2. Time-synchronized emotion updates
3. Event-based triggers (notifications, alerts)
4. MQTT broker setup (if chosen) OR REST client implementation

**Completion criteria:** SANGI reflects Pi status accurately in real-time

### Phase 4: Advanced Features
1. Laptop activity monitoring integration
2. Development environment hooks
3. Pomodoro timer synchronization
4. Sound output implementation
5. Mobile app control (optional)

**Completion criteria:** Full workspace integration with practical utility

### Phase 5: Polish & Extensions
1. Animation library expansion
2. Personality profiles
3. Servo motor integration (physical movement)
4. Voice assistant visual feedback layer

**Completion criteria:** Production-ready companion robot with extensible architecture

---

## 🔋 Power Budget Considerations

**Battery life optimization strategies:**

- **Display management:** OLED partial updates reduce power, dark-mode expressions preferred
- **Deep sleep scheduling:** Enter sleep after 5min inactivity, wake on RTC alarm or button interrupt
- **Wi-Fi duty cycling:** Connect → update → disconnect pattern vs. constant connection
- **Animation frame rate:** 10-15 FPS sufficient for emotional expressions, avoid 30+ FPS unless necessary
- **Alternative consideration:** E-ink display would enable multi-day battery life but sacrifices animation fluidity

**Typical power profile (estimated):**
- Active with OLED: ~80-120mA
- Active with Wi-Fi: ~150-200mA
- Deep sleep: <1mA
- Target battery life: 8-12 hours active use, 3-5 days standby

---

## 📐 System Architecture Diagram Requirements

**Recommended diagram structure:**

1. **Physical topology:** Device placement and connections
2. **Network layer:** Communication protocols between nodes
3. **Data flow:** Message direction and frequency
4. **State management:** Who owns which state, synchronization strategy
5. **Failure modes:** Fallback behaviors when connections lost

**Key elements to visualize:**
- Laptop ↔ Pi communication (Tailscale VPN)
- Pi ↔ SANGI communication (local Wi-Fi, protocol choice)
- SANGI standalone mode trigger conditions
- Optional: Mobile app ↔ SANGI direct connection (BLE)
- Data payload structure at each hop

---

## Implementation Context

**Development environment:**
- ESP32-C3 programming: Arduino IDE or PlatformIO
- Display library: Adafruit GFX or U8g2
- Pi scripting: Python 3.x with psutil, requests/paho-mqtt
- Version control: Git repository recommended for code + animation assets

**Testing approach:**
1. Mock data injection for animation testing (hardcoded states)
2. Serial console for debugging state transitions
3. Isolated network testing before integration
4. Progressive integration testing (one feature at a time)
5. Long-term battery life validation

**Design philosophy rationale:**
This architecture prioritizes **reliable autonomous operation** while enabling **rich integration capabilities**. The fallback mechanism ensures SANGI remains functional in any context, avoiding frustration from dependency failures. Modular design allows stopping at any phase with complete functionality.

---

## Technical Constraints & Considerations

**ESP32-C3 limitations:**
- Flash memory budget for animation frames
- RAM constraints for frame buffering
- Wi-Fi connection stability on battery power
- Display refresh rate vs. power consumption tradeoff

**Network considerations:**
- Latency tolerance (expressions don't require real-time, 1-2s acceptable)
- Message queueing if disconnected temporarily
- Discovery mechanism (mDNS for sangi.local hostname)

**Animation considerations:**
- Bitmap storage format (compressed vs. raw)
- Frame interpolation vs. discrete frames
- Expression library size management

---

## Next Development Step Decision Point

**Current decision required:**

Choose initial focus area:
- [ ] Animation system refinement (frame transitions, expression library)
- [ ] Wi-Fi communication layer (HTTP server, command parsing)
- [ ] Pi integration script (metrics collection, message sending)
- [ ] Architecture diagram creation (visual planning document)
- [ ] Physical enclosure design (form factor, button placement)

**Recommendation:** Start with animation system to establish core personality, then add communication layer for integration capability.

---

*This document serves as both human-readable specification and AI context for development assistance. Technical details are complete enough for implementation while maintaining conceptual clarity for collaboration.*