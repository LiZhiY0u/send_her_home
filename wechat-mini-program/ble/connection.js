const {SERVICE,KEY,WRITE,STATUS}=require('../painting/protocol')
let deviceId='', connecting=null, paintReady=false, generation=0, writeChain=Promise.resolve()
const listeners=new Set(), statusListeners=new Set()
let initialized=false
function emit(){for(const listener of listeners)listener({connected:!!deviceId,connecting:!!connecting})}
function call(name,args={}) {
  return new Promise((resolve,reject)=>{
    let settled=false
    const timer=setTimeout(()=>finish(reject,new Error(name+' 超时')),10000)
    function finish(fn,value){if(settled)return;settled=true;clearTimeout(timer);fn(value)}
    wx[name]({...args,success:value=>finish(resolve,value),fail:error=>finish(reject,error)})
  })
}
function init(){
  if(initialized)return;initialized=true
  wx.onBLEConnectionStateChange(event=>{
    if(event.deviceId===deviceId&&!event.connected){deviceId='';paintReady=false;generation++;emit();for(const f of statusListeners)f(null)}
  })
  wx.onBLECharacteristicValueChange(event=>{
    if(event.deviceId===deviceId&&event.characteristicId.toUpperCase()===STATUS)for(const f of statusListeners)f(event.value)
  })
}
function findDevice(){return new Promise((resolve,reject)=>{
  let settled=false
  const finish=(fn,value)=>{if(settled)return;settled=true;clearTimeout(timer);wx.offBluetoothDeviceFound(found);wx.stopBluetoothDevicesDiscovery({});fn(value)}
  const found=({devices})=>{const target=devices.find(d=>d.name==='FigureCase'||d.localName==='FigureCase');if(target)finish(resolve,target.deviceId)}
  const timer=setTimeout(()=>finish(reject,new Error('未找到 FigureCase')),10000)
  wx.onBluetoothDeviceFound(found);wx.startBluetoothDevicesDiscovery({allowDuplicatesKey:false,fail:e=>finish(reject,e)})
})}
async function characteristics(id){
  const {services}=await call('getBLEDeviceServices',{deviceId:id})
  if(!services.some(s=>s.uuid.toUpperCase()===SERVICE))throw new Error('未找到FigureCase服务')
  return (await call('getBLEDeviceCharacteristics',{deviceId:id,serviceId:SERVICE})).characteristics
}
async function connect(){
  init();if(deviceId)return deviceId;if(connecting)return connecting
  connecting=(async()=>{
    let id=''
    try {
      await call('openBluetoothAdapter');id=await findDevice();await call('createBLEConnection',{deviceId:id,timeout:10000})
      const chars=await characteristics(id)
      if(!chars.some(c=>c.uuid.toUpperCase()===KEY&&c.properties.write))throw new Error('未找到遥控写入特征')
      deviceId=id;paintReady=false;generation++;return id
    }catch(e){if(id)wx.closeBLEConnection({deviceId:id});throw e}
    finally{connecting=null;emit()}
  })();emit();return connecting
}
function write(characteristicId,bytes){
  const id=deviceId,token=generation
  const task=writeChain.catch(()=>{}).then(()=>{
    if(!id||id!==deviceId||token!==generation)throw new Error('蓝牙未连接')
    const value=Uint8Array.from(bytes).buffer
    return call('writeBLECharacteristicValue',{deviceId:id,serviceId:SERVICE,characteristicId,value,writeType:'write'})
  })
  writeChain=task;return task
}
async function enablePainting(){
  await connect();if(paintReady)return
  const id=deviceId,token=generation,chars=await characteristics(id)
  if(!chars.some(c=>c.uuid.toUpperCase()===WRITE&&c.properties.write)||!chars.some(c=>c.uuid.toUpperCase()===STATUS&&c.properties.notify))throw new Error('当前固件不支持绘画，请烧录新版固件')
  await call('notifyBLECharacteristicValueChange',{deviceId:id,serviceId:SERVICE,characteristicId:STATUS,state:true})
  if(id!==deviceId||token!==generation)throw new Error('连接已断开')
  paintReady=true
}
function disconnect(){const id=deviceId;deviceId='';paintReady=false;generation++;if(id)wx.closeBLEConnection({deviceId:id});emit();for(const f of statusListeners)f(null)}
function subscribe(f){listeners.add(f);f({connected:!!deviceId,connecting:!!connecting});return()=>listeners.delete(f)}
function onStatus(f){statusListeners.add(f);return()=>statusListeners.delete(f)}
module.exports={connect,write,enablePainting,disconnect,subscribe,onStatus}
