const {test}=require('node:test'),assert=require('node:assert/strict')
const P=require('../../wechat-mini-program/painting/protocol')
test('shared connection preserves key writes, discovers painting and serializes operations',async()=>{
  let found,changed,active=0,maxActive=0;const writes=[]
  const succeed=value=>args=>queueMicrotask(()=>args.success(value))
  global.wx={
    onBLEConnectionStateChange:f=>{changed=f},onBLECharacteristicValueChange:()=>{},
    openBluetoothAdapter:succeed({}),onBluetoothDeviceFound:f=>{found=f},offBluetoothDeviceFound:()=>{},
    startBluetoothDevicesDiscovery:()=>queueMicrotask(()=>found({devices:[{name:'FigureCase',deviceId:'device'}]})),stopBluetoothDevicesDiscovery:()=>{},
    createBLEConnection:succeed({}),closeBLEConnection:()=>{},
    getBLEDeviceServices:succeed({services:[{uuid:P.SERVICE}]}),
    getBLEDeviceCharacteristics:succeed({characteristics:[{uuid:P.KEY,properties:{write:true}},{uuid:P.WRITE,properties:{write:true}},{uuid:P.STATUS,properties:{notify:true}}]}),
    notifyBLECharacteristicValueChange:succeed({}),
    writeBLECharacteristicValue:args=>{active++;maxActive=Math.max(maxActive,active);writes.push(args);queueMicrotask(()=>{active--;args.success({})})}
  }
  const connection=require('../../wechat-mini-program/ble/connection')
  await connection.connect();await connection.enablePainting()
  await Promise.all([connection.write(P.KEY,[5]),connection.write(P.WRITE,[0xa7,1])])
  assert.equal(maxActive,1);assert.equal(writes[0].characteristicId,P.KEY);assert.equal(new Uint8Array(writes[0].value)[0],5)
  changed({deviceId:'device',connected:false});await assert.rejects(connection.write(P.KEY,[1]))
  delete global.wx
})
