const P=require('./protocol')
function createPaintSender(connection,onState,options={}) {
  const delay=options.delay|| (ms=>new Promise(resolve=>setTimeout(resolve,ms)))
  const now=options.now||Date.now
  let active=false,token=0,session=0,sequence=0,pending=null,running=null,off=null,waiter=null,lastError=null,lastSent=-Infinity
  function cancelWait(error,owner){if(waiter&&(owner===undefined||waiter.owner===owner)){const w=waiter;waiter=null;clearTimeout(w.timer);w.reject(error)}}
  function listen(){
    if(off)return
    off=connection.onStatus(value=>{
      if(!value){active=false;token++;cancelWait(new Error('蓝牙已断开'));onState('连接断开，草稿未丢失');return}
      const s=P.decodeStatus(value)
      if(!s||s.session!==session||!waiter||s.sequence!==waiter.sequence)return
      if(s.code){lastError=new Error('设备拒绝画面，错误码 '+s.code);cancelWait(lastError);return}
      if(s.op!==waiter.op)return
      const w=waiter;waiter=null;clearTimeout(w.timer);w.resolve(s)
    })
  }
  function wait(op,seq,owner){
    if(waiter)throw new Error('绘画传输已有未完成事务')
    const promise=new Promise((resolve,reject)=>{
      waiter={op,sequence:seq,owner,resolve,reject,timer:setTimeout(()=>cancelWait(new Error('设备确认超时'),owner),options.timeout||5000)}
    })
    promise.catch(()=>{})
    return promise
  }
  function check(t){if(t!==token)throw new Error('传输已取消');if(lastError)throw lastError}
  async function handshake(t){
    await connection.enablePainting();check(t);listen()
    session=1+((Math.random()*255)|0);sequence=0;lastError=null
    const ack=wait(1,0,t)
    await connection.write(P.WRITE,P.packet(1,session,0));check(t)
    const status=await ack;check(t)
    if(status.hash!==P.hash||status.count!==133)throw new Error('灯位映射不匹配，请更新小程序和固件')
  }
  async function sendFrame(frame,t){
    for(let attempt=0;attempt<2;++attempt){
      check(t);lastError=null
      const seq=++sequence,ack=wait(4,seq,t)
      try{
        for(const packet of P.framePackets(frame,session,seq)){check(t);await connection.write(P.WRITE,packet);check(t)}
        await ack;check(t);return seq
      }catch(error){cancelWait(error,t);if(t!==token)throw new Error('传输已取消');lastError=null;if(attempt===1)throw error}
    }
  }
  function drain(){
    if(running)return
    const t=token
    running=(async()=>{
      try{
        while(active&&pending){
          await delay(Math.max(0,100-(now()-lastSent)));check(t)
          const frame=pending;pending=null;lastSent=now()
          const seq=await sendFrame(frame,t)
          onState(pending?'最新笔画待同步':`设备已同步 · 帧${seq}`)
        }
      }catch(error){if(t===token){active=false;pending=null;onState(error.errMsg||error.message||'同步失败')}}
      finally{running=null;if(active&&pending)drain()}
    })()
  }
  const api={
    async start(frame){
      api.stop();const t=token;lastError=null;listen();onState('正在连接并核对灯位…')
      try{await handshake(t);check(t);active=true;api.submit(frame)}
      catch(error){cancelWait(error,t);if(t===token){active=false;throw error}}
    },
    submit(frame){
      if(!active)return
      if(frame.length!==399)throw new Error('画面长度错误')
      pending=Uint8Array.from(frame);onState('本地已更新，等待设备确认');drain()
    },
    stop(){active=false;pending=null;token++;cancelWait(new Error('传输已取消'));if(off){off();off=null}},
    async applyEffect(settings){
      const { EFFECTS }=require('../lighting/effects')
      const id=EFFECTS.findIndex(item=>item.value===settings.effect)
      if(id<0||!/^#[0-9a-f]{6}$/i.test(settings.color))throw new Error('灯效参数错误')
      const percent=v=>Math.max(0,Math.min(100,Math.round(Number(v)||0)))
      const color=settings.color.slice(1)
      const payload=[id,percent(settings.speed),percent(settings.brightness),... [0,2,4].map(i=>parseInt(color.slice(i,i+2),16))]
      if(settings.randomColor)payload.push(1)
      api.stop();if(running)await running
      const t=token;lastError=null;listen()
      try{
        await handshake(t);check(t)
        const seq=++sequence,ack=wait(6,seq,t)
        await connection.write(P.WRITE,P.packet(6,session,seq,0,0,payload));await ack;check(t)
        onState('底面灯效已应用')
      }finally{if(t===token)api.stop()}
    },
    async restoreAuto(){
      api.stop();if(running)await running
      const t=token;lastError=null;listen()
      try{
        await handshake(t);check(t);const seq=++sequence,ack=wait(5,seq,t)
        await connection.write(P.WRITE,P.packet(5,session,seq));await ack;check(t)
        onState('设备已恢复自动灯效')
      }finally{if(t===token)api.stop()}
    }
  }
  return api
}
module.exports={createPaintSender}
