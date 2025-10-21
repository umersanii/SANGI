"use client"

// Main application component with layout and connection management

import { useEffect, useState } from "react"
import { useMQTTStore } from "@/stores/mqtt-store"
import { ConnectionStatus } from "@/components/mqtt/connection-status"
import { MessageLog } from "@/components/mqtt/message-log"
import { PublishPanel } from "@/components/mqtt/publish-panel"
import { EmotionGrid } from "@/components/emotion/emotion-grid"
import { SystemStats } from "@/components/system/system-stats"
import { Power, Settings } from "lucide-react"

export default function App() {
  const { connected, connect, disconnect } = useMQTTStore()
  const [useMock, setUseMock] = useState(true)
  const [showSettings, setShowSettings] = useState(false)

  useEffect(() => {
    // Auto-connect on mount
    connect(useMock)

    return () => {
      disconnect()
    }
  }, [])

  const handleToggleMock = async () => {
    disconnect()
    setUseMock(!useMock)
    await connect(!useMock)
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950">
      {/* Header */}
      <header className="border-b border-slate-800 bg-slate-950/50 backdrop-blur-sm sticky top-0 z-10">
        <div className="max-w-7xl mx-auto px-4 py-4 flex items-center justify-between">
          <div>
            <h1 className="text-2xl font-bold text-slate-100">SANGI Control</h1>
            <p className="text-xs text-slate-500">ESP32-C3 Companion Robot Interface</p>
          </div>

          <div className="flex items-center gap-3">
            <ConnectionStatus />

            <button
              type="button"
              onClick={() => setShowSettings(!showSettings)}
              className="border border-slate-700 text-slate-300 hover:bg-slate-800 rounded px-2 py-1"
            >
              <Settings className="w-4 h-4" />
            </button>

            <button
              type="button"
              onClick={() => (connected ? disconnect() : connect(useMock))}
              className="border border-slate-700 text-slate-300 hover:bg-slate-800 rounded px-2 py-1"
            >
              <Power className="w-4 h-4" />
            </button>
          </div>
        </div>

        {/* Settings Panel */}
        {showSettings && (
          <div className="border-t border-slate-800 bg-slate-900/50 px-4 py-3">
            <div className="max-w-7xl mx-auto flex items-center gap-4">
              <label className="flex items-center gap-2 cursor-pointer">
                <input type="checkbox" checked={useMock} onChange={handleToggleMock} className="w-4 h-4" />
                <span className="text-sm text-slate-300">
                  {useMock ? "Mock Mode (Development)" : "Real MQTT (Production)"}
                </span>
              </label>
              <p className="text-xs text-slate-500">
                {useMock ? "Using simulated data for development" : "Connected to real AWS IoT Core"}
              </p>
            </div>
          </div>
        )}
      </header>

      {/* Main Content */}
      <main className="max-w-7xl mx-auto px-4 py-6">
        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* Left Sidebar */}
          <div className="lg:col-span-1 space-y-4">
            <div className="p-4 bg-slate-900/50 border border-slate-800 rounded-lg backdrop-blur-sm">
              <h2 className="text-sm font-semibold text-slate-200 mb-3">System Status</h2>
              <SystemStats />
            </div>

            <PublishPanel />
          </div>

          {/* Center Panel */}
          <div className="lg:col-span-2">
            <div className="p-6 bg-slate-900/50 border border-slate-800 rounded-lg backdrop-blur-sm">
              <h2 className="text-sm font-semibold text-slate-200 mb-4">Emotion Control</h2>
              <EmotionGrid />
            </div>
          </div>
        </div>
      </main>

      {/* Message Log Footer */}
      <MessageLog />
    </div>
  )
}
