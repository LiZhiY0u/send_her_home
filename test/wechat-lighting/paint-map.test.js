const {test}=require('node:test'),assert=require('node:assert/strict'),fs=require('node:fs'),path=require('node:path')
const {hash,points}=require('../../wechat-mini-program/painting/physical-map')
test('paint JS shares verified physical LED order and hash with firmware',()=>{
  const cpp=fs.readFileSync(path.join(__dirname,'../../esp32-firmware/src/Diagnostics/LedPhysicalMap.h'),'utf8')
  const expected=[...cpp.matchAll(/\{(-?\d+\.\d+)f, (-?\d+\.\d+)f\}/g)].map(m=>({x:+m[1],y:+m[2]}))
  assert.equal(points.length,133);assert.deepEqual(points,expected)
  let value=2166136261;for(const c of JSON.stringify(points))value=Math.imul(value^c.charCodeAt(0),16777619)>>>0
  assert.equal(hash,value)
  const identity=fs.readFileSync(path.join(__dirname,'../../esp32-firmware/src/Lighting/PaintMapIdentity.h'),'utf8')
  assert(identity.includes(value+'u'))
})
