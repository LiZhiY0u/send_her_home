const { test } = require('node:test')
const assert = require('node:assert/strict')
const fs = require('node:fs')
const path = require('node:path')
const vm = require('node:vm')

test('page registers using JS-only local modules, before any Canvas or npm initialization', () => {
  const root = path.resolve(__dirname, '../../wechat-mini-program')
  const cache = new Map()
  let page
  function load(filename) {
    if (cache.has(filename)) return cache.get(filename).exports
    const module = { exports: {} }; cache.set(filename, module)
    const localRequire = request => {
      if (!request.startsWith('.')) throw new Error('npm loaded before Canvas initialization')
      // WeChat module resolution appends .js; it does not parse JSON like Node.
      const target = path.resolve(path.dirname(filename), request.endsWith('.js') ? request : request + '.js')
      return load(target)
    }
    const script = new vm.Script('(function(require,module,exports,Page){' + fs.readFileSync(filename, 'utf8') + '\n})', { filename })
    script.runInNewContext()(localRequire, module, module.exports, value => { page = value })
    return module.exports
  }
  load(path.join(root, 'pages/lighting/lighting.js'))
  assert(page && typeof page.onReady === 'function')
  assert.equal(page.data.layers.length, 3)
  assert.equal(page.data.effects.length, 22)
  assert.equal(page.data.linkModes.length, 4)
  const runtime = require('../../wechat-mini-program/lighting/led-map-133')
  const canonical = require('../../wechat-mini-program/lighting/led-map-133.json')
  assert.deepEqual(runtime, canonical)
  load(path.join(root, 'pages/paint/paint.js'))
  assert(page && typeof page.touchMove === 'function' && typeof page.synchronize === 'function')
  assert.equal(page.data.diameter,24)
  load(path.join(root, 'pages/remote/remote.js'))
  assert(page && typeof page.sendKey === 'function')
  load(path.join(root,'pages/motor/motor.js'))
  assert(page && typeof page.stopMotor==='function' && typeof page.keepChanged==='function')
})
