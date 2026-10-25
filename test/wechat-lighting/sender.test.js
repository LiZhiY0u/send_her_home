const {test}=require('node:test'),assert=require('node:assert/strict')
const {createPaintSender}=require('../../wechat-mini-program/painting/sender')
const P=require('../../wechat-mini-program/painting/protocol')
function fake(){
  let listener,commit;const writes=[]
  function ack(packet){const status=new Uint8Array(16);status.set([0xa7,1,0,1,packet[3],packet[4],packet[5],packet[2]]);new DataView(status.buffer).setUint32(8,P.hash,true);status[12]=133;listener(status.buffer)}
  const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{listener=null}},write:async(uuid,packet)=>{writes.push(packet);if(packet[2]===1)ack(packet);if(packet[2]===4)commit=packet;if(packet[2]===5)ack(packet)}}
  return{connection,writes,applied:()=>ack(commit),committed:()=>!!commit}
}
test('sender waits for application ACK, coalesces pending snapshots and stops',async()=>{
  const f=fake(),states=[]
  const sender=createPaintSender(f.connection,s=>states.push(s),{delay:async()=>{},timeout:200})
  await sender.start(new Uint8Array(399))
  for(let i=0;i<100&&!f.committed();++i)await Promise.resolve()
  assert(f.committed());assert(!states.some(s=>s.startsWith('设备已同步')))
  sender.submit(new Uint8Array(399).fill(1));sender.submit(new Uint8Array(399).fill(2))
  f.applied();for(let i=0;i<100;++i)await Promise.resolve()
  const chunks=f.writes.filter(p=>p[2]===3&&p[4]===2)
  assert(chunks.length>0);assert.equal(chunks[0][8],2)
  sender.stop();const n=f.writes.length;for(let i=0;i<10;++i)await Promise.resolve();assert.equal(f.writes.length,n)
})
function statusFor(packet,code=0){
  const s=new Uint8Array(16);s.set([0xa7,1,code,1,packet[3],packet[4],packet[5],packet[2]])
  new DataView(s.buffer).setUint32(8,P.hash,true);s[12]=133;return s.buffer
}
test('effect sends stable ID and parameters and waits for firmware application ACK',async()=>{
 let listener,effectPacket;const states=[]
 const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{listener=null}},write:async(uuid,p)=>{
   if(p[2]===1)listener(statusFor(p));if(p[2]===6)effectPacket=p
 }}
 const sender=createPaintSender(connection,s=>states.push(s),{timeout:200})
 const promise=sender.applyEffect({effect:'squareMove',speed:50,brightness:80,color:'#ff8000'})
 for(let i=0;i<30&&!effectPacket;++i)await Promise.resolve()
 assert.deepEqual(Array.from(effectPacket.slice(8)),[7,50,80,255,128,0])
 assert(!states.includes('底面灯效已应用'));listener(statusFor(effectPacket));await promise
 assert(states.includes('底面灯效已应用'))
})
test('random color effect appends mode flag to the BLE command',async()=>{
 let listener,packet
 const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{}},write:async(uuid,p)=>{if(p[2]===6)packet=p;listener(statusFor(p))}}
 const sender=createPaintSender(connection,()=>{},{timeout:100})
 await sender.applyEffect({effect:'waveX',speed:50,brightness:65,color:'#ff0000',randomColor:true})
 assert.equal(packet.length,15);assert.equal(packet[14],1)
})
test('device busy is retried once with a fresh frame sequence',async()=>{
  let listener,commits=0;const states=[]
  const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{listener=null}},write:async(uuid,packet)=>{
    if(packet[2]===1)listener(statusFor(packet))
    if(packet[2]===4)listener(statusFor(packet,++commits===1?3:0))
  }}
  const sender=createPaintSender(connection,s=>states.push(s),{delay:async()=>{},timeout:100})
  await sender.start(new Uint8Array(399))
  for(let i=0;i<250;++i)await Promise.resolve()
  assert.equal(commits,2);assert(states.some(s=>s.startsWith('设备已同步')));sender.stop()
})
test('late canceled handshake cannot cancel a newer start',async()=>{
  let listener,release;const hello=[]
  const blocked=new Promise(resolve=>{release=resolve})
  const connection={enablePainting:async()=>{},onStatus:f=>{listener=f;return()=>{listener=null}},write:async(uuid,p)=>{
    if(p[2]===1){hello.push(p);if(hello.length===1)await blocked}
    else if(p[2]===4)listener(statusFor(p))
  }}
  const sender=createPaintSender(connection,()=>{},{delay:async()=>{},timeout:500})
  const old=sender.start(new Uint8Array(399));old.catch(()=>{})
  for(let i=0;i<10;++i)await Promise.resolve()
  sender.stop();const newer=sender.start(new Uint8Array(399));newer.catch(()=>{})
  for(let i=0;i<10;++i)await Promise.resolve()
  assert.equal(hello.length,2);release();for(let i=0;i<10;++i)await Promise.resolve()
  listener(statusFor(hello[1]));await newer;sender.stop();await old
})
