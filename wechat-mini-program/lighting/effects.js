const finite = (value, fallback) => Number.isFinite(value) ? value : fallback
const clamp = (value, min, max) => Math.max(min, Math.min(max, value))
function selected(point, settings) {
  return (!settings.layer || settings.layer === 'both' || settings.layer === point.layer) &&
    (!settings.region || settings.region === 'all' || settings.region === point.region ||
      (settings.region === 'corners' && point.region !== 'CENTER'))
}
function sampleEffect(point, settings, elapsedSeconds) {
  if (!selected(point, settings)) return [0, 0, 0]
  const brightness = clamp(finite(settings.brightness, 60), 0, 100) / 100
  const speed = clamp(finite(settings.speed, 50), 0, 100) / 50
  const time = Math.max(0, finite(elapsedSeconds, 0))
  const reverse = settings.linkedMode === 'reverse' && point.layer === 'top'
  let strength = 1
  if (settings.effect === 'rotate') {
    const phase = (reverse ? -1 : 1) * time * speed * Math.PI / 2
    const distance = Math.abs(Math.sin(finite(point.angle, 0) - phase)) * finite(point.radius, 0)
    strength = clamp(1 - distance / 14, 0, 1)
  } else if (settings.effect === 'ripple') {
    const phase = ((time * speed / 2) % 1 + 1) % 1
    const radius = (reverse ? 1 - phase : phase) * 160
    strength = clamp(1 - Math.abs(finite(point.radius, 0) - radius) / 16, 0, 1)
  }
  const hex = /^#[0-9a-f]{6}$/i.test(settings.color) ? settings.color.slice(1) : '57d9ff'
  return [0, 2, 4].map(i => parseInt(hex.slice(i, i + 2), 16) / 255 * brightness * strength)
}
module.exports = { sampleEffect, selected }
