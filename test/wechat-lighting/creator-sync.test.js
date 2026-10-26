const {test}=require('node:test'),assert=require('node:assert/strict')
const fs=require('node:fs'),vm=require('node:vm'),path=require('node:path')
test('creator syncs on show and reconnect, and unsubscribes while hidden',()=>{
  let page,listener,writes=[],stops=0
  const connection={subscribe(fn){listener=fn;fn({connected:true});return()=>{listener=null;stops++}},write(key,bytes){writes.push([key,...bytes]);return Promise.resolve()}}
  const source=fs.readFileSync(path.resolve(__dirname,'../../wechat-mini-program/pages/creator/creator.js'),'utf8')
  vm.runInNewContext(source,{Page(p){page=p},require(name){return name.includes('connection')?connection:{KEY:'key'}}})
  page.onShow();assert.deepEqual(writes,[['key',6]])
  listener({connected:true});assert.equal(writes.length,1)
  listener({connected:false});listener({connected:true});assert.equal(writes.length,2)
  page.onHide();assert.equal(listener,null);assert.equal(stops,1)
  page.onUnload();assert.equal(stops,1)
  page.onShow();assert.equal(writes.length,3)
})
