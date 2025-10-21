"use client"

// Panel for publishing custom MQTT messages

import { useMQTTStore } from "@/stores/mqtt-store"
import { Button } from "@/components/ui/button"
import { Input } from "@/components/ui/input"
import { useState } from "react"
import { Send } from "lucide-react"

export function PublishPanel() {
  const { connected, publishSound } = useMQTTStore()
  const [topic, setTopic] = useState("sangi/command/sound")
  const [payload, setPayload] = useState('{"type":"BEEP","volume":75}')
  const [qos, setQos] = useState(0)

  const handlePublish = () => {
    if (!topic.trim() || !payload.trim()) return

    try {
      // For sound commands, use the store method
      if (topic === "sangi/command/sound") {
        const parsed = JSON.parse(payload)
        publishSound(parsed.type, parsed.volume)
      } else {
        // For other topics, would need to add generic publish method
        console.log("Publishing to", topic, payload)
      }
    } catch (e) {
      console.error("Invalid JSON payload")
    }
  }

  return (
    <div className="p-4 bg-slate-900/50 border border-slate-800 rounded-lg">
      <h3 className="text-sm font-semibold text-slate-200 mb-3">Publish Message</h3>

      <div className="space-y-3">
        <div>
          <label className="text-xs text-slate-400 mb-1 block">Topic</label>
          <Input
            value={topic}
            onChange={(e) => setTopic(e.target.value)}
            placeholder="sangi/command/sound"
            className="bg-slate-800 border-slate-700 text-slate-100"
            disabled={!connected}
          />
        </div>

        <div>
          <label className="text-xs text-slate-400 mb-1 block">Payload (JSON)</label>
          <textarea
            value={payload}
            onChange={(e) => setPayload(e.target.value)}
            placeholder='{"type":"BEEP","volume":75}'
            className="w-full h-20 px-3 py-2 bg-slate-800 border border-slate-700 rounded text-slate-100 text-xs font-mono"
            disabled={!connected}
          />
        </div>

        <div>
          <label className="text-xs text-slate-400 mb-1 block">QoS</label>
          <select
            value={qos}
            onChange={(e) => setQos(Number(e.target.value))}
            className="w-full px-3 py-2 bg-slate-800 border border-slate-700 rounded text-slate-100 text-sm"
            disabled={!connected}
          >
            <option value={0}>0 - Fire and Forget</option>
            <option value={1}>1 - At Least Once</option>
          </select>
        </div>

        <Button
          onClick={handlePublish}
          disabled={!connected}
          className="w-full bg-blue-600 hover:bg-blue-700 text-white"
        >
          <Send className="w-4 h-4 mr-2" />
          Publish
        </Button>
      </div>
    </div>
  )
}
