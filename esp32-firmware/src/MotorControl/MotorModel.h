#pragma once
#include <cstdint>
#include "OledUI/MotorPageState.h"
struct MotorModel:MotorPageState {
 bool keepRunning=true,remote=false;
 uint32_t ownerEpoch=0,lastSeen=0;
 bool SetRemote(bool run,bool rev,unsigned percent,bool keep,uint32_t now,uint32_t epoch){
  if(percent>100||(running&&rev!=reverse))return false;
  running=run;reverse=rev;speed=percent;keepRunning=keep;remote=true;ownerEpoch=epoch;lastSeen=now;return true;
 }
 void TakeLocal(){remote=false;}
 bool Patch(uint8_t mask,bool run,bool rev,unsigned percent,bool keep,uint32_t now,uint32_t epoch){
  if(!mask||mask>15)return false;
  return SetRemote(mask&1?run:running,mask&2?rev:reverse,mask&4?percent:speed,mask&8?keep:keepRunning,now,epoch);
 }
 void Tick(uint32_t now,bool connected,uint32_t epoch){
  if(remote&&!keepRunning&&(!connected||ownerEpoch!=epoch||static_cast<uint32_t>(now-lastSeen)>=3000))Stop();
 }
};
