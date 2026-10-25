#include "MotorControl/MotorModel.h"
#include <cassert>
int main(){
 MotorModel m;assert(!m.running&&m.speed==30&&m.keepRunning);
 assert(m.SetRemote(true,false,30,true,10,5));assert(m.Duty()==76);
 m.Tick(100000,false,5);assert(m.running);
 m.Stop();assert(m.Patch(4,true,false,40,true,0,5));assert(!m.running&&m.speed==40);
 assert(m.SetRemote(true,false,30,true,10,5));
 assert(!m.SetRemote(true,true,40,true,11,5));assert(!m.reverse&&m.speed==30);
 assert(m.SetRemote(false,false,30,false,12,5));
 assert(m.SetRemote(true,true,50,false,100,5));
 m.Tick(3099,true,5);assert(m.running);m.Tick(3100,true,5);assert(!m.running);
 assert(m.SetRemote(true,false,30,false,0xfffffff0u,6));
 m.Tick(2984,true,6);assert(!m.running);
 assert(m.SetRemote(true,false,30,false,0,7));m.Tick(1,false,8);assert(!m.running);
 assert(m.SetRemote(true,false,30,true,0,7));m.TakeLocal();m.Tick(5000,false,8);assert(m.running);
}
