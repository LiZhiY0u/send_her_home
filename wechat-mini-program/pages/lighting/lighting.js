const { createLightingPage } = require('../../lighting/page-controller')
const { createScene } = require('../../lighting/scene')

Page(createLightingPage({
  connection: require('../../ble/connection'),
  query: callback => wx.createSelectorQuery().select('#light-scene').fields({ node: true, size: true, rect:true }).exec(callback),
  pixelRatio: () => wx.getWindowInfo ? wx.getWindowInfo().pixelRatio : wx.getSystemInfoSync().pixelRatio,
  now: () => Date.now(),
  nextTick: callback => wx.nextTick(callback),
  openPaint: () => wx.navigateTo({url:'/pages/paint/paint'}),
  createView: (canvas, points, options) => {
    // Lazy import allows a readable error when npm has not been built.
    const { createScopedThreejs } = require('threejs-miniprogram')
    return createScene(canvas, createScopedThreejs(canvas), points, options)
  }
}))
