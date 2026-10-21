const { test } = require('node:test')
const assert = require('node:assert/strict')
const { paintSegment } = require('../../wechat-mini-program/painting/brush')
test('soft segment fills nearby LEDs and interpolates fast movement', () => {
  const points = [{x:0,y:0},{x:0,y:6},{x:0,y:13},{x:30,y:0}]
  const frame = new Uint8Array(12)
  paintSegment(frame, points, {x:-20,y:0}, {x:20,y:0}, {diameter:24,color:[255,0,0]})
  assert.equal(frame[0],255); assert.equal(frame[3],128); assert.equal(frame[6],0); assert(Math.abs(frame[9]-43)<=1)
  paintSegment(frame, points, {x:0,y:0}, {x:0,y:0}, {diameter:24,color:[0,0,0]})
  assert.equal(frame[0],0)
})
