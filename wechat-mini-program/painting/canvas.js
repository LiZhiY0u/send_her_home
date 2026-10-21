function toWorld(touch,rect){
  const x=Number.isFinite(touch.x)?touch.x:touch.clientX-rect.left
  const y=Number.isFinite(touch.y)?touch.y:touch.clientY-rect.top
  if(!Number.isFinite(x)||!Number.isFinite(y)||x<0||y<0||x>rect.width||y>rect.height)return null
  return {x:(x/rect.width-0.5)*200,y:(0.5-y/rect.height)*200}
}
function drawBoard(ctx,points,frame,size){
  ctx.clearRect(0,0,size,size);ctx.fillStyle='#091522';ctx.fillRect(0,0,size,size)
  ctx.strokeStyle='#243c4e';ctx.lineWidth=1
  for(let n=0;n<=10;n++){const p=n*size/10;ctx.beginPath();ctx.moveTo(p,0);ctx.lineTo(p,size);ctx.moveTo(0,p);ctx.lineTo(size,p);ctx.stroke()}
  points.forEach((p,i)=>{
    const r=frame[i*3],g=frame[i*3+1],b=frame[i*3+2],on=r||g||b
    ctx.fillStyle=on?`rgb(${r},${g},${b})`:'#2e4658';ctx.beginPath()
    ctx.arc((p.x/200+0.5)*size,(0.5-p.y/200)*size,Math.max(2.8,size*0.009),0,Math.PI*2);ctx.fill()
  })
}
module.exports={toWorld,drawBoard}
