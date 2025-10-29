import type { EmotionState } from "@/types/sangi"

export class MockMQTTClient {
  private listeners: Map<string, Set<(payload: string) => void>> = new Map()
  private isConnected = false
  private simulationInterval: NodeJS.Timeout | null = null

  async connect(): Promise<void> {
    return new Promise((resolve) => {
      setTimeout(() => {
        this.isConnected = true
        this.startSimulation()
        resolve()
      }, 500)
    })
  }

  disconnect(): void {
    this.isConnected = false
    if (this.simulationInterval) {
      clearInterval(this.simulationInterval)
    }
  }

  subscribe(topic: string, callback: (payload: string) => void): void {
    if (!this.listeners.has(topic)) {
      this.listeners.set(topic, new Set())
    }
    this.listeners.get(topic)!.add(callback)
  }

  unsubscribe(topic: string, callback: (payload: string) => void): void {
    this.listeners.get(topic)?.delete(callback)
  }

  publish(topic: string, payload: string): void {
    // Simulate publish delay
    setTimeout(() => {
      const callbacks = this.listeners.get(topic)
      if (callbacks) {
        callbacks.forEach((cb) => cb(payload))
      }
    }, 100)
  }

  private startSimulation(): void {
    // Simulate ESP32 publishing status updates every 5 seconds
    this.simulationInterval = setInterval(() => {
      if (!this.isConnected) return

      // Simulate battery status
      const batteryPayload = JSON.stringify({
        level: Math.floor(Math.random() * 100),
        voltage: (3.0 + Math.random() * 1.2).toFixed(2),
        charging: Math.random() > 0.7,
      })
      this.publish("sangi/status/battery", batteryPayload)

      // Simulate system status
      const systemPayload = JSON.stringify({
        uptime: Math.floor(Date.now() / 1000),
        heap: 150000 + Math.floor(Math.random() * 50000),
        rssi: -90 + Math.floor(Math.random() * 60),
      })
      this.publish("sangi/status/system", systemPayload)

      // Occasionally simulate emotion changes
      if (Math.random() > 0.7) {
        const emotions: EmotionState[] = [
          "IDLE",
          "HAPPY",
          "SLEEPY",
          "EXCITED",
          "SAD",
          "ANGRY",
          "CONFUSED",
          "THINKING",
          "LOVE",
          "SURPRISED",
        ]
        const randomEmotion = emotions[Math.floor(Math.random() * emotions.length)]
        const emotionPayload = JSON.stringify({
          current: randomEmotion,
          timestamp: Date.now(),
        })
        this.publish("sangi/status/emotion", emotionPayload)
      }
    }, 5000)

    // Simulate serial logs every 1-3 seconds
    const serialInterval = setInterval(() => {
      if (!this.isConnected) return

      const mockSerialLines = [
        "=== Network Manager Initializing ===",
        "Connecting to WiFi: MyNetwork",
        "✅ WiFi connected - IP: 192.168.1.42",
        "📡 MQTT connecting...",
        "✅ MQTT connected to AWS IoT Core",
        "🔊 BeepManager initialized on GPIO 9",
        "Battery: 85% (4.12V)",
        "Emotion transition: NEUTRAL → HAPPY",
        "GitHub stats loaded: umersanii - 42 repos, 156 followers, 397 contributions",
        "⚠️  Warning: Heap low (142KB free)",
        "❌ Error: Failed to parse JSON payload",
        "System uptime: 3600 seconds",
        "Received command: EMOTION_EXCITED",
        "Display frame rendered in 12ms",
        "Audio detection: Music playing"
      ]

      const randomLine = mockSerialLines[Math.floor(Math.random() * mockSerialLines.length)]
      const serialPayload = JSON.stringify({
        line: randomLine,
        timestamp: Date.now()
      })
      this.publish("sangi/logs/serial", serialPayload)
    }, 1000 + Math.random() * 2000)

    // Store interval for cleanup
    if (this.simulationInterval) {
      const originalInterval = this.simulationInterval
      this.simulationInterval = {
        ...originalInterval,
        [Symbol.dispose]: () => {
          clearInterval(originalInterval)
          clearInterval(serialInterval)
        }
      } as any
    }
  }

  isConnectedStatus(): boolean {
    return this.isConnected
  }
}
