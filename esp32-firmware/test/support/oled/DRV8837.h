#pragma once
struct Motor {
    int duty = 0;
    void drive(int value) { duty = value; }
    void stop() { duty = 0; }
};
