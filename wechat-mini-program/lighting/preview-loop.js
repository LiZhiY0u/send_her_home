function createPreviewLoop({ requestFrame, cancelFrame, render, now }) {
  let running = false, disposed = false, frameId = null, elapsed = 0, previous = 0, painted = -Infinity, generation = 0
  function schedule(token) {
    frameId = requestFrame(() => {
      if (!running || disposed || token !== generation) return
      frameId = null
      const current = now()
      elapsed += Math.max(0, current - previous); previous = current
      if (current - painted >= 1000 / 30) { render(elapsed / 1000); painted = current }
      if (running && !disposed && token === generation) schedule(token)
    })
  }
  function stop() {
    if (!running) return
    elapsed += Math.max(0, now() - previous)
    running = false; generation++
    if (frameId !== null) cancelFrame(frameId)
    frameId = null
  }
  return {
    start() {
      if (running || disposed) return
      running = true; previous = now(); painted = -Infinity
      schedule(++generation)
    },
    stop,
    draw() { if (!disposed) render(elapsed / 1000) },
    dispose() { stop(); disposed = true }
  }
}
module.exports = { createPreviewLoop }
