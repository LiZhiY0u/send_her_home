const { test } = require('node:test')
const assert = require('node:assert/strict')
const { EFFECTS, LINK_MODES, sampleEffect } = require('../../wechat-mini-program/lighting/effects')
test('waves extinguish troughs and keep narrow bright peaks',()=>{
 const settings={effect:'waveX',color:'#ffffff',brightness:100,speed:50}
 const sample=x=>sampleEffect({x,z:0,layer:'bottom',region:'CENTER'},settings,0)[0]
 assert(sample(-Math.PI/2/.045)<.001)
 assert(Math.abs(sample(Math.PI/2/.045)-1)<.00001)
 assert(sample(0)<.1)
})
test('effect library provides twenty two effects with bounded animated output', () => {
  assert.equal(EFFECTS.length, 22)
  assert.equal(LINK_MODES.length, 4)
  for (const { value } of EFFECTS) {
    const samples = [0, .37, 1.2, 3.8].flatMap(t => [-80, 0, 40, 90].map(x =>
      sampleEffect({ x, z: -30, radius: Math.hypot(x,30), angle: Math.atan2(30,x), layer:'top',region:'UL' },
        {effect:value,linkedMode:'delay',brightness:100,color:'#ff8000'},t)))
    assert(samples.flat().every(v => Number.isFinite(v) && v >= 0 && v <= 1), value)
    if(value !== 'solid') assert(new Set(samples.map(JSON.stringify)).size > 1, value)
  }
})
test('mirror and delay transform top-layer wave while sync keeps layers equal', () => {
  const p={x:45,z:-25,radius:51,angle:.5,region:'CENTER',layer:'bottom'}
  const s={effect:'waveX',color:'#ffffff',brightness:100,speed:50}
  const bottom=sampleEffect(p,s,1.3)
  assert.deepEqual(bottom,sampleEffect({...p,layer:'top'},s,1.3))
  assert.notDeepEqual(bottom,sampleEffect({...p,layer:'top'},{...s,linkedMode:'mirror'},1.3))
  assert.deepEqual(sampleEffect({...p,layer:'top'},{...s,linkedMode:'delay'},1.8),bottom)
})
