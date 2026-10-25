const P=require('../painting/protocol')
function createLightingStateClient(connection){
 let sequence=0,pending=null,off=null,closed=false
 const session=1+Math.floor(Math.random()*255)
 function cancel(error){if(pending){const p=pending;pending=null;clearTimeout(p.timer);p.reject(error)}}
 off=connection.onStatus(value=>{
  if(!value){cancel(new Error('蓝牙已断开'));return}
  const state=P.decodeStatus(value)
  if(!state||state.op!==7||state.session!==session||!pending||state.sequence!==pending.sequence)return
  const p=pending;pending=null;clearTimeout(p.timer)
  if(state.code)p.reject(new Error('设备状态忙，请稍后重试'));else p.resolve(state)
 })
 return {
  async read(){
   if(closed)throw new Error('页面已关闭')
   if(pending)return pending.promise
   await connection.enablePainting();if(closed)throw new Error('页面已关闭')
   if(pending)return pending.promise
   const seq=sequence=(sequence+1)&65535
   const promise=new Promise((resolve,reject)=>{pending={sequence:seq,resolve,reject,timer:setTimeout(()=>cancel(new Error('灯光状态读取超时')),2500)}})
   pending.promise=promise;promise.catch(()=>{})
   try{await connection.write(P.WRITE,P.packet(7,session,seq));return await promise}catch(e){cancel(e);throw e}
  },
  dispose(){closed=true;cancel(new Error('页面已关闭'));if(off)off();off=null}
  ,idle(){return pending?pending.promise.catch(()=>{}):Promise.resolve()}
 }
}
module.exports={createLightingStateClient}
