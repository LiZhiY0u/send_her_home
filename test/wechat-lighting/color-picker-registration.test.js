const {test}=require('node:test')
const assert=require('node:assert/strict')
const fs=require('node:fs')
const path=require('node:path')

test('both picker hosts resolve the component relative to their page and hide native canvases under it',()=>{
  for(const name of ['lighting','paint']){
    const base=path.resolve(__dirname,`../../wechat-mini-program/pages/${name}/${name}`)
    const registration=JSON.parse(fs.readFileSync(base+'.json','utf8')).usingComponents['color-picker']
    assert.equal(registration,'../../components/color-picker/color-picker')
    const component=path.resolve(path.dirname(base),registration)
    assert.equal(JSON.parse(fs.readFileSync(component+'.json','utf8')).component,true)
    assert.match(fs.readFileSync(component+'.wxml','utf8'),/id="color-wheel"/)
    assert.match(fs.readFileSync(base+'.wxml','utf8'),/<canvas[^>]*hidden="\{\{colorPickerVisible\}\}"/)
  }
})
