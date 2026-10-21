const finite = (value, fallback) => Number.isFinite(value) ? value : fallback
const clamp = (value, min, max) => Math.max(min, Math.min(max, value))
const EFFECTS = [
  ['solid','常亮'], ['rotate','旋转光线'], ['ripple','圆形扩散'],
  ['waveX','水平波浪'], ['waveDiagonal','斜向波浪'], ['sine','正弦光带'],
  ['ocean','海浪叠加'], ['squareMove','方框平移'], ['squareExpand','方形扩散'],
  ['squareRotate','方框旋转'], ['radar','雷达扫描'], ['spiral','螺旋光带'],
  ['cornerRelay','四角接力'], ['aurora','极光流动']
].map(([value,label]) => ({value,label}))
const LINK_MODES = [['sync','同步'],['reverse','反向'],['mirror','镜像'],['delay','延迟回声']]
  .map(([value,label]) => ({value,label}))
const TAU = Math.PI * 2
const band = (distance, width = 22) => clamp(1 - Math.abs(distance) / width, 0, 1)
const periodicBand = (phase, width=.2) => band(Math.atan2(Math.sin(phase),Math.cos(phase)), width * Math.PI)
function spatialStrength(point, settings, time, speed) {
  const top = point.layer === 'top'
  let x = finite(point.x,0), y = -finite(point.z,0)
  if(top && settings.linkedMode === 'mirror') x = -x
  const t = (time - (top && settings.linkedMode === 'delay' ? .5 : 0)) * speed *
    (top && settings.linkedMode === 'reverse' ? -1 : 1)
  const angle = Math.atan2(y,x), r = Math.hypot(x,y)
  const a=t*.6, c=Math.cos(a), s=Math.sin(a)
  const square = (px,py,size) => band(Math.max(Math.abs(px),Math.abs(py))-size)
  switch(settings.effect) {
    case 'waveX': return .08 + .92*(.5+.5*Math.sin(x*.045-t*1.8))**2
    case 'waveDiagonal': return .08+.92*(.5+.5*Math.sin((x+y)*.035-t*1.8))**2
    case 'sine': return band(y-38*Math.sin(x*.035-t),26)
    case 'ocean': return clamp(.12+.48*(.5+.5*Math.sin(x*.035+y*.022-t))+.4*(.5+.5*Math.sin(y*.05-x*.018-t*1.4)),0,1)
    case 'squareMove': return square(x-105*Math.sin(t*.65),y,40)
    case 'squareExpand': return square(x,y,((t/4%1+1)%1)*150)
    case 'squareRotate': return square(x*c+y*s,-x*s+y*c,62)
    case 'radar': { const d=(a-angle+TAU*100)%TAU;return Math.exp(-d*3)*(.25+.75*Math.min(r/40,1)) }
    case 'spiral': return periodicBand(angle-r*.045+t, .3)
    case 'cornerRelay': {
      const centers=[[-78,78],[-78,-78],[78,-78],[78,78]]
      return centers.reduce((v,[cx,cy],i)=>Math.max(v,
        Math.exp(-((x-cx)**2+(y-cy)**2)/3200)*Math.max(0,Math.cos(t*1.5-i*Math.PI/2))**3),0)
    }
    case 'aurora': return clamp(.12+.55*(.5+.5*Math.sin(x*.024+t*.35+1.8*Math.sin(y*.018-t*.25)))+
      .3*(.5+.5*Math.sin(y*.04-x*.015+t*.45)),0,1)
    default: return 1
  }
}
function selected(point, settings) {
  return (!settings.layer || settings.layer === 'both' || settings.layer === point.layer) &&
    (!settings.region || settings.region === 'all' || settings.region === point.region ||
      (settings.region === 'corners' && point.region !== 'CENTER'))
}
function sampleEffect(point, settings, elapsedSeconds) {
  if (!selected(point, settings)) return [0, 0, 0]
  const brightness = clamp(finite(settings.brightness, 60), 0, 100) / 100
  const speed = clamp(finite(settings.speed, 50), 0, 100) / 50
  const time = Math.max(0, finite(elapsedSeconds, 0))
  const linkedTime = time - (point.layer === 'top' && settings.linkedMode === 'delay' ? .5 : 0)
  const reverse = settings.linkedMode === 'reverse' && point.layer === 'top'
  let strength = 1
  if (settings.effect === 'rotate') {
    const phase = (reverse ? -1 : 1) * linkedTime * speed * Math.PI / 2
    const angle = finite(point.angle, 0)
    const mirroredAngle = point.layer === 'top' && settings.linkedMode === 'mirror' ? Math.PI-angle : angle
    const distance = Math.abs(Math.sin(mirroredAngle - phase)) * finite(point.radius, 0)
    strength = clamp(1 - distance / 14, 0, 1)
  } else if (settings.effect === 'ripple') {
    const phase = ((linkedTime * speed / 2) % 1 + 1) % 1
    const radius = (reverse ? 1 - phase : phase) * 160
    strength = clamp(1 - Math.abs(finite(point.radius, 0) - radius) / 16, 0, 1)
  } else {
    strength = spatialStrength(point, settings, time, speed)
  }
  const hex = /^#[0-9a-f]{6}$/i.test(settings.color) ? settings.color.slice(1) : '57d9ff'
  return [0, 2, 4].map(i => parseInt(hex.slice(i, i + 2), 16) / 255 * brightness * strength)
}
module.exports = { sampleEffect, selected, EFFECTS, LINK_MODES }
