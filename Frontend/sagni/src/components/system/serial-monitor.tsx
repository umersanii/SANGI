"use client"

import { useEffect, useRef, useState } from "react"
import { useMQTTStore } from "@/stores/mqtt-store"
import { Terminal, Trash2, Pause, Play, Download } from "lucide-react"

export function SerialMonitor() {
  const { serialLogs, clearSerialLogs } = useMQTTStore()
  const [isPaused, setIsPaused] = useState(false)
  const [filter, setFilter] = useState("")
  const scrollRef = useRef<HTMLDivElement>(null)
  const [autoScroll, setAutoScroll] = useState(true)

  // Auto-scroll to bottom when new logs arrive
  useEffect(() => {
    if (autoScroll && !isPaused && scrollRef.current) {
      scrollRef.current.scrollTop = scrollRef.current.scrollHeight
    }
  }, [serialLogs, autoScroll, isPaused])

  // Filter logs based on search term
  const filteredLogs = serialLogs.filter((log) => {
    if (!filter) return true
    return log.line.toLowerCase().includes(filter.toLowerCase())
  })

  // Display logs (show paused snapshot or live logs)
  const displayLogs = isPaused ? filteredLogs : filteredLogs

  const handleDownload = () => {
    const content = serialLogs.map((log) => {
      const date = new Date(log.timestamp)
      return `[${date.toISOString()}] ${log.line}`
    }).join("\n")

    const blob = new Blob([content], { type: "text/plain" })
    const url = URL.createObjectURL(blob)
    const a = document.createElement("a")
    a.href = url
    a.download = `sangi-serial-${Date.now()}.log`
    a.click()
    URL.revokeObjectURL(url)
  }

  const formatTime = (timestamp: number) => {
    const date = new Date(timestamp)
    return date.toLocaleTimeString("en-US", { 
      hour12: false,
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      fractionalSecondDigits: 3
    })
  }

  // Detect log level from content
  const getLogLevel = (line: string): "error" | "warning" | "info" | "debug" => {
    const lower = line.toLowerCase()
    if (lower.includes("error") || lower.includes("❌") || lower.includes("failed")) return "error"
    if (lower.includes("warning") || lower.includes("⚠️")) return "warning"
    if (lower.includes("===") || lower.includes("✅") || lower.includes("🔌")) return "info"
    return "debug"
  }

  const getLevelColor = (level: string) => {
    switch (level) {
      case "error": return "text-red-400"
      case "warning": return "text-yellow-400"
      case "info": return "text-blue-400"
      default: return "text-slate-400"
    }
  }

  return (
    <div className="flex flex-col h-full bg-slate-950/50 border border-slate-800 rounded-lg overflow-hidden">
      {/* Header */}
      <div className="flex items-center justify-between px-4 py-3 border-b border-slate-800 bg-slate-900/30">
        <div className="flex items-center gap-2">
          <Terminal className="w-4 h-4 text-green-400" />
          <h3 className="text-sm font-semibold text-slate-200">Serial Monitor</h3>
          <span className="text-xs text-slate-500">
            {serialLogs.length} line{serialLogs.length !== 1 ? "s" : ""}
            {filter && ` (${filteredLogs.length} filtered)`}
          </span>
        </div>

        <div className="flex items-center gap-2">
          {/* Filter input */}
          <input
            type="text"
            placeholder="Filter logs..."
            value={filter}
            onChange={(e) => setFilter(e.target.value)}
            className="px-3 py-1 text-xs bg-slate-900 border border-slate-700 rounded text-slate-300 focus:outline-none focus:border-slate-600 w-40"
          />

          {/* Auto-scroll toggle */}
          <button
            onClick={() => setAutoScroll(!autoScroll)}
            className={`p-1.5 rounded hover:bg-slate-800 transition-colors ${
              autoScroll ? "text-green-400" : "text-slate-500"
            }`}
            title={autoScroll ? "Auto-scroll enabled" : "Auto-scroll disabled"}
          >
            <svg className="w-4 h-4" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 14l-7 7m0 0l-7-7m7 7V3" />
            </svg>
          </button>

          {/* Pause/Resume */}
          <button
            onClick={() => setIsPaused(!isPaused)}
            className="p-1.5 rounded hover:bg-slate-800 transition-colors text-slate-400"
            title={isPaused ? "Resume" : "Pause"}
          >
            {isPaused ? <Play className="w-4 h-4" /> : <Pause className="w-4 h-4" />}
          </button>

          {/* Download logs */}
          <button
            onClick={handleDownload}
            className="p-1.5 rounded hover:bg-slate-800 transition-colors text-slate-400"
            title="Download logs"
          >
            <Download className="w-4 h-4" />
          </button>

          {/* Clear logs */}
          <button
            onClick={clearSerialLogs}
            className="p-1.5 rounded hover:bg-slate-800 transition-colors text-slate-400"
            title="Clear logs"
          >
            <Trash2 className="w-4 h-4" />
          </button>
        </div>
      </div>

      {/* Log content */}
      <div
        ref={scrollRef}
        className="flex-1 overflow-y-auto font-mono text-xs p-3 space-y-0.5"
        style={{ maxHeight: "400px" }}
      >
        {displayLogs.length === 0 ? (
          <div className="flex items-center justify-center h-full text-slate-600">
            <div className="text-center">
              <Terminal className="w-8 h-8 mx-auto mb-2 opacity-50" />
              <p>No serial output yet</p>
              <p className="text-xs mt-1">
                {filter ? "No logs match your filter" : "Waiting for ESP32 data..."}
              </p>
            </div>
          </div>
        ) : (
          displayLogs.map((log, idx) => {
            const level = getLogLevel(log.line)
            return (
              <div key={idx} className="flex gap-2 hover:bg-slate-900/30 px-1 -mx-1 rounded">
                <span className="text-slate-600 select-none shrink-0">
                  {formatTime(log.timestamp)}
                </span>
                <span className={`${getLevelColor(level)} break-all`}>
                  {log.line}
                </span>
              </div>
            )
          })
        )}
      </div>

      {/* Status bar */}
      <div className="px-4 py-2 border-t border-slate-800 bg-slate-900/30 text-xs text-slate-500 flex items-center justify-between">
        <div>
          {isPaused ? (
            <span className="text-yellow-400">⏸ Paused</span>
          ) : (
            <span className="text-green-400">● Live</span>
          )}
        </div>
        <div>
          Topic: <span className="text-slate-400 font-mono">sangi/logs/serial</span>
        </div>
      </div>
    </div>
  )
}
