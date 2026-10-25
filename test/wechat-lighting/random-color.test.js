const {test}=require('node:test'),assert=require('node:assert/strict')
const {cycleColor}=require('../../wechat-mini-program/lighting/random-color')
test('random color holds four seconds then smoothly fades to next color in one second',()=>{
 const base=[255,0,0]
 assert.deepEqual(cycleColor(0,base),base)
 assert.deepEqual(cycleColor(4,base),base)
 const end=cycleColor(5,base),mid=cycleColor(4.5,base)
 assert.notDeepEqual(end,base)
 for(let i=0;i<3;++i)assert(Math.abs(mid[i]-(base[i]+end[i])/2)<=1)
 assert.deepEqual(cycleColor(9,base),end)
})
