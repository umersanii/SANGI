import { useMQTTStore } from "@/stores/mqtt-store"
import { Wifi, WifiOff, Loader } from "lucide-react"

export function ConnectionStatus() {
  const { connected, connecting, error } = useMQTTStore()

  if (connecting) {
    return (
      <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-slate-800/50 border border-slate-700">
        <Loader className="w-4 h-4 animate-spin text-yellow-500" />
        <span className="text-sm text-slate-300">Connecting...</span>
      </div>
    )
  }

  if (error) {
    return (
      <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-red-900/30 border border-red-700/50">
        <WifiOff className="w-4 h-4 text-red-500" />
        <span className="text-sm text-red-300">Error</span>
      </div>
    )
  }

  if (connected) {
    return (
      <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-green-900/30 border border-green-700/50">
        <Wifi className="w-4 h-4 text-green-500" />
        <span className="text-sm text-green-300">Connected</span>
      </div>
    )
  }

  return (
    <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-slate-800/50 border border-slate-700">
      <WifiOff className="w-4 h-4 text-slate-500" />
      <span className="text-sm text-slate-400">Disconnected</span>
    </div>
  )
}
