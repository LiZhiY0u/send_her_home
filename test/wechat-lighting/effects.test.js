const { test } = require('node:test')
const assert = require('node:assert/strict')
const { sampleEffect } = require('../../wechat-mini-program/lighting/effects')
const p = { layer: 'bottom', region: 'CENTER', x: 40, z: 0, radius: 40, angle: 0 }
const s = { layer: 'both', region: 'all', effect: 'solid', color: '#ff8000', brightness: 100, speed: 50, linkedMode: 'sync' }
test('solid, brightness, layer and region mask', () => {
  assert.deepEqual(sampleEffect(p, s, 0), [1, 128 / 255, 0])
  assert.deepEqual(sampleEffect(p, { ...s, brightness: 0 }, 0), [0, 0, 0])
  assert.deepEqual(sampleEffect(p, { ...s, layer: 'top' }, 0), [0, 0, 0])
  assert.deepEqual(sampleEffect(p, { ...s, region: 'corners' }, 0), [0, 0, 0])
})
test('rotating diameter illuminates opposite points and reverses top phase', () => {
  const rotate = { ...s, effect: 'rotate' }
  assert.equal(sampleEffect(p, rotate, 0)[0], 1)
  assert(Math.abs(sampleEffect({ ...p, x: -40, angle: Math.PI }, rotate, 0)[0] - 1) < 1e-12)
  assert.equal(sampleEffect({ ...p, angle: Math.PI / 2 }, rotate, 0)[0], 0)
  const top = { ...p, layer: 'top', angle: 0.5 }
  assert.notDeepEqual(sampleEffect(top, rotate, 0.3), sampleEffect(top, { ...rotate, linkedMode: 'reverse' }, 0.3))
})
test('expanding ring travels outward and samples deterministically', () => {
  const ring = { ...s, effect: 'ripple' }
  assert.equal(sampleEffect({ ...p, radius: 0 }, ring, 0)[0], 1)
  assert.equal(sampleEffect(p, ring, 0)[0], 0)
  assert(sampleEffect(p, ring, 0.5)[0] > 0)
  assert.deepEqual(sampleEffect(p, ring, 0.5), sampleEffect(p, ring, 0.5))
})
test('invalid values never create NaN, negative or overbright colors', () => {
  for (const time of [NaN, Infinity, -1, 0]) {
    const rgb = sampleEffect(p, { brightness: Infinity, speed: NaN, color: 'bad', effect: 'ripple' }, time)
    assert(rgb.every(c => Number.isFinite(c) && c >= 0 && c <= 1))
  }
})
