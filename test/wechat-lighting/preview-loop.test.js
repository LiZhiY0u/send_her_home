const { test } = require('node:test')
const assert = require('node:assert/strict')
const { createPreviewLoop } = require('../../wechat-mini-program/lighting/preview-loop')
test('loop is single owner, throttles, freezes hidden time and disposes safely', () => {
  let now = 0, id = 0; const pending = new Map(), rendered = []
  const loop = createPreviewLoop({ now: () => now,
    requestFrame: cb => { pending.set(++id, cb); return id },
    cancelFrame: key => pending.delete(key), render: t => rendered.push(t) })
  const frame = ms => { now = ms; const jobs = [...pending.values()]; pending.clear(); jobs.forEach(cb => cb()) }
  loop.start(); loop.start(); assert.equal(pending.size, 1)
  frame(0); frame(10); assert.equal(rendered.length, 1)
  frame(40); assert.equal(rendered.length, 2)
  loop.stop(); assert.equal(pending.size, 0)
  now = 10040; loop.start(); frame(10040)
  assert.equal(rendered.at(-1), 0.04)
  const late = [...pending.values()][0]; loop.dispose(); late()
  assert.equal(pending.size, 0); loop.start(); assert.equal(pending.size, 0)
})
