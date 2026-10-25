const {WRITE}=require('../painting/protocol')
function packet(op,session,sequence,state,mask=15){
 const p=[0xb8,1,op,session&255,session>>>8,sequence&255,sequence>>>8]
 if(op===2)p.push(+state.running,+state.reverse,state.speed,+state.keepRunning,mask)
 return Uint8Array.from(p)
}
function decode(value){
 const p=new Uint8Array(value)
 if(p.length!==16||p[0]!==0xb8||p[1]!==1||p[8]>1||p[9]>1||p[10]>100||p[11]>1)return null
 return {code:p[2],op:p[3],session:p[4]|p[5]<<8,sequence:p[6]|p[7]<<8,
  running:!!p[8],reverse:!!p[9],speed:p[10],keepRunning:!!p[11],remote:!!p[12]}
}
module.exports={WRITE,packet,decode}
