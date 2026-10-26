#pragma once
struct MotorPageState {
    bool running = false, reverse = false;
    int speed = 60;
    int Duty() const { return running ? (reverse ? -1 : 1) * speed * 255 / 100 : 0; }
    void ToggleRun() { running = !running; }
    void ToggleDirection() { if (!running) reverse = !reverse; }
    void AdjustSpeed(int delta) { speed += delta; if (speed < 0) speed = 0; if (speed > 100) speed = 100; }
    void Stop() { running = false; }
};
