function target(index){
 let v=(index+1)>>>0;v=(Math.imul(v,1664525)+1013904223)>>>0;v^=v>>>16
 const palette=[[255,70,130],[60,200,255],[255,180,40],[140,80,255],[50,255,130],[255,70,40],[70,100,255]]
 return palette[(v>>>0)%palette.length]
}
function cycleColor(seconds,base){
 const ms=Math.max(0,Math.floor((Number.isFinite(seconds)?seconds:0)*1000))
 const cycle=Math.floor(ms/5000),phase=ms%5000
 const from=cycle===0?base:target(cycle-1),to=target(cycle)
 const mix=Math.max(0,(phase-1000)/4000)
 return from.map((v,i)=>Math.floor(v+(to[i]-v)*mix))
}
module.exports={cycleColor}
