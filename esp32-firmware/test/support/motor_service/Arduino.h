#pragma once
#include <cstdint>
extern uint32_t motor_test_now;
inline uint32_t millis(){return motor_test_now;}
