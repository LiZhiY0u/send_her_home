#pragma once
#include <Arduino.h>
struct Motor {int duty=0;unsigned calls=0;void drive(int d){duty=d;++calls;}void stop(){duty=0;++calls;}};
