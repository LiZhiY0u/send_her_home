#pragma once
#include "MotorModel.h"
namespace MotorService {
bool Init();
void Connect();void Disconnect();
// BLE task: validate/copy only. No PWM calls.
bool Enqueue(const uint8_t* packet,unsigned size);
bool ReadReply(uint8_t* packet);
// Display task owns all following methods and the driver.
void Tick();MotorModel& State();void ApplyLocal();void StopLocal();
}
