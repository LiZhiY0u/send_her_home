function buildLayout(source, options = {}) {
  const height = options.heightMm === undefined ? 300 : options.heightMm
  const rotation = options.topRotation === undefined ? 0 : options.topRotation
  if (!Number.isFinite(height) || height <= 0 || !Number.isFinite(rotation)) throw new Error('Invalid dimensions')
  if (!source || source.units !== 'mm' || !Array.isArray(source.points) || source.points.length !== 133)
    throw new Error('Expected 133 LED positions in mm')
  const counts = { CENTER: 0, UR: 0, LR: 0, LL: 0, UL: 0 }
  const ids = new Set(), coordinates = new Set()
  for (const p of source.points) {
    const key = `${p.channel}:${p.index}`, xy = `${p.x}:${p.y}`
    if (!Number.isFinite(p.x) || !Number.isFinite(p.y) || Math.abs(p.x) > 100 || Math.abs(p.y) > 100 ||
        !Object.prototype.hasOwnProperty.call(counts, p.region) || !['A', 'B'].includes(p.channel) ||
        !Number.isInteger(p.index) || p.index < 0 || ids.has(key) || coordinates.has(xy))
      throw new Error('Invalid or duplicate LED position')
    ids.add(key); coordinates.add(xy); counts[p.region]++
  }
  if (Object.keys(counts).some(k => counts[k] !== (k === 'CENTER' ? 33 : 25))) throw new Error('Invalid region counts')
  const a = rotation * Math.PI / 180
  return ['bottom', 'top'].reduce((all, layer) => all.concat(source.points.map(p => {
    let x = p.x, horizontalY = p.y
    if (layer === 'top') {
      if (options.topMirror) x = -x
      const rotatedX = x * Math.cos(a) - horizontalY * Math.sin(a)
      horizontalY = x * Math.sin(a) + horizontalY * Math.cos(a); x = rotatedX
    }
    return { id: `${layer}:${p.channel}:${p.index}`, layer, region: p.region,
      x, y: (layer === 'top' ? 1 : -1) * height / 2, z: -horizontalY,
      radius: Math.hypot(x, horizontalY), angle: Math.atan2(horizontalY, x) }
  })), [])
}
module.exports = { buildLayout }
