#include "PaintBridge.h"
#include "PaintProtocol.h"
#include "PaintMapIdentity.h"
#include "MotorControl/MotorService.h"
#include "Lighting/DeviceState.h"
#include <Arduino.h>
#include <btstack.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <cstring>
namespace {
PaintProtocol::Receiver receiver;
QueueHandle_t commands=nullptr,acks=nullptr;
struct Ack {uint8_t session;uint16_t sequence;bool automatic;bool effect;};
uint16_t phone=0xffff;bool subscribed=false,notifyPending=false,paintMode=false;
uint8_t status[20]={0xa7,1,0,0,0,0,0,0};uint16_t statusSize=16;
uint8_t motorStatus[16]={};bool motorPending=false;
uint8_t lightingStatus[20]={};bool lightingPending=false;
btstack_timer_source_t timer;
btstack_context_callback_registration_t registration;
void Status(uint8_t code,uint8_t op,uint8_t session,uint16_t sequence) {
    statusSize=16;
    status[2]=code;status[3]=paintMode?1:0;status[4]=session;status[5]=sequence;status[6]=sequence>>8;status[7]=op;
    for(unsigned i=0;i<4;++i) status[8+i]=PaintMapHash>>(8*i);
    status[12]=133;status[13]=26;notifyPending=true;
    if(phone!=0xffff&&subscribed)att_server_request_can_send_now_event(phone);
}
void Tick(btstack_timer_source_t*) {
    if(!motorPending&&phone!=0xffff&&subscribed)motorPending=MotorService::ReadReply(motorStatus);
    Ack ack;
    if(acks&&xQueueReceive(acks,&ack,0)==pdTRUE) {
        paintMode=!ack.automatic&&!ack.effect;
        if(ack.session==receiver.Session()) Status(0,ack.effect?6:(ack.automatic?5:4),ack.session,ack.sequence);
    }
    if((notifyPending||motorPending||lightingPending)&&subscribed&&phone!=0xffff)att_server_request_can_send_now_event(phone);
    btstack_run_loop_set_timer(&timer,30);btstack_run_loop_add_timer(&timer);
}
void StartTimer(void*) {
    btstack_run_loop_set_timer_handler(&timer,Tick);
    btstack_run_loop_set_timer(&timer,30);btstack_run_loop_add_timer(&timer);
}
}
bool PaintBridge::Init() {
    if(!MotorService::Init())return false;
    if(commands)return true;
    commands=xQueueCreate(1,sizeof(Command));acks=xQueueCreate(1,sizeof(Ack));
    if(!commands||!acks) { if(commands)vQueueDelete(commands);if(acks)vQueueDelete(acks);commands=acks=nullptr;return false; }
    registration.callback=StartTimer;registration.context=nullptr;
    btstack_run_loop_execute_on_main_thread(&registration);return true;
}
void PaintBridge::Connect(uint16_t handle) {phone=handle;subscribed=false;motorPending=lightingPending=false;receiver.Disconnect();MotorService::Connect();}
void PaintBridge::Disconnect(uint16_t handle) {
    if(handle!=phone)return;
    phone=0xffff;subscribed=notifyPending=false;receiver.Disconnect();
    motorPending=lightingPending=false;MotorService::Disconnect();
    if(commands)xQueueReset(commands);
}
int PaintBridge::Write(uint16_t handle,uint16_t attribute,uint16_t transaction,uint16_t offset,uint8_t* data,uint16_t size) {
    if(handle!=phone||transaction!=ATT_TRANSACTION_MODE_NONE||offset)return 0;
    if(attribute==CccHandle) {
        if(size==2&&data)subscribed=(data[0]==1&&data[1]==0);
        return 0;
    }
    if(attribute!=WriteHandle||!commands||!data)return 0;
    if(size&&data[0]==0xb8)return MotorService::Enqueue(data,size)?0:0x13;
    if(size==8&&data[0]==0xa7&&data[1]==1&&data[2]==7&&data[3]&&!data[6]&&!data[7]){
        LightingState::Snapshot snapshot;
        const uint8_t header[]={0xa7,1,static_cast<uint8_t>(LightingState::Read(snapshot)?0:3),snapshot.mode,data[3],data[4],data[5],7};
        std::memcpy(lightingStatus,header,8);
        std::memcpy(lightingStatus+8,snapshot.settings,10);lightingStatus[18]=snapshot.elapsed;lightingStatus[19]=snapshot.elapsed>>8;
        lightingPending=true;if(subscribed)att_server_request_can_send_now_event(phone);
        return 0;
    }
    const uint8_t code=receiver.Accept(data,size,millis());
    const uint8_t op=size>=3?data[2]:0;
    const uint16_t sequence=size>=6?static_cast<uint16_t>(data[4]|(data[5]<<8)):0;
    if(code) {Status(code,op,receiver.Session(),sequence);return 0;}
    if(op==1) {xQueueReset(commands);Status(0,op,receiver.Session(),0);}
    if(receiver.Ready()||receiver.AutoRequested()||receiver.EffectRequested()) {
        Command command{};command.session=receiver.Session();command.sequence=receiver.Sequence();command.automatic=receiver.AutoRequested();
        command.effect=receiver.EffectRequested();
        if(command.effect)std::memcpy(command.settings,receiver.EffectSettings(),10);
        else if(!command.automatic)std::memcpy(command.pixels,receiver.Pixels(),399);
        if(xQueueSend(commands,&command,0)!=pdTRUE)Status(3,op,command.session,command.sequence);
        receiver.ClearReady();
    }
    return 0;
}
uint16_t PaintBridge::ReadStatus(uint16_t handle,uint16_t attribute,uint16_t offset,uint8_t* data,uint16_t size) {
    if(handle!=phone)return 0;
    if(attribute==CccHandle) {const uint8_t value[2]={static_cast<uint8_t>(subscribed),0};return att_read_callback_handle_blob(value,2,offset,data,size);}
    if(attribute!=StatusHandle)return 0;
    return att_read_callback_handle_blob(status,statusSize,offset,data,size);
}
void PaintBridge::CanSend() {
    if(phone!=0xffff&&subscribed&&motorPending){
        if(att_server_notify(phone,StatusHandle,motorStatus,16)==0)motorPending=false;
        return;
    }
    if(phone!=0xffff&&subscribed&&notifyPending){if(att_server_notify(phone,StatusHandle,status,statusSize)==0)notifyPending=false;return;}
    if(phone!=0xffff&&subscribed&&lightingPending&&att_server_notify(phone,StatusHandle,lightingStatus,20)==0)lightingPending=false;
}
bool PaintBridge::Read(Command& command) {return commands&&xQueueReceive(commands,&command,0)==pdTRUE;}
bool PaintBridge::SelectLocalEffect(uint8_t effect,bool automatic) {
    if(!commands||effect>=22)return false;
    Command command{};command.automatic=automatic;command.effect=!automatic;
    LightingState::Snapshot current;LightingState::Read(current);
    std::memcpy(command.settings,current.settings,10);command.settings[0]=effect;
    return xQueueSend(commands,&command,0)==pdTRUE;
}
bool PaintBridge::SetLocalRandomColor(bool enabled){
    if(!commands)return false;
    Command command{};command.colorModeOnly=true;command.settings[6]=enabled?1:0;
    return xQueueSend(commands,&command,0)==pdTRUE;
}
bool PaintBridge::SetLocalBrightness(uint8_t percent){
    if(!commands||percent>100)return false;
    Command command{};command.brightnessOnly=true;command.settings[2]=percent;
    return xQueueSend(commands,&command,0)==pdTRUE;
}
void PaintBridge::Applied(const Command& command) {
    if(command.session==0)return;
    if(acks){const Ack ack={command.session,command.sequence,command.automatic,command.effect};xQueueOverwrite(acks,&ack);}
}
