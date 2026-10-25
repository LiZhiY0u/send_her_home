const {test}=require('node:test'),assert=require('node:assert/strict')
const {createMotorClient}=require('../../wechat-mini-program/motor/client')
function fake(){let listener;const writes=[];const state={running:false,reverse:false,speed:30,keepRunning:true}
 return {writes,state,connection:{enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{listener=null}},write:async(uuid,p)=>{
 writes.push(p);if(p[2]===2){if(p[11]&1)state.running=!!p[7];if(p[11]&2)state.reverse=!!p[8];if(p[11]&4)state.speed=p[9];if(p[11]&8)state.keepRunning=!!p[10]}
 if(p[2]===4)state.running=false
 const reply=Uint8Array.from([0xb8,1,0,p[2],p[3],p[4],p[5],p[6],+state.running,+state.reverse,state.speed,+state.keepRunning,1,0,0,0]);if(listener)listener(reply.buffer)
 }}}
}
const options={setInterval:()=>1,clearInterval:()=>{},timeout:100}
test('speed and keep policy changes do not replay a stale start after OLED stops',async()=>{
 const f=fake(),client=createMotorClient(f.connection,()=>{},options)
 await client.open();await client.set({running:true});f.state.running=false
 await client.set({speed:50});assert(!f.state.running);assert.equal(f.writes.at(-1)[11],4)
 await client.set({keepRunning:false});assert(!f.state.running);assert.equal(f.writes.at(-1)[11],8)
 await client.leave()
})
test('opening queries only; keep-running exit sends no stop and reconnect never starts',async()=>{
 const f=fake(),client=createMotorClient(f.connection,()=>{},options)
 await client.open();assert.deepEqual(f.writes.map(p=>p[2]),[1])
 await client.set({running:true});assert(f.state.running)
 await client.leave();assert.equal(f.writes.at(-1)[2],2);assert(f.state.running)
 await client.open();assert.equal(f.writes.at(-1)[2],1);await client.leave()
})
test('disabled keep mode stops on leave and start requires known applied state',async()=>{
 const f=fake(),client=createMotorClient(f.connection,()=>{},options)
 await assert.rejects(client.set({running:true}))
 await client.open();await client.set({keepRunning:false});await client.set({running:true})
 await client.leave();assert.equal(f.writes.at(-1)[2],4);assert(!f.state.running)
})
test('stop from unknown state connects and queries before sending only STOP',async()=>{
 const f=fake(),client=createMotorClient(f.connection,()=>{},options)
 await client.stop();assert.deepEqual(f.writes.map(p=>p[2]),[1,4]);assert(!f.state.running);await client.leave()
})
test('late old leave cannot dispose a newer connection session',async()=>{
 let listener,stopPacket;const callbacks=[]
 const reply=p=>Uint8Array.from([0xb8,1,0,p[2],p[3],p[4],p[5],p[6],0,0,30,0,1,0,0,0]).buffer
 const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;callbacks.push(f);return()=>{}},write:async(uuid,p)=>{
  if(p[2]===4)stopPacket=p;else listener(reply(p))
 }}
 const client=createMotorClient(connection,()=>{},options)
 await client.open();const leaving=client.leave();leaving.catch(()=>{})
 for(let i=0;i<20&&!stopPacket;++i)await Promise.resolve()
 assert(stopPacket);await client.open();await leaving.catch(()=>{})
 await client.set({speed:40});await client.leave().catch(()=>{})
})
test('delayed canceled write cannot reject the new session acknowledgement',async()=>{
 let listener,release,latest,helloCount=0
 const blocked=new Promise((resolve,reject)=>{release=()=>reject(new Error('old write failed'))})
 const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{}},write:async(uuid,p)=>{
  if(p[2]===1&&++helloCount===1)await blocked
  else latest=p
 }}
 const client=createMotorClient(connection,()=>{}, {...options,timeout:500})
 const old=client.open();old.catch(()=>{})
 for(let i=0;i<20&&helloCount===0;++i)await Promise.resolve()
 await client.leave();const current=client.open();current.catch(()=>{})
 for(let i=0;i<20&&!latest;++i)await Promise.resolve()
 release();await old.catch(()=>{})
 listener(Uint8Array.from([0xb8,1,0,1,latest[3],latest[4],latest[5],latest[6],0,0,30,1,1,0,0,0]).buffer)
 await current;await client.leave()
})
