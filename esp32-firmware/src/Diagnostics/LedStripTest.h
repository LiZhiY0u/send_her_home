#pragma once
#include <cstdint>
namespace LedStripTest {
constexpr unsigned PixelCount = 133;
constexpr unsigned RegionCount = 5, CornerCount = 4, CornerPixels = 25;
constexpr unsigned CornerStart = RegionCount;
constexpr unsigned ChainStart = CornerStart + CornerPixels;
constexpr unsigned StepCount = ChainStart + PixelCount + 1;
constexpr uint8_t Brightness = 26; // About 10% of 255; not a current limiter.
// Actual chain: center, upper-left, lower-left, lower-right, upper-right.
constexpr unsigned Starts[] = {0, 33, 58, 83, 108};
constexpr unsigned Counts[] = {33, 25, 25, 25, 25};
constexpr uint32_t Colors[] = {0xffffffu, 0xff0000u, 0x00ff00u, 0x0000ffu, 0x8000ffu};
inline unsigned DelayMs(unsigned step) {
    step %= StepCount;
    return step < CornerStart || step == StepCount - 1 ? 1000 : step < ChainStart ? 200 : 80;
}
template <typename Strip> void Draw(Strip& strip, unsigned step) {
    step %= StepCount;
    strip.clear();
    if (step < CornerStart) {
        for (unsigned i = 0; i < Counts[step]; ++i) strip.setPixelColor(Starts[step] + i, Colors[step]);
    } else if (step < ChainStart) {
        const unsigned local = step - CornerStart;
        for (unsigned corner = 1; corner <= CornerCount; ++corner)
            strip.setPixelColor(Starts[corner] + local, Colors[corner]);
    } else if (step < ChainStart + PixelCount) {
        strip.setPixelColor(step - ChainStart, 0xffffffu);
    }
    strip.show();
}
}
