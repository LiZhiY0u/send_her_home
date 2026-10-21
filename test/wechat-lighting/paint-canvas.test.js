const {test}=require('node:test'),assert=require('node:assert/strict')
const {toWorld}=require('../../wechat-mini-program/painting/canvas')
test('Canvas and view coordinates map to the same 200mm board, excluding outside',()=>{
  const rect={left:20,top:100,width:400,height:400}
  assert.deepEqual(toWorld({x:200,y:200},rect),{x:0,y:0})
  assert.deepEqual(toWorld({clientX:220,clientY:300},rect),{x:0,y:0})
  assert.deepEqual(toWorld({x:0,y:0},rect),{x:-100,y:100})
  assert.equal(toWorld({x:-1,y:0},rect),null)
  assert.equal(toWorld({x:NaN,y:1},rect),null)
})
