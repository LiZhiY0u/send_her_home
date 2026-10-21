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
  if(p.length!==16||p[0]!==0xa7||p[1]!==1)return null
  return {code:p[2],mode:p[3],session:p[4],sequence:p[5]|p[6]<<8,op:p[7],
    hash:(p[8]|p[9]<<8|p[10]<<16|p[11]<<24)>>>0,count:p[12],brightness:p[13]}
}
module.exports={SERVICE,KEY,WRITE,STATUS,hash,crc,packet,framePackets,decodeStatus}
