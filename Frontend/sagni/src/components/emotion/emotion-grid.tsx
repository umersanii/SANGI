import type { EmotionState } from "@/types/sangi"
import { EmotionButton } from "./emotion-button"

const EMOTIONS: EmotionState[] = [
  "IDLE",
  "HAPPY",
  "SLEEPY",
  "EXCITED",
  "SAD",
  "ANGRY",
  "CONFUSED",
  "THINKING",
  "LOVE",
  "SURPRISED",
  "DEAD",
  "MUSIC",
  "BLINK",
  "NOTIFICATION",
  "CODING",
  "GITHUB_STATS",
]

export function EmotionGrid() {
  return (
    <div className="grid grid-cols-3 md:grid-cols-4 gap-3">
      {EMOTIONS.map((emotion) => (
        <EmotionButton key={emotion} emotion={emotion} />
      ))}
    </div>
  )
}
