const { buildLayout } = require('./layout')
const source = require('./led-map-133')
const { createPreviewLoop } = require('./preview-loop')
const { getDraft } = require('../painting/draft')
const { EFFECTS, LINK_MODES } = require('./effects')
const { createPaintSender } = require('../painting/sender')
const {normalizeHex}=require('../color/color-math')
const {createLightingStateClient}=require('./device-state')
const choices = {
  layer: ['both', 'top', 'bottom'], region: ['all', 'CENTER', 'corners', 'UR', 'LR', 'LL', 'UL'],
  effect: EFFECTS.map(item => item.value), linkedMode: LINK_MODES.map(item => item.value),
  color: ['#57d9ff', '#a78bfa', '#ff7096', '#ffc46b', '#e9f4ff', '#66e3a4']
}
function createLightingPage(deps) {
  function fail(page, error) {
    if (!page._alive) return
    if (page._loop) page._loop.dispose()
    if (page._view) page._view.dispose()
    page._loop = null; page._view = null
    page.setData({ error: `3D 预览不可用：${error.message || error}。请返回后重试，或检查构建 npm。` })
  }
  return {
    data: {
      error: '', ready: false, playing: true, layer: 'both', region: 'all', effect: 'rotate',
      deviceBusy:false,deviceLinked:false,deviceStatus:'预览不会自动控制设备',
      randomColor:false,
      colorPickerVisible:false,
      color: '#57d9ff', brightness: 65, speed: 50, linkedMode: 'sync',
      layers: [{ value: 'both', label: '上下联动' }, { value: 'top', label: '顶部' }, { value: 'bottom', label: '底部' }],
      regions: [{ value: 'all', label: '全部' }, { value: 'CENTER', label: '中心' }, { value: 'corners', label: '四角' },
        { value: 'UL', label: '左上' }, { value: 'UR', label: '右上' }, { value: 'LL', label: '左下' }, { value: 'LR', label: '右下' }],
      effects: EFFECTS, linkModes: LINK_MODES,
      colors: choices.color
    },
    onLoad() { this._alive = true; this._visible = false
      if(deps.connection)this._deviceSender=createPaintSender(deps.connection,text=>{if(this._alive)this.setData({deviceStatus:text})})
      if(deps.connection&&deps.connection.subscribe){
        this._stateClient=createLightingStateClient(deps.connection)
        this._unsubscribeConnection=deps.connection.subscribe(state=>{
          this._connected=state.connected
          if(!state.connected){this._deviceClock=null;this.setData({deviceLinked:false,deviceStatus:state.connecting?'正在连接…':'蓝牙未连接 · 本地预览'})}
          else if(this._visible)this.readDeviceState()
        })
        this._stateTimer=setInterval(()=>{if(this._visible&&this._connected)this.readDeviceState()},500)
      }
    },
    async readDeviceState(){
      if(!this._stateClient||this._stateReading||this._localEditing||this.data.deviceBusy||this.data.colorPickerVisible||Date.now()<(this._ignoreStateUntil||0))return
      this._stateReading=true
      try{
        const state=await this._stateClient.read()
        if(!this._alive||!this._visible||this._localEditing||this.data.deviceBusy||this.data.colorPickerVisible)return
        if(state.mode===3||state.mode===2){
          this._paintFrame=null
          this._deviceAuto=state.mode===2
          this._deviceClock={time:state.elapsed,at:deps.now?deps.now():Date.now()}
          this.setData({effect:EFFECTS[state.effect].value,speed:state.speed,brightness:state.brightness,color:state.color,randomColor:state.randomColor,
            layer:state.mode===2?'both':state.layer,region:state.mode===2?'all':state.region,linkedMode:state.mode===2?'sync':state.linkedMode,
            deviceStatus:state.mode===2?'设备正在自动随机灯效':'设备灯光状态已同步'})
        }else {this._deviceClock=null;this._deviceAuto=false;this.setData({brightness:state.brightness,deviceStatus:state.mode===4?'设备正在显示绘画':state.mode===1?'设备正在播放白光启动动画':'设备待唤醒'})}
        if(this.data.playing)this.redraw()
      }catch(error){if(this._alive&&this._visible&&this._connected)this.setData({deviceStatus:error.message||'状态读取失败'})}
      finally{this._stateReading=false}
    },
    async applyDeviceEffect(){
      if(!this._deviceSender)return
      this._deviceEffectSettings={...this.data}
      this._paintFrame=null
      if(this.data.deviceBusy){this._effectPending=true;return}
      this.setData({deviceBusy:true,deviceStatus:'正在连接并应用…'})
      try{
        if(this._stateClient)await this._stateClient.idle()
        do{this._effectPending=false;await this._deviceSender.applyEffect(this._deviceEffectSettings)}
        while(this._effectPending&&this._alive&&this._visible)
        if(this._alive&&this._visible){this._localEditing=false;this._deviceAuto=false;this._deviceClock=null;this._ignoreStateUntil=Date.now()+100;this.setData({deviceLinked:true})}
      }
      catch(error){if(this._alive)this.setData({deviceLinked:false,deviceStatus:error.errMsg||error.message||'应用失败'})}
      finally{if(this._alive)this.setData({deviceBusy:false})}
    },
    onReady() {
      deps.query(result => {
        if (!this._alive) return
        try {
          const entry = result && result[0]
          if (!entry || !entry.node || !entry.width || !entry.height) throw new Error('Canvas 节点获取失败')
          const canvas = entry.node
          this._canvasRect=entry
          const points = buildLayout(source)
          this._view = deps.createView(canvas, points, { width: entry.width, height: entry.height, pixelRatio: deps.pixelRatio() })
          this._loop = createPreviewLoop({
            requestFrame: callback => canvas.requestAnimationFrame(callback),
            cancelFrame: id => canvas.cancelAnimationFrame(id), now: deps.now,
            render: time => { try { const clock=this._deviceClock;const elapsed=clock?clock.time+((deps.now?deps.now():Date.now())-clock.at)/1000:time;this._view.render({...this.data,...(this._deviceAuto?{randomColor:false}:{}),paintFrame:this._paintFrame}, elapsed) } catch (error) { fail(this, error) } }
          })
          this.setData({ ready: true })
          this._loop.draw()
          if (this._loop && this._visible && this.data.playing) this._loop.start()
        } catch (error) { fail(this, error) }
      })
    },
    onShow() { this._localEditing=false;const draft=getDraft();this._paintFrame=draft.painted?draft.frame:null;this._visible = true; if (this._loop && this.data.playing) this._loop.start();this.redraw();if(this._connected)this.readDeviceState() },
    onHide() { if(this.data.colorPickerVisible)this.cancelColorPicker();this._visible = false;this._effectPending=false; if(this._deviceSender)this._deviceSender.stop();this.setData({deviceBusy:false,deviceLinked:false}); if (this._loop) this._loop.stop() },
    onUnload() {
      if(this._stateTimer)clearInterval(this._stateTimer)
      if(this._unsubscribeConnection)this._unsubscribeConnection()
      if(this._stateClient)this._stateClient.dispose()
      this._alive = false; this._visible = false
      if(this._deviceSender)this._deviceSender.stop()
      if (this._loop) this._loop.dispose()
      if (this._view) this._view.dispose()
      this._loop = null; this._view = null
    },
    onResize() {
      if (!this._view) return
      const update = () => deps.query(result => {
        if (!this._alive || !this._view || !result || !result[0]) return
        if (!(result[0].width > 0 && result[0].height > 0)) return
        this._canvasRect = result[0]
        this._view.resize(result[0].width, result[0].height); this.redraw()
      })
      if (deps.nextTick) deps.nextTick(update)
      else update()
    },
    redraw() { if (this._loop && this._visible) this._loop.draw() },
    openColorPicker(){
      this._dirtyBeforePicker=this._localEditing
      this._localEditing=true
      this._deviceAuto=false
      this._colorBeforePicker={color:this.data.color,randomColor:this.data.randomColor}
      this.setData({colorPickerVisible:true})
    },
    previewPickerColor(event){
      if(!this.data.colorPickerVisible)return
      const color=normalizeHex(event.detail.color);if(!color)return
      this.setData({color,randomColor:false});this.redraw()
    },
    cancelColorPicker(){
      this._localEditing=this._dirtyBeforePicker
      this.setData({...(this._colorBeforePicker||{}),colorPickerVisible:false},()=>this.redraw())
    },
    confirmPickerColor(event){
      if(!this.data.colorPickerVisible)return
      const color=normalizeHex(event.detail.color);if(!color)return
      this.setData({color,randomColor:false,colorPickerVisible:false},()=>this.redraw())
      if(this.data.deviceLinked)this.applyDeviceEffect()
    },
    choose(event) {
      const { field, value } = event.currentTarget.dataset
      if (!choices[field] || !choices[field].includes(value)) return
      this._localEditing=true;this._deviceAuto=false
      if(field === 'effect') this._paintFrame = null
      this.setData({ [field]: value,...(field==='color'?{randomColor:false}:{}) }); this.redraw()
      this._deviceClock=null
      if(this.data.deviceLinked)this.applyDeviceEffect()
    },
    slider(event) {
      const field = event.currentTarget.dataset.field
      if (!['brightness', 'speed'].includes(field)) return
      const value = Number(event.detail.value)
      if (!Number.isFinite(value)) return
      this._localEditing=true;this._deviceAuto=false;this._deviceClock=null
      this.setData({ [field]: Math.max(0, Math.min(100, value)) }); this.redraw()
      if(this.data.deviceLinked&&event.type!=='changing')this.applyDeviceEffect()
    },
    toggleRandomColor(event){
      this._localEditing=true;this._deviceAuto=false;this._deviceClock=null
      this.setData({randomColor:!!event.detail.value});this.redraw()
      if(this.data.deviceLinked)this.applyDeviceEffect()
    },
    togglePlayback() {
      this.setData({ playing: !this.data.playing })
      if (!this._loop) return
      if (this.data.playing && this._visible) this._loop.start()
      else this._loop.stop()
      this.redraw()
    },
    setView(event) { if (this._view) { this._view.setView(event.currentTarget.dataset.view); this.redraw() } },
    tapScene(event) {
      if(!this._view||!this._view.hitBottom||this._dragged||!deps.openPaint)return
      const p=event.detail||{},rect=this._canvasRect||{}
      const x=Number.isFinite(p.x)?p.x- (rect.left||0):NaN
      const y=Number.isFinite(p.y)?p.y- (rect.top||0):NaN
      if(this._view.hitBottom(x,y))deps.openPaint()
    },
    touchStart(event) { this._dragged=false;if (this._view) this._view.touchStart(event) },
    touchMove(event) { this._dragged=true;if (this._view) { this._view.touchMove(event); this.redraw() } },
    touchEnd(event) { if (this._view) this._view.touchEnd(event) }
  }
}
module.exports = { createLightingPage }
