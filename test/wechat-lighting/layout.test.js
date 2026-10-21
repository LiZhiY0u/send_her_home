const { test } = require('node:test')
const assert = require('node:assert/strict')
const source = require('../../wechat-mini-program/lighting/led-map-133.json')
const { buildLayout } = require('../../wechat-mini-program/lighting/layout')

test('real layout produces 266 unique spatial LEDs in two layers', () => {
  const points = buildLayout(source)
  assert.equal(points.length, 266)
  assert.equal(new Set(points.map(p => p.id)).size, 266)
  for (const layer of ['top', 'bottom']) {
    const group = points.filter(p => p.layer === layer)
    assert.equal(group.length, 133)
    for (const region of ['CENTER', 'UR', 'LR', 'LL', 'UL'])
      assert.equal(group.filter(p => p.region === region).length, region === 'CENTER' ? 33 : 25)
    assert(group.every(p => p.y === (layer === 'top' ? 150 : -150)))
  }
  const sample = points.find(p => p.layer === 'bottom' && p.id.endsWith(':A:1'))
  assert.equal(sample.x, 9.4281)
  assert.equal(sample.z, 9.4281)
})
test('top orientation is independent of bottom', () => {
  const points = buildLayout(source, { heightMm: 400, topRotation: 90, topMirror: true })
  const top = points.find(p => p.id === 'top:A:1')
  assert(Math.abs(top.x - 9.4281) < 1e-6)
  assert(Math.abs(top.z - 9.4281) < 1e-6)
  assert.equal(top.y, 200)
  assert.equal(points.find(p => p.id === 'bottom:A:1').x, 9.4281)
})
test('reject malformed coordinates, IDs, counts and dimensions', () => {
  for (const mutate of [s => s.points.pop(), s => { s.points[1].x = null },
    s => { s.points[1].x = 999 }, s => { s.points[1] = { ...s.points[0] } },
    s => { s.points[1].region = 'BAD' }]) {
    const invalid = JSON.parse(JSON.stringify(source)); mutate(invalid)
    assert.throws(() => buildLayout(invalid))
  }
  assert.throws(() => buildLayout(source, { heightMm: -1 }))
})
