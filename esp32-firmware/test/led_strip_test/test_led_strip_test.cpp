#include <cassert>
#include <cstdint>
#include "Diagnostics/LedStripTest.h"
struct Strip {
    uint32_t pixels[133] = {}; int shown = 0;
    void clear() { for (auto& pixel : pixels) pixel = 0; }
    void setPixelColor(unsigned i, uint32_t value) { assert(i < 133); pixels[i] = value; }
    void show() { ++shown; }
};
int main() {
    Strip strip;
    const unsigned starts[] = {0, 33, 58, 83, 108};
    const unsigned counts[] = {33, 25, 25, 25, 25};
    const uint32_t colors[] = {0xffffff, 0xff0000, 0x00ff00, 0x0000ff, 0x8000ff};
    for (unsigned region = 0; region < 5; ++region) {
        LedStripTest::Draw(strip, region);
        for (unsigned i = 0; i < 133; ++i)
            assert(strip.pixels[i] == (i >= starts[region] && i < starts[region] + counts[region] ? colors[region] : 0u));
    }
    for (unsigned local = 0; local < 25; ++local) {
        LedStripTest::Draw(strip, 5 + local);
        for (unsigned i = 0; i < 133; ++i) {
            uint32_t expected = 0;
            for (unsigned corner = 1; corner < 5; ++corner)
                if (i == starts[corner] + local) expected = colors[corner];
            assert(strip.pixels[i] == expected);
        }
    }
    for (unsigned pixel = 0; pixel < 133; ++pixel) {
        LedStripTest::Draw(strip, 30 + pixel);
        for (unsigned i = 0; i < 133; ++i) assert(strip.pixels[i] == (i == pixel ? 0xffffffu : 0u));
    }
    LedStripTest::Draw(strip, 163);
    for (auto pixel : strip.pixels) assert(pixel == 0);
    LedStripTest::Draw(strip, 164);
    for (unsigned i = 0; i < 133; ++i) assert(strip.pixels[i] == (i < 33 ? 0xffffffu : 0u));
    assert(strip.shown == 165);
    assert(LedStripTest::StepCount == 164);
    assert(LedStripTest::DelayMs(0) == 1000);
    assert(LedStripTest::DelayMs(5) == 200);
    assert(LedStripTest::DelayMs(30) == 80);
    assert(LedStripTest::DelayMs(163) == 1000);
}
