"use client"

// Message log showing MQTT publish/subscribe activity

import { useMQTTStore } from "@/stores/mqtt-store"
import { Button } from "@/components/ui/button"
import { ChevronDown, Trash2 } from "lucide-react"
import { useState } from "react"

export function MessageLog() {
  const { messages, clearMessages } = useMQTTStore()
  const [expanded, setExpanded] = useState(false)

  const formatTime = (timestamp: number) => {
    return new Date(timestamp).toLocaleTimeString()
  }

  const formatPayload = (payload: string) => {
    try {
      return JSON.stringify(JSON.parse(payload), null, 2)
    } catch {
      return payload
    }
  }

  return (
    <div className="border-t border-slate-800 bg-slate-950/50">
      <button
        onClick={() => setExpanded(!expanded)}
        className="w-full flex items-center justify-between px-4 py-3 hover:bg-slate-900/50 transition-colors"
      >
        <div className="flex items-center gap-2">
          <ChevronDown className={`w-4 h-4 text-slate-400 transition-transform ${expanded ? "rotate-180" : ""}`} />
          <span className="text-sm font-medium text-slate-300">Message Log ({messages.length})</span>
        </div>
        {messages.length > 0 && (
          <Button
            variant="ghost"
            size="sm"
            onClick={(e) => {
              e.stopPropagation()
              clearMessages()
            }}
            className="h-8 w-8 p-0"
          >
            <Trash2 className="w-4 h-4" />
          </Button>
        )}
      </button>

      {expanded && (
        <div className="max-h-64 overflow-y-auto bg-slate-900/30 border-t border-slate-800">
          {messages.length === 0 ? (
            <div className="p-4 text-center text-slate-500 text-sm">No messages yet</div>
          ) : (
            <div className="divide-y divide-slate-800">
              {messages.map((msg, idx) => (
                <div key={idx} className="p-3 text-xs font-mono">
                  <div className="flex justify-between mb-1">
                    <span className="text-blue-400">{msg.topic}</span>
                    <span className="text-slate-500">{formatTime(msg.timestamp)}</span>
                  </div>
                  <pre className="text-slate-300 whitespace-pre-wrap break-words">{formatPayload(msg.payload)}</pre>
                </div>
              ))}
            </div>
          )}
        </div>
      )}
    </div>
  )
}
