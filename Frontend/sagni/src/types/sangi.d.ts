export type EmotionState =
  | "NEUTRAL"
  | "HAPPY"
  | "SAD"
  | "ANGRY"
  | "SURPRISED"
  | "SLEEPY"
  | "CONFUSED"
  | "LOVING"
  | "EXCITED"
  | "BORED"
  | "CURIOUS"
  | "SCARED"
  | "PROUD"

export interface BatteryStatus {
  level: number
  voltage: number
  charging: boolean
}

export interface SystemStatus {
  uptime: number
  heap: number
  rssi: number
}

export interface EmotionStatus {
  current: EmotionState
  timestamp: number
}

export interface MQTTMessage {
  topic: string
  payload: string
  timestamp: number
}

export interface SerialLog {
  line: string
  timestamp: number
}

export interface MQTTState {
  connected: boolean
  connecting: boolean
  error: string | null
  messages: MQTTMessage[]
  serialLogs: SerialLog[]
  battery: BatteryStatus | null
  system: SystemStatus | null
  emotion: EmotionStatus | null
  lastEmotionTriggered: { emotion: EmotionState; timestamp: number } | null
}
