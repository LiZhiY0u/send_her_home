const connection=require('../../ble/connection')
const {createMotorClient}=require('../../motor/client')
Page({
 data:{ready:false,busy:false,running:false,reverse:false,speed:30,draftSpeed:30,keepRunning:true,message:'连接后读取电机状态',error:''},
 onLoad(){
  this.alive=true
  this.client=createMotorClient(connection,state=>{
   if(!this.alive)return
   this.setData({...state,...(!this.dragging?{draftSpeed:state.speed}:{})})
  })
 },
 onShow(){this.visible=true;const token=this.showToken=(this.showToken||0)+1;Promise.resolve(this.leavePromise).then(()=>{if(this.visible&&this.showToken===token)this.connectMotor()})},
 async connectMotor(){
  if(this.opening){this.reopenRequested=true;return}
  if(!this.client||!this.visible)return
  this.opening=true;this.setData({error:''})
  try{await this.client.open()}catch(error){if(this.alive&&this.visible)this.setData({error:error.errMsg||error.message||'连接失败'})}
  finally{this.opening=false;if(this.reopenRequested&&this.visible){this.reopenRequested=false;this.connectMotor()}}
 },
 onHide(){this.visible=false;this.showToken++;this.dragging=false;this.leavePromise=this.client.leave().catch(()=>{})},
 onUnload(){this.alive=false;this.visible=false;this.client.leave().catch(()=>{})},
 async command(action){
  this.setData({error:''})
  try{await action()}catch(error){if(this.alive)this.setData({error:error.errMsg||error.message||'操作失败'})}
 },
 toggleRun(){if(this.data.ready&&!this.data.busy)this.command(()=>this.client.set({running:!this.data.running}))},
 stopMotor(){this.command(()=>this.client.stop())},
 direction(event){if(!this.data.running&&this.data.ready&&!this.data.busy)this.command(()=>this.client.set({reverse:event.currentTarget.dataset.reverse==='true'}))},
 speedChanging(event){this.dragging=true;this.setData({draftSpeed:event.detail.value})},
 speedChanged(event){this.dragging=false;this.command(()=>this.client.set({speed:Number(event.detail.value)}))},
 keepChanged(event){this.command(()=>this.client.set({keepRunning:!!event.detail.value}))}
})
