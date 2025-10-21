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
          "HAPPY",
          "SAD",
          "ANGRY",
          "SURPRISED",
          "SLEEPY",
          "CONFUSED",
          "LOVING",
          "EXCITED",
          "BORED",
          "CURIOUS",
          "SCARED",
          "PROUD",
        ]
        const randomEmotion = emotions[Math.floor(Math.random() * emotions.length)]
        const emotionPayload = JSON.stringify({
          current: randomEmotion,
          timestamp: Date.now(),
        })
        this.publish("sangi/status/emotion", emotionPayload)
      }
    }, 5000)
  }

  isConnectedStatus(): boolean {
    return this.isConnected
  }
}
