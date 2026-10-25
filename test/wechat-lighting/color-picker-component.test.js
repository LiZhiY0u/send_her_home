const {test}=require('node:test'),assert=require('node:assert/strict')
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm'),{createRequire}=require('node:module')
function component(){
 const file=path.resolve(__dirname,'../../wechat-mini-program/components/color-picker/color-picker.js')
 let definition;vm.runInNewContext(fs.readFileSync(file,'utf8'),{require:createRequire(file),Component:value=>{definition=value},wx:{nextTick:()=>{}}})
 const events=[],instance={data:{...definition.data},properties:{visible:true,color:'#FF0000'},_ready:true,
  setData(patch){Object.assign(this.data,patch)},triggerEvent(name,detail){events.push({name,detail})}}
 Object.assign(instance,definition.methods);instance.drawWheel=()=>{};instance.measureWheel=()=>{};instance.initialize()
 return {instance,events,definition}
}
test('picker preserves partially typed HEX, rejects invalid RGB and emits only explicit confirmation',()=>{
 const {instance:p,events}=component()
 p.hexChanged({detail:{value:'#123'}});assert.equal(p.data.hexInput,'#123');assert.equal(p.data.hex,'#112233')
 p.hexChanged({detail:{value:'#123456'}});assert.equal(p.data.hex,'#123456')
 assert(events.every(e=>e.name==='preview'))
 p.rgbChanged({currentTarget:{dataset:{field:'rText'}},detail:{value:'256'}})
 assert(p.data.inputError);p.confirm();assert(!events.some(e=>e.name==='confirm'))
 p.hexChanged({detail:{value:'#ABCDEF'}});p.confirm()
 assert.equal(events.at(-1).name,'confirm');assert.equal(events.at(-1).detail.color,'#ABCDEF')
})
test('cancel blocks subsequent stale input and confirm events',()=>{
 const {instance:p,events}=component()
 p.cancel();const n=events.length,hex=p.data.hex
 p.hsvChanged({currentTarget:{dataset:{field:'h'}},detail:{value:120}})
 p.hexChanged({detail:{value:'#000000'}});p.confirm()
 assert.equal(events.length,n);assert.equal(p.data.hex,hex)
})
