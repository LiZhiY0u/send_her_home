#pragma once
#include <Arduino.h>
class Motor {
public:
    Motor(int in1, int in2, int frequency, int channel1, int channel2)
        : in1_(in1), in2_(in2), frequency_(frequency),
          channel1_(channel1), channel2_(channel2) {}
    void begin() {
        if (ready_) return;
        digitalWrite(in1_, LOW);
        digitalWrite(in2_, LOW);
        pinMode(in1_, OUTPUT);
        pinMode(in2_, OUTPUT);
        ledcSetup(channel1_, frequency_, 8);
        ledcSetup(channel2_, frequency_, 8);
        ledcWrite(channel1_, 0);
        ledcWrite(channel2_, 0);
        ledcAttachPin(in1_, channel1_);
        ledcAttachPin(in2_, channel2_);
        ready_ = true;
    }
    // Signed duty: -255..255. Only call from the owning task, never an ISR.
    void drive(int speed) {
        if (!ready_) return;
        if (speed > 255) speed = 255;
        if (speed < -255) speed = -255;
        stop(); // Disable both inputs before changing direction.
        if (speed > 0) ledcWrite(channel1_, speed);
        else if (speed < 0) ledcWrite(channel2_, -speed);
    }
    void brake() {
        if (!ready_) return;
        ledcWrite(channel1_, 255);
        ledcWrite(channel2_, 255);
    }
    void stop() {
        if (!ready_) return;
        ledcWrite(channel1_, 0);
        ledcWrite(channel2_, 0);
    }
private:
    int in1_, in2_, frequency_, channel1_, channel2_;
    bool ready_ = false;
};
