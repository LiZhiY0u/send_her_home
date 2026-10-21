const { test } = require('node:test')
const assert = require('node:assert/strict')
const { createLightingPage } = require('../../wechat-mini-program/lighting/page-controller')
function fixture(overrides = {}) {
  let callback, disposed = 0, renders = 0, frames = 0
  const canvas = { getContext: () => ({}), requestAnimationFrame: () => ++frames, cancelAnimationFrame() {} }
  const deps = { query: cb => { callback = cb }, pixelRatio: () => 1,
    createView: () => ({ render() { renders++ }, dispose() { disposed++ }, resize() {}, setView() {}, touchStart() {}, touchMove() {}, touchEnd() {} }), now: () => 0, ...overrides }
  const page = createLightingPage(deps)
  page.data = JSON.parse(JSON.stringify(page.data)); page.setData = patch => Object.assign(page.data, patch)
  return { page, canvas, ready: () => callback([{ node: canvas, width: 320, height: 400 }]),
    stats: () => ({ disposed, renders, frames }) }
}
test('late canvas after unload never creates a scene or animation', () => {
  const f = fixture(); f.page.onLoad(); f.page.onShow(); f.page.onReady(); f.page.onUnload(); f.ready()
  assert.deepEqual(f.stats(), { disposed: 0, renders: 0, frames: 0 })
})
test('missing canvas and WebGL failure give an actionable error', () => {
  const f = fixture({ createView() { throw new Error('GPU unavailable') } })
  f.page.onLoad(); f.page.onShow(); f.page.onReady(); f.ready()
  assert.match(f.page.data.error, /GPU unavailable/)
  assert.equal(f.stats().frames, 0)
  const g = fixture({ query: cb => cb([]) }); g.page.onLoad(); g.page.onReady()
  assert.match(g.page.data.error, /Canvas/)
})
test('controls stay local, redraw paused scenes and release once', () => {
  const f = fixture(); f.page.onLoad(); f.page.onShow(); f.page.onReady(); f.ready()
  f.page.togglePlayback(); assert.equal(f.page.data.playing, false)
  f.page.choose({ currentTarget: { dataset: { field: 'layer', value: 'top' } } })
  assert.equal(f.page.data.layer, 'top'); assert(f.stats().renders > 0)
  f.page.slider({ currentTarget: { dataset: { field: 'brightness' } }, detail: { value: 200 } })
  assert.equal(f.page.data.brightness, 100)
  f.page.choose({ currentTarget: { dataset: { field: 'bad', value: 'bad' } } })
  assert.equal(f.page.data.bad, undefined)
  f.page.onHide(); f.page.onUnload(); f.page.onUnload(); assert.equal(f.stats().disposed, 1)
})
