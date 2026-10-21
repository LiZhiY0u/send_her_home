const clamp = (v, lo, hi) => Math.max(lo, Math.min(hi, v))
function createGestures() {
  const state = { yaw: 0.6, pitch: 1.12, distance: 620 }
  let previous = []
  // Canvas events use x/y; ordinary view/browser events use clientX/clientY.
  const valid = touches => (touches || []).map(p => ({
    x: Number.isFinite(p.x) ? p.x : p.clientX,
    y: Number.isFinite(p.y) ? p.y : p.clientY
  })).filter(p => Number.isFinite(p.x) && Number.isFinite(p.y))
  const distance = p => Math.hypot(p[0].x - p[1].x, p[0].y - p[1].y)
  return { state,
    start(touches) { previous = valid(touches) },
    move(touches) {
      const next = valid(touches)
      if (next.length === previous.length) {
        if (next.length === 1) {
          state.yaw -= (next[0].x - previous[0].x) * 0.008
          state.pitch = clamp(state.pitch - (next[0].y - previous[0].y) * 0.008, 0.01, Math.PI - 0.01)
        } else if (next.length === 2) {
          const before = distance(previous), after = distance(next)
          if (before > 0 && after > 0) state.distance = clamp(state.distance * before / after, 280, 1100)
        }
      }
      previous = next
    },
    end() { previous = [] },
    setView(name) {
      previous = []
      Object.assign(state, { yaw: 0.6, pitch: 1.12, distance: 620 })
      if (name === 'top') { state.pitch = 0.01; state.yaw = 0; state.distance = 540 }
    }
  }
}
module.exports = { createGestures }
