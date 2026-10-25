const {test}=require('node:test'),assert=require('node:assert/strict')
const C=require('../../wechat-mini-program/color/color-math')
test('HSV RGB and HEX conversions support precise arbitrary colors',()=>{
 assert.deepEqual(C.hsvToRgb(0,100,100),[255,0,0])
 assert.deepEqual(C.hsvToRgb(120,100,100),[0,255,0])
 assert.deepEqual(C.hsvToRgb(240,100,100),[0,0,255])
 assert.equal(C.normalizeHex('#abc'),'#AABBCC');assert.equal(C.normalizeHex('ff7096'),'#FF7096')
 assert.equal(C.normalizeHex('#ff00xx'),null)
 for(let r=0;r<=255;r+=51)for(let g=0;g<=255;g+=51)for(let b=0;b<=255;b+=51){
  const hsv=C.rgbToHsv(r,g,b),rgb=C.hsvToRgb(hsv.h,hsv.s,hsv.v)
  assert(rgb.every((v,i)=>Math.abs(v-[r,g,b][i])<=1))
 }
})
test('wheel maps local and viewport touches consistently and clamps edge saturation',()=>{
 const rect={left:20,top:30,width:200,height:200}
 assert.deepEqual(C.wheelPoint({x:200,y:100},rect),{h:0,s:100})
 assert.deepEqual(C.wheelPoint({clientX:220,clientY:130},rect),{h:0,s:100})
 assert.equal(C.wheelPoint({x:100,y:100},rect).s,0)
 assert.equal(C.wheelPoint({x:400,y:100},rect).s,100)
 assert.equal(C.wheelPoint({x:NaN,y:0},rect),null)
})
