#include "MotorService.h"
#include "DRV8837.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>
#include <cstring>
extern Motor motor;
namespace {
struct Request {uint32_t epoch;uint8_t bytes[12];};
struct Reply {uint32_t epoch;uint8_t bytes[16];};
QueueHandle_t requests=nullptr,replies=nullptr;
std::atomic<uint32_t> epoch{0};std::atomic<bool> connected{false};
MotorModel state;int appliedDuty=0;
uint16_t session=0,lastSequence=0,ownerSession=0;uint32_t sessionEpoch=0;
void Apply(){const int duty=state.Duty();if(duty!=appliedDuty){motor.drive(duty);appliedDuty=duty;}}
void Respond(const Request& r,uint8_t code){
 Reply reply{};reply.epoch=r.epoch;const auto p=r.bytes;
 const uint8_t head[]={0xb8,1,code,p[2],p[3],p[4],p[5],p[6],static_cast<uint8_t>(state.running),
  static_cast<uint8_t>(state.reverse),static_cast<uint8_t>(state.speed),static_cast<uint8_t>(state.keepRunning),static_cast<uint8_t>(state.remote)};
 std::memcpy(reply.bytes,head,sizeof(head));xQueueSend(replies,&reply,0);
}
}
bool MotorService::Init(){
 if(requests)return true;
 requests=xQueueCreate(8,sizeof(Request));replies=xQueueCreate(8,sizeof(Reply));
 if(!requests||!replies){if(requests)vQueueDelete(requests);if(replies)vQueueDelete(replies);requests=replies=nullptr;return false;}
 return true;
}
void MotorService::Connect(){++epoch;connected=true;}
void MotorService::Disconnect(){connected=false;++epoch;}
bool MotorService::Enqueue(const uint8_t* p,unsigned n){
 if(!requests||!connected||!p||n<7||p[0]!=0xb8||p[1]!=1||(!p[3]&&!p[4]))return false;
 if(p[2]<1||p[2]>4||n!=(p[2]==2?12u:7u))return false;
 if(p[2]==2&&(p[7]>1||p[8]>1||p[9]>100||p[10]>1||!p[11]||p[11]>15))return false;
 Request r{};r.epoch=epoch.load();std::memcpy(r.bytes,p,n);
 return xQueueSend(requests,&r,0)==pdTRUE;
}
bool MotorService::ReadReply(uint8_t* p){
 if(!replies)return false;
 Reply reply;
 for(unsigned i=0;i<8&&xQueueReceive(replies,&reply,0)==pdTRUE;++i){
  if(reply.epoch!=epoch.load())continue;
  std::memcpy(p,reply.bytes,16);return true;
 }
 return false;
}
MotorModel& MotorService::State(){return state;}
void MotorService::ApplyLocal(){state.TakeLocal();Apply();}
void MotorService::StopLocal(){if(!state.remote&&!state.keepRunning){state.Stop();motor.stop();appliedDuty=0;}}
void MotorService::Tick(){
 const uint32_t now=millis(),currentEpoch=epoch.load();
 state.Tick(now,connected.load(),currentEpoch);Apply();
 if(!requests)return;
 Request r;
 // Bounded work per UI frame. Old-link commands never execute after reconnect.
 for(unsigned i=0;i<4&&xQueueReceive(requests,&r,0)==pdTRUE;++i){
  if(r.epoch!=currentEpoch||!connected.load())continue;
  const auto p=r.bytes;const uint16_t token=p[3]|(p[4]<<8),seq=p[5]|(p[6]<<8);
  if(sessionEpoch!=currentEpoch){sessionEpoch=currentEpoch;session=0;}
  if(token!=session){if(p[2]!=1){Respond(r,3);continue;}session=token;lastSequence=seq;}
  else {if(static_cast<int16_t>(seq-lastSequence)<=0){Respond(r,3);continue;}lastSequence=seq;}
  uint8_t code=0;
  if(p[2]==2){
   if(!state.Patch(p[11],p[7],p[8],p[9],p[10],now,currentEpoch))code=2;
   else ownerSession=token;
  }else if(p[2]==4){state.Stop();}
  if(state.remote&&state.ownerEpoch==currentEpoch&&token==ownerSession)state.lastSeen=now;
  Apply();Respond(r,code);
 }
}
