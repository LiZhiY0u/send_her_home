const clamp=(v,min,max)=>Math.max(min,Math.min(max,Number.isFinite(v)?v:min))
function normalizeHex(value){
 let text=String(value||'').trim().replace(/^#/,'')
 if(/^[0-9a-f]{3}$/i.test(text))text=text.split('').map(c=>c+c).join('')
 return /^[0-9a-f]{6}$/i.test(text)?'#'+text.toUpperCase():null
}
function hexToRgb(value){const hex=normalizeHex(value);return hex?[1,3,5].map(i=>parseInt(hex.slice(i,i+2),16)):null}
function rgbToHex(rgb){return '#'+rgb.map(v=>Math.round(clamp(v,0,255)).toString(16).padStart(2,'0')).join('').toUpperCase()}
function hsvToRgb(h,s,v){
 h=((Number.isFinite(h)?h:0)%360+360)%360;s=clamp(s,0,100)/100;v=clamp(v,0,100)/100
 const c=v*s,x=c*(1-Math.abs(h/60%2-1)),m=v-c
 const parts=h<60?[c,x,0]:h<120?[x,c,0]:h<180?[0,c,x]:h<240?[0,x,c]:h<300?[x,0,c]:[c,0,x]
 return parts.map(n=>Math.round((n+m)*255))
}
function rgbToHsv(r,g,b){
 r=clamp(r,0,255)/255;g=clamp(g,0,255)/255;b=clamp(b,0,255)/255
 const max=Math.max(r,g,b),min=Math.min(r,g,b),d=max-min
 let h=0
 if(d)h=60*(max===r?((g-b)/d%6):max===g?(b-r)/d+2:(r-g)/d+4)
 return {h:(h+360)%360,s:max?d/max*100:0,v:max*100}
}
function wheelPoint(touch,rect){
 if(!touch||!rect||!(rect.width>0&&rect.height>0))return null
 const x=Number.isFinite(touch.x)?touch.x:touch.clientX-rect.left
 const y=Number.isFinite(touch.y)?touch.y:touch.clientY-rect.top
 if(!Number.isFinite(x)||!Number.isFinite(y))return null
 const dx=x-rect.width/2,dy=rect.height/2-y
 return {h:(Math.atan2(dy,dx)*180/Math.PI+360)%360,s:clamp(Math.hypot(dx,dy)/(Math.min(rect.width,rect.height)/2)*100,0,100)}
}
module.exports={normalizeHex,hexToRgb,rgbToHex,hsvToRgb,rgbToHsv,wheelPoint}
