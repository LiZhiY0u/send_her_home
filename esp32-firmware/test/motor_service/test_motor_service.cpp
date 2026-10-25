#include "MotorControl/MotorService.h"
#include "DRV8837.h"
#include <cassert>
Motor motor;uint32_t motor_test_now=0;
void command(uint8_t op,uint8_t token,uint8_t seq,uint8_t run=0,uint8_t reverse=0,uint8_t speed=30,uint8_t keep=1,uint8_t mask=15){
 uint8_t p[]={0xb8,1,op,token,0,seq,0,run,reverse,speed,keep,mask};
 assert(MotorService::Enqueue(p,op==2?12:7));MotorService::Tick();
}
unsigned reply(){uint8_t p[16];assert(MotorService::ReadReply(p));return p[2];}
int main(){
 assert(MotorService::Init());MotorService::Connect();command(1,1,1);assert(reply()==0&&motor.duty==0);
 command(2,1,2,1);assert(reply()==0&&motor.duty==76);
 command(2,1,2,0);assert(reply()==3&&motor.duty==76);
 command(2,1,3,1,1);assert(reply()==2&&motor.duty==76);
 MotorService::Disconnect();MotorService::Tick();assert(motor.duty==76);
 MotorService::Connect();command(1,2,1);assert(reply()==0&&motor.duty==76);
 command(4,2,2);assert(reply()==0&&motor.duty==0);
 // A stale running=true in a speed-only packet cannot restart the motor.
 command(2,2,3,1,0,50,1,4);assert(reply()==0&&motor.duty==0&&MotorService::State().speed==50);
 command(2,2,4,1,0,30,0);assert(reply()==0&&motor.duty==76);
 motor_test_now=3000;MotorService::Tick();assert(motor.duty==0);
 command(2,2,5,1,0,30,0);assert(reply()==0&&motor.duty==76);
 MotorService::Disconnect();MotorService::Tick();assert(motor.duty==0);
 MotorService::Connect();command(1,3,1);assert(reply()==0);
 uint8_t queued[]={0xb8,1,2,3,0,2,0,1,0,30,1,15};assert(MotorService::Enqueue(queued,12));
 MotorService::Disconnect();MotorService::Connect();MotorService::Tick();assert(motor.duty==0);
 auto& state=MotorService::State();state.running=true;state.speed=30;state.keepRunning=true;
 MotorService::ApplyLocal();assert(motor.duty==76);MotorService::StopLocal();assert(motor.duty==76);
 state.keepRunning=false;MotorService::StopLocal();assert(motor.duty==0);
}
