import { useMQTTStore } from "@/stores/mqtt-store"
import { Battery, Wifi, Clock } from "lucide-react"

export function SystemStats() {
  const { battery, system } = useMQTTStore()

  const formatUptime = (seconds: number) => {
    const hours = Math.floor(seconds / 3600)
    const minutes = Math.floor((seconds % 3600) / 60)
    const secs = seconds % 60
    return `${hours}h ${minutes}m ${secs}s`
  }

  const getBatteryColor = (level: number) => {
    if (level > 60) return "text-green-500"
    if (level > 30) return "text-yellow-500"
    return "text-red-500"
  }

  const getSignalColor = (rssi: number) => {
    if (rssi > -50) return "text-green-500"
    if (rssi > -70) return "text-yellow-500"
    return "text-red-500"
  }

  return (
    <div className="space-y-3">
      {battery && (
        <div className="p-3 bg-slate-800/50 border border-slate-700 rounded-lg">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-2">
              <Battery className={`w-4 h-4 ${getBatteryColor(battery.level)}`} />
              <span className="text-xs text-slate-400">Battery</span>
            </div>
            <div className="text-sm font-semibold text-slate-200">
              {battery.level}% ({battery.voltage}V)
            </div>
          </div>
        </div>
      )}

      {system && (
        <>
          <div className="p-3 bg-slate-800/50 border border-slate-700 rounded-lg">
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-2">
                <Wifi className={`w-4 h-4 ${getSignalColor(system.rssi)}`} />
                <span className="text-xs text-slate-400">Signal</span>
              </div>
              <div className="text-sm font-semibold text-slate-200">{system.rssi} dBm</div>
            </div>
          </div>

          <div className="p-3 bg-slate-800/50 border border-slate-700 rounded-lg">
            <div className="flex items-center justify-between">
              <div className="flex items-center gap-2">
                <Clock className="w-4 h-4 text-blue-500" />
                <span className="text-xs text-slate-400">Uptime</span>
              </div>
              <div className="text-sm font-semibold text-slate-200">{formatUptime(system.uptime)}</div>
            </div>
          </div>

          <div className="p-3 bg-slate-800/50 border border-slate-700 rounded-lg">
            <div className="flex items-center justify-between">
              <span className="text-xs text-slate-400">Heap Free</span>
              <div className="text-sm font-semibold text-slate-200">{(system.heap / 1024).toFixed(1)} KB</div>
            </div>
          </div>
        </>
      )}

      {!battery && !system && (
        <div className="p-3 bg-slate-800/50 border border-slate-700 rounded-lg text-center">
          <p className="text-xs text-slate-500">Waiting for device data...</p>
        </div>
      )}
    </div>
  )
}
