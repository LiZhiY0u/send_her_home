#pragma once
#include <cassert>
struct Motor {
    Motor(int in1, int in2, int frequency, int channel1, int channel2) {
        assert(in1 == 4 && in2 == 16 && frequency == 1000 && channel1 == 1 && channel2 == 2);
    }
    void begin() {}
};
