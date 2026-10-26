const connection = require('../../ble/connection')
const { KEY } = require('../../painting/protocol')
Page({
  onShow() {
    this.stopCreatorSync()
    let wasConnected = false
    this.unsubscribe = connection.subscribe(state => {
      if (state.connected && !wasConnected) {
        connection.write(KEY, [6]).catch(() => {})
      }
      wasConnected = state.connected
    })
  },
  stopCreatorSync() {
    if (this.unsubscribe) this.unsubscribe()
    this.unsubscribe = null
  },
  onHide() { this.stopCreatorSync() },
  onUnload() { this.stopCreatorSync() }
})
