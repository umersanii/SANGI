import { create } from "zustand"
import type { MQTTState, EmotionState, BatteryStatus, SystemStatus, EmotionStatus, SerialLog } from "@/types/sangi"
import { MockMQTTClient } from "@/lib/mock-mqtt"
import { RealMQTTClient } from "@/lib/mqtt-client"

interface MQTTStore extends MQTTState {
  // Connection management
  connect: (useMock: boolean) => Promise<void>
  disconnect: () => void

  // Publishing
  publishEmotion: (emotion: EmotionState) => void
  publishSound: (type: string, volume: number) => void

  // Message handling
  addMessage: (topic: string, payload: string) => void
  clearMessages: () => void

  // Serial log handling
  addSerialLog: (log: SerialLog) => void
  clearSerialLogs: () => void

  // Status updates
  updateBattery: (battery: BatteryStatus) => void
  updateSystem: (system: SystemStatus) => void
  updateEmotion: (emotion: EmotionStatus) => void

  // Internal
  setConnecting: (connecting: boolean) => void
  setError: (error: string | null) => void
}

let mqttClient: MockMQTTClient | RealMQTTClient | null = null

export const useMQTTStore = create<MQTTStore>((set, get) => ({
  connected: false,
  connecting: false,
  error: null,
  messages: [],
  serialLogs: [],
  battery: null,
  system: null,
  emotion: null,
  lastEmotionTriggered: null,

  connect: async (useMock: boolean) => {
    set({ connecting: true, error: null })
    try {
      if (useMock) {
        mqttClient = new MockMQTTClient()
      } else {
        const endpoint = process.env.NEXT_PUBLIC_AWS_IOT_ENDPOINT || "wss://localhost:8883"
        const clientPrefix = process.env.NEXT_PUBLIC_MQTT_CLIENT_PREFIX || "sangi-web"
        const clientId = `${clientPrefix}-${Date.now()}`
        const region = process.env.NEXT_PUBLIC_AWS_REGION || "us-east-1"
        
        // Optional: AWS credentials for SigV4 signing
        const credentials = process.env.NEXT_PUBLIC_AWS_ACCESS_KEY_ID ? {
          accessKeyId: process.env.NEXT_PUBLIC_AWS_ACCESS_KEY_ID,
          secretAccessKey: process.env.NEXT_PUBLIC_AWS_SECRET_ACCESS_KEY || "",
          sessionToken: process.env.NEXT_PUBLIC_AWS_SESSION_TOKEN,
        } : undefined
        
        mqttClient = new RealMQTTClient(endpoint, clientId, region, credentials)
      }

      await mqttClient.connect()

      // Subscribe to status topics
      mqttClient.subscribe("sangi/status/battery", (payload) => {
        try {
          const battery = JSON.parse(payload)
          get().updateBattery(battery)
          get().addMessage("sangi/status/battery", payload)
        } catch (e) {
          console.error("[MQTT] Failed to parse battery message:", e)
        }
      })

      mqttClient.subscribe("sangi/status/system", (payload) => {
        try {
          const system = JSON.parse(payload)
          get().updateSystem(system)
          get().addMessage("sangi/status/system", payload)
        } catch (e) {
          console.error("[MQTT] Failed to parse system message:", e)
        }
      })

      mqttClient.subscribe("sangi/status/emotion", (payload) => {
        try {
          const emotion = JSON.parse(payload)
          get().updateEmotion(emotion)
          get().addMessage("sangi/status/emotion", payload)
        } catch (e) {
          console.error("[MQTT] Failed to parse emotion message:", e)
        }
      })

      // Subscribe to serial logs
      mqttClient.subscribe("sangi/logs/serial", (payload) => {
        try {
          const log = JSON.parse(payload)
          get().addSerialLog(log)
        } catch (e) {
          console.error("[MQTT] Failed to parse serial log:", e)
        }
      })

      set({ connected: true, connecting: false })
    } catch (error) {
      const errorMessage = error instanceof Error ? error.message : "Connection failed"
      set({ error: errorMessage, connecting: false, connected: false })
    }
  },

  disconnect: () => {
    if (mqttClient) {
      mqttClient.disconnect()
      mqttClient = null
    }
    set({ connected: false })
  },

  publishEmotion: (emotion: EmotionState) => {
    if (!mqttClient) {
      set({ error: "MQTT client not connected" })
      return
    }

    // Emotion commands are plain text, uppercase
    mqttClient.publish("sangi/command/emotion", emotion, 0)
    set({ lastEmotionTriggered: { emotion, timestamp: Date.now() } })
    get().addMessage("sangi/command/emotion", emotion)
  },

  publishSound: (type: string, volume: number) => {
    if (!mqttClient) {
      set({ error: "MQTT client not connected" })
      return
    }

    const payload = JSON.stringify({ type, volume })
    mqttClient.publish("sangi/command/sound", payload, 0)
    get().addMessage("sangi/command/sound", payload)
  },

  addMessage: (topic: string, payload: string) => {
    set((state) => ({
      messages: [
        { topic, payload, timestamp: Date.now() },
        ...state.messages.slice(0, 49), // Keep last 50 messages
      ],
    }))
  },

  clearMessages: () => {
    set({ messages: [] })
  },

  addSerialLog: (log: SerialLog) => {
    set((state) => ({
      serialLogs: [
        log,
        ...state.serialLogs.slice(0, 499), // Keep last 500 log lines
      ],
    }))
  },

  clearSerialLogs: () => {
    set({ serialLogs: [] })
  },

  updateBattery: (battery: BatteryStatus) => {
    set({ battery })
  },

  updateSystem: (system: SystemStatus) => {
    set({ system })
  },

  updateEmotion: (emotion: EmotionStatus) => {
    set({ emotion })
  },

  setConnecting: (connecting: boolean) => {
    set({ connecting })
  },

  setError: (error: string | null) => {
    set({ error })
  },
}))
