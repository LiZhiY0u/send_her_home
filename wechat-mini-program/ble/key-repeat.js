function createKeyRepeat(send,clock={setTimeout,clearTimeout}){
 let timer=null,generation=0
 function stop(){generation++;if(timer!==null)clock.clearTimeout(timer);timer=null}
 async function pulse(code,token,first){
  if(token!==generation)return
  try{await send(code)}catch(_){if(token===generation)stop();return}
  if(token===generation)timer=clock.setTimeout(()=>pulse(code,token,false),first?400:150)
 }
 return {start(code){stop();pulse(code,generation,true)},stop}
}
module.exports={createKeyRepeat}
