const P=require('./protocol')
function createMotorClient(connection,onState,options={}){
 const interval=options.setInterval||setInterval,clear=options.clearInterval||clearInterval
 let generation=0,session=0,sequence=0,off=null,poll=null,waiter=null,chain=Promise.resolve(),ready=false,busy=0
 let state={running:false,reverse:false,speed:30,keepRunning:true},message='尚未读取设备状态'
 function emit(){onState({...state,ready,busy:busy>0,message})}
 function cancel(error,owner){if(waiter&&(owner===undefined||waiter.owner===owner)){clearTimeout(waiter.timer);waiter.reject(error);waiter=null}}
 function dispose(){ready=false;generation++;if(poll){clear(poll);poll=null}cancel(new Error('操作已取消'));if(off){off();off=null}}
 async function exchange(op,patch,token){
  if(token!==generation)throw new Error('页面已离开')
  if(op===2&&!ready)throw new Error('设备状态未知，请重新读取')
  if(sequence>=65535)sequence=0
  const seq=++sequence,next={...state,...patch}
  if(op===2&&(!Number.isInteger(next.speed)||next.speed<0||next.speed>100))throw new Error('速度必须是0–100%')
  const has=key=>Object.prototype.hasOwnProperty.call(patch,key)
  const mask=(has('running')?1:0)|(has('reverse')?2:0)|(has('speed')?4:0)|(has('keepRunning')?8:0)
  if(op===2&&(mask&2)&&state.running&&next.reverse!==state.reverse)throw new Error('请先停止再换向')
  const ack=new Promise((resolve,reject)=>{waiter={op,seq,owner:token,resolve,reject,timer:setTimeout(()=>{cancel(new Error('电机确认超时，请确认固件已更新'),token);},options.timeout||2500)}})
  ack.catch(()=>{})
  try{await connection.write(P.WRITE,P.packet(op,session,seq,next,mask));const reply=await ack
   if(token!==generation)throw new Error('操作已取消')
   return reply
  }catch(error){cancel(error,token);throw error}
 }
 function enqueue(op,patch={}){
  if(!ready)return Promise.reject(new Error('请先连接并读取电机状态'))
  const token=generation;busy++;emit()
  const task=chain.catch(()=>{}).then(()=>exchange(op,patch,token))
  chain=task
  return task.catch(error=>{if(token===generation){ready=false;message=error.errMsg||error.message;emit()}throw error})
   .finally(()=>{busy--;if(token===generation)emit()})
 }
 const api={
  async open(){
   dispose();chain=Promise.resolve();const token=generation;message='正在连接并读取电机…';emit()
   try{
    await connection.enablePainting();if(token!==generation)return
    session=1+Math.floor(Math.random()*65535);sequence=0
    off=connection.onStatus(value=>{
     if(token!==generation)return
     if(!value){dispose();message=state.keepRunning?'蓝牙已断开 · 设备可能仍在运行':'蓝牙断开 · 设备将执行停止保护';emit();return}
     const reply=P.decode(value)
     if(!reply||reply.session!==session||!waiter||reply.sequence!==waiter.seq||reply.op!==waiter.op)return
     const w=waiter;waiter=null;clearTimeout(w.timer)
     state={running:reply.running,reverse:reply.reverse,speed:reply.speed,keepRunning:reply.keepRunning,remote:reply.remote}
     if(reply.code){message=reply.code===2?'设备拒绝换向，请先停止':'设备拒绝过期指令';w.reject(new Error(message))}
     else {message='设备状态已同步';w.resolve(reply)}
     emit()
    })
    await exchange(1,{},token);if(token!==generation)return
    ready=true;emit()
    poll=interval(()=>{if(ready&&!busy)enqueue(1).catch(()=>{})},1000)
   }catch(error){if(token===generation){dispose();message=error.errMsg||error.message||'读取失败';emit()}throw error}
  },
  set:patch=>enqueue(2,patch),
  async stop(){if(!ready)await api.open();return enqueue(4)},
  async leave(){
   const token=generation
   if(poll){clear(poll);poll=null}
   try{if(ready&&!state.keepRunning)await enqueue(4)}finally{if(token===generation)dispose()}
  }
 }
 return api
}
module.exports={createMotorClient}
