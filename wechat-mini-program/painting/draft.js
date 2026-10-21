let frame=new Uint8Array(399),painted=false
function saveDraft(value){frame=Uint8Array.from(value);painted=true}
function getDraft(){return {frame:Uint8Array.from(frame),painted}}
module.exports={saveDraft,getDraft}
