const {test}=require('node:test')
const assert=require('node:assert/strict')
const fs=require('node:fs'),path=require('node:path'),vm=require('node:vm')
const {createRequire}=require('node:module')
test('paint resize redraws existing draft with fresh bounds and resets stroke continuity',()=>{
 const file=path.resolve(__dirname,'../../wechat-mini-program/pages/paint/paint.js')
 let page,callback,scaled=0,drawn=0
 const canvas={getContext:()=>({scale:()=>scaled++})}
 const wx={nextTick:f=>f(),getWindowInfo:()=>({pixelRatio:2}),createSelectorQuery:()=>({select:()=>({fields:()=>({exec:cb=>{callback=cb}})})})}
 vm.runInNewContext(fs.readFileSync(file,'utf8'),{require:createRequire(file),Page:p=>{page=p},wx})
 const frame=new Uint8Array(399).fill(12)
 page.alive=true;page.frame=frame;page.previous={x:1,y:2};page.redraw=()=>drawn++
 page.onResize();callback([{node:canvas,width:260,height:260,left:30,top:10}])
 assert.equal(page.frame,frame);assert.equal(frame[0],12)
 assert.equal(page.previous,null);assert.equal(page.rect.left,30)
 assert.equal(canvas.width,520);assert.equal(scaled,1);assert.equal(drawn,1)
})
