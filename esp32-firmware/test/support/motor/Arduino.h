#pragma once
constexpr int OUTPUT = 1, LOW = 0;
void pinMode(int, int);
void digitalWrite(int, int);
double ledcSetup(int, double, int);
void ledcAttachPin(int, int);
void ledcWrite(int, int);
