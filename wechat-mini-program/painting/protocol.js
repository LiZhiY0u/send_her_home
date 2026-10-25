const { hash } = require('./physical-map')
const base='0000FF00-0000-1000-8000-00805F9B34FB'
const uuid=n=>base.replace('FF00',n)
const SERVICE=uuid('FF10'), KEY=uuid('FF11'), WRITE=uuid('FF12'), STATUS=uuid('FF13')
function crc(bytes) {
  let v=65535
  for(const b of bytes){v^=b<<8;for(let i=0;i<8;++i)v=((v&32768)?(v<<1)^0x1021:v<<1)&65535}
  return v
}
function packet(op,session,sequence,start=0,count=0,payload=[]) {
  return Uint8Array.from([0xa7,1,op,session,sequence&255,sequence>>>8,start,count,...payload])
}
function framePackets(frame,session,sequence) {
  if(frame.length!==399)throw new Error('画面必须包含133颗灯')
  const packets=[packet(2,session,sequence)]
  for(let start=0;start<133;start+=4){const count=Math.min(4,133-start);packets.push(packet(3,session,sequence,start,count,frame.slice(start*3,(start+count)*3)))}
  const checksum=crc(frame);packets.push(packet(4,session,sequence,0,0,[checksum&255,checksum>>>8]))
  return packets
}
function decodeStatus(bytes) {
  const p=new Uint8Array(bytes)
  if(![16,20].includes(p.length)||p[0]!==0xa7||p[1]!==1)return null
  if(p.length===20){
    if(p[7]!==7||p[8]>=22||p[9]>100||p[10]>100||p[14]>1||p[15]>2||p[16]>6||p[17]>3)return null
    return {code:p[2],mode:p[3],session:p[4],sequence:p[5]|p[6]<<8,op:7,
      effect:p[8],speed:p[9],brightness:p[10],color:'#'+Array.from(p.slice(11,14),v=>v.toString(16).padStart(2,'0')).join(''),
      randomColor:!!p[14],layer:['both','top','bottom'][p[15]],region:['all','CENTER','corners','UL','UR','LL','LR'][p[16]],linkedMode:['sync','reverse','mirror','delay'][p[17]],elapsed:(p[18]|p[19]<<8)/100}
  }
  return {code:p[2],mode:p[3],session:p[4],sequence:p[5]|p[6]<<8,op:p[7],
    hash:(p[8]|p[9]<<8|p[10]<<16|p[11]<<24)>>>0,count:p[12],brightness:p[13]}
}
module.exports={SERVICE,KEY,WRITE,STATUS,hash,crc,packet,framePackets,decodeStatus}
