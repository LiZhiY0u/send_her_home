#include <cassert>
#include "DRV8837.h"
static int duty[16] = {};
void pinMode(int, int) {}
void digitalWrite(int, int) {}
double ledcSetup(int, double frequency, int bits) { assert(bits == 8); return frequency; }
void ledcAttachPin(int pin, int channel) { assert((pin == 4 && channel == 1) || (pin == 16 && channel == 2)); }
void ledcWrite(int channel, int value) { duty[channel] = value; }
int main() {
    Motor motor(4, 16, 1000, 1, 2);
    motor.drive(120); // Calls before initialization must not activate outputs.
    assert(duty[1] == 0 && duty[2] == 0);
    motor.begin();
    assert(duty[1] == 0 && duty[2] == 0);
    motor.drive(120);
    assert(duty[1] == 120 && duty[2] == 0);
    motor.drive(-90);
    assert(duty[1] == 0 && duty[2] == 90);
    motor.drive(0);
    assert(duty[1] == 0 && duty[2] == 0);
    motor.drive(999);
    assert(duty[1] == 255 && duty[2] == 0);
    motor.drive(-999);
    assert(duty[1] == 0 && duty[2] == 255);
    motor.brake();
    assert(duty[1] == 255 && duty[2] == 255);
    motor.stop();
    assert(duty[1] == 0 && duty[2] == 0);
}
