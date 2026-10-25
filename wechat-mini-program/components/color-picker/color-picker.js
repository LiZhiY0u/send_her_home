const C=require('../../color/color-math')
Component({
 properties:{visible:{type:Boolean,value:false},color:{type:String,value:'#57D9FF'}},
 data:{h:0,s:100,v:100,hText:0,sText:100,vText:100,hex:'#57D9FF',hexInput:'#57D9FF',rText:'87',gText:'217',bText:'255',rgbExpanded:false,inputError:'',wheelError:''},
 observers:{visible(value){if(value&&this._ready)this.initialize();else if(!value){this._closing=true;this._generation=(this._generation||0)+1;this._context=null;this._rect=null}}},
 lifetimes:{
  ready(){this._ready=true;if(this.properties.visible)this.initialize()},
  detached(){this._ready=false;this._generation=(this._generation||0)+1;this._context=null;this._rect=null}
 },
 pageLifetimes:{resize(){if(this.properties.visible)this.measureWheel()}},
 methods:{
  swallow(){},
  initialize(){
   this._closing=false
   const rgb=C.hexToRgb(this.properties.color)||[87,217,255]
   const hsv=C.rgbToHsv(...rgb)
   this.setData({...hsv,hText:Math.round(hsv.h),sText:Math.round(hsv.s),vText:Math.round(hsv.v),hex:C.rgbToHex(rgb),hexInput:C.rgbToHex(rgb),rText:String(rgb[0]),gText:String(rgb[1]),bText:String(rgb[2]),inputError:'',wheelError:''})
   this.measureWheel()
  },
  measureWheel(){
   const generation=this._generation=(this._generation||0)+1
   wx.nextTick(()=>{
    if(!this._ready||!this.properties.visible||generation!==this._generation)return
    this.createSelectorQuery().select('#color-wheel').fields({node:true,size:true,rect:true}).exec(result=>{
     if(!this._ready||!this.properties.visible||generation!==this._generation)return
     try{
      const item=result&&result[0]
      if(!item||!item.node||!(item.width>0&&item.height>0))throw new Error('色盘不可用，可使用滑条或色值输入')
      const ratio=Math.max(1,Math.min(2,(wx.getWindowInfo?wx.getWindowInfo():wx.getSystemInfoSync()).pixelRatio||1))
      const canvas=item.node;canvas.width=Math.round(item.width*ratio);canvas.height=Math.round(item.height*ratio)
      this._context=canvas.getContext('2d');this._context.scale(ratio,ratio);this._rect=item;this.drawWheel()
     }catch(error){this.setData({wheelError:error.message})}
    })
   })
  },
  drawWheel(){
   if(!this._context||!this._rect)return
   const ctx=this._context,size=this._rect.width,center=size/2,radius=center-2
   ctx.clearRect(0,0,size,size)
   for(let h=0;h<360;h+=2){
    ctx.beginPath();ctx.moveTo(center,center)
    ctx.arc(center,center,radius,-(h+2)*Math.PI/180,-h*Math.PI/180)
    ctx.closePath();ctx.fillStyle=C.rgbToHex(C.hsvToRgb(h,100,100));ctx.fill()
   }
   const gradient=ctx.createRadialGradient(center,center,0,center,center,radius)
   gradient.addColorStop(0,'rgba(255,255,255,1)');gradient.addColorStop(1,'rgba(255,255,255,0)')
   ctx.fillStyle=gradient;ctx.beginPath();ctx.arc(center,center,radius,0,Math.PI*2);ctx.fill()
   const angle=this.data.h*Math.PI/180,r=this.data.s/100*radius
   const x=center+Math.cos(angle)*r,y=center-Math.sin(angle)*r
   ctx.beginPath();ctx.arc(x,y,6,0,Math.PI*2);ctx.strokeStyle='#202030';ctx.lineWidth=3;ctx.stroke()
   ctx.beginPath();ctx.arc(x,y,6,0,Math.PI*2);ctx.strokeStyle='#ffffff';ctx.lineWidth=1.5;ctx.stroke()
  },
  update(hsv){
   if(!this._ready||!this.properties.visible||this._closing)return
   const next={h:this.data.h,s:this.data.s,v:this.data.v,...hsv}
   const rgb=C.hsvToRgb(next.h,next.s,next.v),hex=C.rgbToHex(rgb)
   this.setData({...next,hText:Math.round(next.h),sText:Math.round(next.s),vText:Math.round(next.v),hex,hexInput:hex,rText:String(rgb[0]),gText:String(rgb[1]),bText:String(rgb[2]),inputError:''})
   this.drawWheel();this.triggerEvent('preview',{color:hex})
  },
  touchWheel(event){
   if(this._closing||!this.properties.visible)return
   if(!event.touches||event.touches.length!==1)return
   const point=C.wheelPoint(event.touches[0],this._rect)
   if(point)this.update(point)
  },
  hsvChanged(event){if(this._closing||!this.properties.visible)return;const field=event.currentTarget.dataset.field,value=Number(event.detail.value);if(['h','s','v'].includes(field)&&Number.isFinite(value))this.update({[field]:value})},
  hexChanged(event){
   if(this._closing||!this.properties.visible)return
   const text=event.detail.value;this.setData({hexInput:text})
   const rgb=C.hexToRgb(text)
   if(!rgb){this.setData({inputError:'请输入有效 HEX，如 #FF7096'});return}
   this.update(C.rgbToHsv(...rgb))
   this.setData({hexInput:text})
  },
  rgbChanged(event){
   if(this._closing||!this.properties.visible)return
   const field=event.currentTarget.dataset.field
   if(!['rText','gText','bText'].includes(field))return
   const values={rText:this.data.rText,gText:this.data.gText,bText:this.data.bText,[field]:event.detail.value}
   this.setData(values)
   if(Object.values(values).some(v=>!/^\d{1,3}$/.test(v)||Number(v)>255)){this.setData({inputError:'RGB 每项需为 0–255 整数'});return}
   this.update(C.rgbToHsv(Number(values.rText),Number(values.gText),Number(values.bText)))
  },
  toggleRgb(){this.setData({rgbExpanded:!this.data.rgbExpanded})},
  cancel(){if(this._closing||!this.properties.visible)return;this._closing=true;this.triggerEvent('cancel')},
  confirm(){if(!this._closing&&this.properties.visible&&!this.data.inputError){this._closing=true;this.triggerEvent('confirm',{color:this.data.hex})}}
 }
})
