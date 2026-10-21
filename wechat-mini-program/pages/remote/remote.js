const connection = require('../../ble/connection')
const { KEY } = require('../../painting/protocol')
Page({
  data: { connected:false, connecting:false, statusText:'未连接' },
  onLoad() {
    this.unsubscribe=connection.subscribe(state=>this.setData({...state,statusText:state.connecting?'正在连接…':state.connected?'已连接':'未连接'}))
  },
  onUnload() { if(this.unsubscribe)this.unsubscribe() },
  async toggleConnection() {
    if(this.data.connected){connection.disconnect();return}
    try{await connection.connect()}catch(error){wx.showToast({title:error.errMsg||error.message||'连接失败',icon:'none'})}
  },
  async sendKey(event) {
    if(!this.data.connected){wx.showToast({title:'请先连接设备',icon:'none'});return}
    try{await connection.write(KEY,[Number(event.currentTarget.dataset.code)])}
    catch(error){wx.showToast({title:error.errMsg||error.message||'发送失败',icon:'none'})}
  }
})
