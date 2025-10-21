import type { EmotionState } from "@/types/sangi"
import { EmotionButton } from "./emotion-button"

const EMOTIONS: EmotionState[] = [
  "HAPPY",
  "SAD",
  "ANGRY",
  "SURPRISED",
  "SLEEPY",
  "CONFUSED",
  "LOVING",
  "EXCITED",
  "BORED",
  "CURIOUS",
  "SCARED",
  "PROUD",
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
