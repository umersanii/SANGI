"use client"

// Individual emotion button with icon, label, and last triggered timestamp

import { useMQTTStore } from "@/stores/mqtt-store"
import type { EmotionState } from "@/types/sangi"
import { Button } from "@/components/ui/button"
import { useState } from "react"

const EMOTION_ICONS: Record<EmotionState, string> = {
  IDLE: "😐",
  HAPPY: "😊",
  SLEEPY: "😴",
  EXCITED: "🤩",
  SAD: "😢",
  ANGRY: "😠",
  CONFUSED: "😕",
  THINKING: "🤔",
  LOVE: "😍",
  SURPRISED: "😲",
  DEAD: "💀",
  MUSIC: "🎵",
  BLINK: "👁️",
  NOTIFICATION: "🔔",
  CODING: "💻",
  GITHUB_STATS: "📊",
}

interface EmotionButtonProps {
  emotion: EmotionState
}

export function EmotionButton({ emotion }: EmotionButtonProps) {
  const { publishEmotion, lastEmotionTriggered, emotion: currentEmotion } = useMQTTStore()
  const [isPublishing, setIsPublishing] = useState(false)

  const isActive = currentEmotion?.current === emotion
  const wasLastTriggered = lastEmotionTriggered?.emotion === emotion

  const handleClick = async () => {
    setIsPublishing(true)
    publishEmotion(emotion)
    setTimeout(() => setIsPublishing(false), 300)
  }

  const formatLastTriggered = () => {
    if (!wasLastTriggered || !lastEmotionTriggered) return ""
    const now = Date.now()
    const diff = now - lastEmotionTriggered.timestamp
    const seconds = Math.floor(diff / 1000)

    if (seconds < 60) return `${seconds}s ago`
    const minutes = Math.floor(seconds / 60)
    if (minutes < 60) return `${minutes}m ago`
    const hours = Math.floor(minutes / 60)
    return `${hours}h ago`
  }

  return (
    <Button
      onClick={handleClick}
      disabled={isPublishing}
      className={`flex flex-col items-center justify-center h-24 rounded-lg transition-all ${
        isActive
          ? "bg-blue-600 border-2 border-blue-400 shadow-lg shadow-blue-500/50"
          : wasLastTriggered
            ? "bg-slate-800 border-2 border-slate-700 hover:border-slate-600"
            : "bg-slate-800/50 border-2 border-slate-700 hover:bg-slate-800"
      }`}
    >
      <div className="text-3xl mb-1">{EMOTION_ICONS[emotion]}</div>
      <div className="text-xs font-semibold text-slate-200">{emotion}</div>
      {wasLastTriggered && formatLastTriggered() && (
        <div className="text-xs text-slate-400 mt-0.5">{formatLastTriggered()}</div>
      )}
    </Button>
  )
}
