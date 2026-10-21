function paintSegment(frame, points, a, b, { diameter, color }) {
  if (![a.x,a.y,b.x,b.y,diameter].every(Number.isFinite) || diameter <= 0) return
  const dx=b.x-a.x, dy=b.y-a.y, length=dx*dx+dy*dy, radius=diameter/2
  points.forEach((p,i) => {
    const t=length ? Math.max(0,Math.min(1,((p.x-a.x)*dx+(p.y-a.y)*dy)/length)) : 0
    const distance=Math.hypot(p.x-a.x-t*dx,p.y-a.y-t*dy)
    const weight=Math.max(0,1-distance/radius)
    for(let c=0;c<3;++c) frame[i*3+c]=Math.round(frame[i*3+c]*(1-weight)+color[c]*weight)
  })
}
module.exports = { paintSegment }
