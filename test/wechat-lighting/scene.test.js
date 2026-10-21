const { test } = require('node:test')
const assert = require('node:assert/strict')
const { createScopedThreejs } = require('../../wechat-mini-program/node_modules/threejs-miniprogram')
const { createScene } = require('../../wechat-mini-program/lighting/scene')
const { buildLayout } = require('../../wechat-mini-program/lighting/layout')
const source = require('../../wechat-mini-program/lighting/led-map-133.json')
test('real geometry holds 266 LED colors; sizing capped; dispose is idempotent', () => {
  const THREE = createScopedThreejs({})
  const calls = []; let scene
  class Renderer {
    setClearColor(v) { calls.push(['background', v]) } setPixelRatio(v) { calls.push(['ratio', v]) }
    setSize(w, h) { calls.push(['size', w, h]) }
    render(s) { scene = s; calls.push(['render']) }
    dispose() { calls.push(['dispose']) }
  }
  const view = createScene({}, { ...THREE, WebGLRenderer: Renderer }, buildLayout(source), { width: 320, height: 400, pixelRatio: 3 })
  view.render({ layer: 'both', region: 'all', effect: 'solid', color: '#ff0000', brightness: 100 }, 0)
  const points = scene.children.find(c => c.isPoints)
  assert.equal(points.geometry.attributes.position.count, 266)
  assert.equal(points.geometry.attributes.color.array[0], 1)
  assert(calls.some(c => c[0] === 'background' && c[1] === 0x20242d))
  assert(calls.some(c => c[0] === 'ratio' && c[1] === 2))
  view.setView('top');view.render({},0);assert(view.hitBottom(160,200))
  view.resize(0, 0); assert(calls.some(c => c[0] === 'size' && c[1] === 1 && c[2] === 1))
  view.dispose(); view.dispose(); const n = calls.length
  view.render({}, 1); assert.equal(calls.length, n)
  assert.equal(calls.filter(c => c[0] === 'dispose').length, 1)
})
