#pragma once
#include "MotorControl/MotorModel.h"
#include "DRV8837.h"
extern Motor motor;
namespace MotorService {
inline MotorModel& State(){static MotorModel state;return state;}
inline void ApplyLocal(){State().TakeLocal();motor.drive(State().Duty());}
inline void StopLocal(){if(!State().remote&&!State().keepRunning){State().Stop();motor.stop();}}
}
