const connection=require('../../ble/connection')
const {points}=require('../../painting/physical-map')
const {paintSegment}=require('../../painting/brush')
const {toWorld,drawBoard}=require('../../painting/canvas')
const {createPaintSender}=require('../../painting/sender')
const {getDraft,saveDraft}=require('../../painting/draft')
Page({
  data:{status:'本地草稿 · 尚未同步设备',diameter:24,color:'#ff5e93',eraser:false,busy:false,error:'',
    colors:['#ff5e93','#57d9ff','#ffc46b','#a78bfa','#e9f4ff','#66e3a4']},
  onLoad(){
    this.alive=true;this.frame=getDraft().frame
    this.sender=createPaintSender(connection,status=>{if(this.alive)this.setData({status})})
  },
  onReady(){this.resizeBoard()},
  onResize(){this.previous=null;this.rect=null;wx.nextTick(()=>{if(this.alive)this.resizeBoard()})},
  resizeBoard(){
    const generation=this.resizeGeneration=(this.resizeGeneration||0)+1
    wx.createSelectorQuery().select('#paint-board').fields({node:true,size:true,rect:true}).exec(result=>{
      if(!this.alive||generation!==this.resizeGeneration)return
      try{
        const item=result[0];if(!item||!item.node||!(item.width>0&&item.height>0))throw new Error('Canvas 2D 不可用')
        this.canvas=item.node;this.context=this.canvas.getContext('2d');this.rect=item
        const ratio=Math.min(2,(wx.getWindowInfo?wx.getWindowInfo():wx.getSystemInfoSync()).pixelRatio||1)
        this.canvas.width=item.width*ratio;this.canvas.height=item.height*ratio
        this.context.scale(ratio,ratio);this.redraw()
      }catch(error){this.setData({error:error.message})}
    })
  },
  onHide(){this.previous=null;saveDraft(this.frame);this.sender.stop();this.setData({status:'同步已暂停 · 灯板保留最后画面',busy:false})},
  onUnload(){this.alive=false;this.previous=null;saveDraft(this.frame);this.sender.stop();this.context=null;this.canvas=null},
  redraw(){if(this.context&&this.rect)drawBoard(this.context,points,this.frame,this.rect.width)},
  stroke(point){
    if(!point){this.previous=null;return}
    const color=this.data.eraser?[0,0,0]:[1,3,5].map(i=>parseInt(this.data.color.slice(i,i+2),16))
    paintSegment(this.frame,points,this.previous||point,point,{diameter:this.data.diameter,color})
    this.previous=point;saveDraft(this.frame);this.redraw();this.sender.submit(this.frame)
  },
  touchStart(event){
    this.previous=null;if(!this.rect||event.touches.length!==1)return
    this.stroke(toWorld(event.touches[0],this.rect))
  },
  touchMove(event){
    if(!this.rect||event.touches.length!==1){this.previous=null;return}
    this.stroke(toWorld(event.touches[0],this.rect))
  },
  touchEnd(){this.previous=null},
  chooseColor(event){this.setData({color:event.currentTarget.dataset.color,eraser:false})},
  setWidth(event){this.setData({diameter:Math.max(10,Math.min(60,Number(event.detail.value)))})},
  toggleEraser(){this.setData({eraser:!this.data.eraser})},
  clearBoard(){this.frame.fill(0);saveDraft(this.frame);this.redraw();this.sender.submit(this.frame)},
  async synchronize(){
    if(this.data.busy)return;this.setData({busy:true})
    try{await this.sender.start(this.frame)}
    catch(error){if(this.alive)this.setData({status:error.errMsg||error.message||'连接失败'})}
    finally{if(this.alive)this.setData({busy:false})}
  },
  async restoreAuto(){
    if(this.data.busy)return;this.setData({busy:true})
    try{await this.sender.restoreAuto()}
    catch(error){if(this.alive)this.setData({status:error.errMsg||error.message||'恢复失败'})}
    finally{if(this.alive)this.setData({busy:false})}
  }
})
