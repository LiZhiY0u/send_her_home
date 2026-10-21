#pragma once
#include <cstdint>
#include <cmath>
#include "LedPhysicalMap.h"
namespace SpatialLedTest {
constexpr unsigned StepCount = 1050, StepMs = 20;
constexpr unsigned RippleSteps = 450; // Three 3-second waves, then two 6-second rotations.
inline uint32_t ColorAt(LedPhysicalMap::Point point, unsigned step) {
    step %= StepCount;
    float distance;
    uint8_t red = 0, green = 180, blue = 255;
    if (step < RippleSteps) {
        const float radius = static_cast<float>(step % 150); // 0..149mm / 3s.
        distance = std::fabs(std::hypot(point.x, point.y) - radius);
    } else {
        const float angle = static_cast<float>((step - RippleSteps) % 300) * 6.28318530718f / 300.0f;
        distance = std::fabs(-point.x * std::sin(angle) + point.y * std::cos(angle));
        red = 255; green = 100; blue = 0;
    }
    const float strength = distance < 12.0f ? 1.0f - distance / 12.0f : 0.0f;
    return (static_cast<uint32_t>(red * strength) << 16) |
           (static_cast<uint32_t>(green * strength) << 8) |
            static_cast<uint32_t>(blue * strength);
}
template <typename Strip> void Draw(Strip& strip, unsigned step) {
    strip.clear();
    for (unsigned i = 0; i < LedPhysicalMap::Count; ++i)
        strip.setPixelColor(i, ColorAt(LedPhysicalMap::Get(i), step));
    strip.show();
}
}
