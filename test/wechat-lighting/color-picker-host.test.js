const {test}=require('node:test'),assert=require('node:assert/strict')
const {createLightingPage}=require('../../wechat-mini-program/lighting/page-controller')
function host(){const p=createLightingPage({});p.data={...p.data,color:'#57d9ff',randomColor:true,deviceLinked:true};p.setData=v=>Object.assign(p.data,v);p.redraw=()=>{};return p}
test('host previews locally, restores on cancel and transmits only on Apply',()=>{
 const p=host();let sends=0;p.applyDeviceEffect=()=>{sends++}
 p.openColorPicker();p.previewPickerColor({detail:{color:'#123456'}})
 assert.equal(sends,0);assert.equal(p.data.randomColor,false)
 p.cancelColorPicker();assert.equal(p.data.color,'#57d9ff');assert(p.data.randomColor)
 p.previewPickerColor({detail:{color:'#000000'}});assert.equal(p.data.color,'#57d9ff')
 p.openColorPicker();p.confirmPickerColor({detail:{color:'#ABCDEF'}})
 assert.equal(sends,1);assert.equal(p.data.color,'#ABCDEF');assert(!p.data.randomColor)
})
test('pending device update retains committed settings while picker previews another color',async()=>{
 const p=host();p._visible=true;p._alive=true
 const sent=[],release=[];p._deviceSender={applyEffect:settings=>{sent.push(settings.color);return new Promise(resolve=>release.push(resolve))}}
 const task=p.applyDeviceEffect();p.data.color='#445566';p.applyDeviceEffect()
 p.openColorPicker();p.previewPickerColor({detail:{color:'#ABCDEF'}})
 release[0]();for(let i=0;i<10&&sent.length<2;++i)await Promise.resolve()
 assert.deepEqual(sent,['#57d9ff','#445566']);release[1]();await task
})
