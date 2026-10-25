const {test}=require('node:test'),assert=require('node:assert/strict')
const P=require('../../wechat-mini-program/painting/protocol')
const {createLightingStateClient}=require('../../wechat-mini-program/lighting/device-state')
const {createKeyRepeat}=require('../../wechat-mini-program/ble/key-repeat')
const {createLightingPage}=require('../../wechat-mini-program/lighting/page-controller')

test('lighting query reads full device settings without resetting paint session or issuing a command',async()=>{
 let listener;const writes=[]
 const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{listener=null}},write:async(_,p)=>{
  writes.push(p);const status=new Uint8Array([0xa7,1,0,3,p[3],p[4],p[5],7,7,40,80,255,128,0,1,2,6,3,123,0]);listener(status.buffer)
 }}
 const client=createLightingStateClient(connection),state=await client.read()
 assert.deepEqual(writes.map(p=>p[2]),[7]);assert.equal(state.effect,7);assert.equal(state.layer,'bottom');assert.equal(state.region,'LR');assert.equal(state.linkedMode,'delay');assert.equal(state.color,'#ff8000');assert.equal(state.elapsed,1.23)
 client.dispose();assert.equal(listener,null)
})
test('invalid extended lighting status is rejected',()=>{
 const status=new Uint8Array([0xa7,1,0,3,1,1,0,7,7,40,80,255,128,0,1,2,6,3,0,0]);assert(P.decodeStatus(status.buffer));status[16]=7;assert.equal(P.decodeStatus(status.buffer),null)
})
test('direction repeats after hold delay and release prevents further writes',async()=>{
 const writes=[],timers=[]
 const repeat=createKeyRepeat(async code=>writes.push(code),{setTimeout:(f,ms)=>{timers.push({f,ms});return timers.length-1},clearTimeout:i=>{timers[i].cleared=true}})
 repeat.start(1);await Promise.resolve();assert.deepEqual(writes,[1]);assert.equal(timers[0].ms,400)
 timers[0].f();await Promise.resolve();assert.deepEqual(writes,[1,1]);assert.equal(timers[1].ms,150)
 repeat.stop();assert(timers[1].cleared);timers[1].f();await Promise.resolve();assert.deepEqual(writes,[1,1])
})
test('device state synchronizes controls but never overwrites local uncommitted edits',async()=>{
 const p=createLightingPage({now:()=>1000});p.data={...p.data};p.setData=patch=>Object.assign(p.data,patch);p.redraw=()=>{};p._alive=p._visible=true
 p._stateClient={read:async()=>({mode:3,effect:7,speed:40,brightness:80,color:'#ff8000',randomColor:false,layer:'top',region:'UL',linkedMode:'mirror',elapsed:1})}
 await p.readDeviceState();assert.equal(p.data.effect,'squareMove');assert.equal(p.data.layer,'top');assert.equal(p.data.brightness,80)
 p._localEditing=true;p.data.brightness=23;await p.readDeviceState();assert.equal(p.data.brightness,23)
})
test('old direction write failure cannot cancel a newer hold',async()=>{
 let rejectOld;const timers=[],writes=[]
 const repeat=createKeyRepeat(code=>{writes.push(code);return code===1?new Promise((_,reject)=>{rejectOld=reject}):Promise.resolve()},
 {setTimeout:(f,ms)=>{timers.push({f,ms});return timers.length-1},clearTimeout:i=>{timers[i].cleared=true}})
 repeat.start(1);repeat.stop();repeat.start(2);await Promise.resolve()
 rejectOld(new Error('late failure'));await Promise.resolve();await Promise.resolve()
 assert(!timers[0].cleared);timers[0].f();await Promise.resolve();assert.deepEqual(writes,[1,2,2]);repeat.stop()
})
