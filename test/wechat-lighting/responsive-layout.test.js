const {test}=require('node:test')
const assert=require('node:assert/strict')
const fs=require('node:fs')
const path=require('node:path')
const {createLightingPage}=require('../../wechat-mini-program/lighting/page-controller')
test('linked effect choices send device updates but preview-only region selection does not',()=>{
 const page=createLightingPage({});page.data={...page.data,deviceLinked:true}
 page.setData=patch=>Object.assign(page.data,patch);page.redraw=()=>{}
 let calls=0;page.applyDeviceEffect=()=>{calls++}
 page.choose({currentTarget:{dataset:{field:'effect',value:'squareMove'}}})
 page.choose({currentTarget:{dataset:{field:'region',value:'CENTER'}}})
 page.slider({type:'changing',currentTarget:{dataset:{field:'speed'}},detail:{value:40}})
 page.slider({type:'change',currentTarget:{dataset:{field:'speed'}},detail:{value:40}})
 assert.equal(calls,2)
})
test('resize refreshes canvas bounds without resetting settings or recreating renderer',()=>{
  let callback, size
  const page=createLightingPage({query:cb=>{callback=cb}})
  page._alive=true;page._visible=true
  page._view={resize:(...v)=>{size=v}}
  page.redraw=()=>{}
  page.data.effect='squareMove'
  page.onResize()
  callback([{width:500,height:280,left:20,top:40}])
  assert.deepEqual(size,[500,280])
  assert.equal(page._canvasRect.left,20)
  assert.equal(page.data.effect,'squareMove')
})
test('3D page enables rotation and scrolls controls rather than whole page',()=>{
  const root=path.resolve(__dirname,'../../wechat-mini-program/pages/lighting')
  const config=JSON.parse(fs.readFileSync(path.join(root,'lighting.json'),'utf8'))
  assert.equal(config.pageOrientation,'auto')
  assert.equal(config.disableScroll,true)
  const wxml=fs.readFileSync(path.join(root,'lighting.wxml'),'utf8')
  assert(wxml.includes('<scroll-view class="controls-scroll" scroll-y'))
  assert(wxml.includes('class="preview-pane"'))
})
