import mqtt, { type MqttClient } from "mqtt"
import { SignatureV4 } from "@smithy/signature-v4"
import { HttpRequest } from "@smithy/protocol-http"
import { Sha256 } from "@aws-crypto/sha256-js"
import { toHex } from "@smithy/util-hex-encoding"

interface AWSCredentials {
  accessKeyId: string
  secretAccessKey: string
  sessionToken?: string
}

export class RealMQTTClient {
  private client: MqttClient | null = null
  private endpoint: string
  private clientId: string
  private region: string
  private credentials: AWSCredentials | null = null

  constructor(endpoint: string, clientId: string, region?: string, credentials?: AWSCredentials) {
    this.endpoint = endpoint
    this.clientId = clientId
    this.region = region || process.env.NEXT_PUBLIC_AWS_REGION || "us-east-1"
    this.credentials = credentials || null
  }

  private async signWebSocketUrl(): Promise<string> {
    if (!this.credentials) {
      // If no credentials provided, use IoT Core's certificate-based auth
      // This requires proper CORS configuration on AWS IoT
      return this.endpoint
    }

    // Extract hostname from endpoint (e.g., "wss://xxx.iot.us-east-1.amazonaws.com/mqtt")
    const url = new URL(this.endpoint)
    const hostname = url.hostname
    
    // Create HTTP request for signing
    const request = new HttpRequest({
      method: "GET",
      protocol: "wss:",
      hostname: hostname,
      path: "/mqtt",
      headers: {
        host: hostname,
      },
    })

    // Sign the request using AWS SigV4
    const signer = new SignatureV4({
      service: "iotdevicegateway",
      region: this.region,
      credentials: this.credentials,
      sha256: Sha256,
    })

    const signedRequest = await signer.sign(request)
    
    // Build signed WebSocket URL with query parameters
    const searchParams = new URLSearchParams()
    if (signedRequest.headers) {
      Object.entries(signedRequest.headers).forEach(([key, value]) => {
        if (typeof value === "string") {
          searchParams.append(key, value)
        }
      })
    }

    return `wss://${hostname}/mqtt?${searchParams.toString()}`
  }

  async connect(): Promise<void> {
    return new Promise(async (resolve, reject) => {
      try {
        // Sign WebSocket URL if credentials are provided
        const wsUrl = await this.signWebSocketUrl()

        // Connect to AWS IoT Core via WebSocket
        this.client = mqtt.connect(wsUrl, {
          clientId: this.clientId,
          protocol: "wss",
          protocolVersion: 4, // AWS IoT supports MQTT 3.1.1
          clean: true,
          reconnectPeriod: 1000,
          connectTimeout: 30000,
          keepalive: 60,
        })

        this.client.on("connect", () => {
          console.log("[MQTT] Connected to AWS IoT Core")
          resolve()
        })

        this.client.on("error", (error) => {
          console.error("[MQTT] Connection error:", error)
          reject(error)
        })

        this.client.on("offline", () => {
          console.warn("[MQTT] Client offline")
        })

        this.client.on("reconnect", () => {
          console.log("[MQTT] Attempting to reconnect...")
        })
      } catch (error) {
        reject(error)
      }
    })
  }

  disconnect(): void {
    if (this.client) {
      this.client.end()
      this.client = null
    }
  }

  subscribe(topic: string, callback: (payload: string) => void): void {
    if (!this.client) return

    this.client.subscribe(topic, (err) => {
      if (err) {
        console.error(`[MQTT] Failed to subscribe to ${topic}:`, err)
      }
    })

    this.client.on("message", (receivedTopic, message) => {
      if (receivedTopic === topic) {
        callback(message.toString())
      }
    })
  }

  unsubscribe(topic: string): void {
    if (!this.client) return
    this.client.unsubscribe(topic)
  }

  publish(topic: string, payload: string, qos: 0 | 1 | 2 = 0): void {
    if (!this.client) {
      console.warn("[MQTT] Client not connected, cannot publish")
      return
    }

    this.client.publish(topic, payload, { qos }, (err) => {
      if (err) {
        console.error(`[MQTT] Failed to publish to ${topic}:`, err)
      }
    })
  }

  isConnectedStatus(): boolean {
    return this.client?.connected ?? false
  }
}
