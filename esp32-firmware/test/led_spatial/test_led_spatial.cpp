#include <cassert>
#include <cmath>
#include <cstdint>
#include "Diagnostics/SpatialLedTest.h"
struct Strip {
    uint32_t pixels[133] = {}; unsigned shown = 0;
    void clear() { for (auto& p : pixels) p = 0; }
    void setPixelColor(unsigned i, uint32_t c) { assert(i < 133); pixels[i] = c; }
    void show() { ++shown; }
};
int main() {
    const auto first = LedPhysicalMap::Get(0);
    assert(std::fabs(first.x) < 0.01f && std::fabs(first.y + 40) < 0.01f);
    const auto center = LedPhysicalMap::Get(32);
    assert(center.x == 0 && center.y == 0);
    for (unsigned i = 0; i < 133; ++i) {
        const auto p = LedPhysicalMap::Get(i);
        assert(std::isfinite(p.x) && std::isfinite(p.y));
        for (unsigned j = i + 1; j < 133; ++j) {
            const auto q = LedPhysicalMap::Get(j);
            assert(std::hypot(p.x - q.x, p.y - q.y) > 1);
        }
    }
    for (unsigned local = 0; local < 25; ++local) {
        auto p = LedPhysicalMap::Get(33 + local);
        for (unsigned corner = 1; corner < 4; ++corner) {
            const auto q = LedPhysicalMap::Get(33 + corner * 25 + local);
            assert(std::fabs(q.x + p.y) < 0.01f && std::fabs(q.y - p.x) < 0.01f);
            p = q;
        }
    }
    assert(SpatialLedTest::ColorAt({0,0}, 0) != 0);
    assert(SpatialLedTest::ColorAt({40,0}, 0) == 0);
    assert(SpatialLedTest::ColorAt({40,0}, 40) != 0); // 0.8s -> radius40mm.
    assert(SpatialLedTest::ColorAt({-94,94}, 133) != 0);
    assert(SpatialLedTest::ColorAt({40,0}, 450) != 0);
    assert(SpatialLedTest::ColorAt({-40,0}, 450) != 0);
    assert(SpatialLedTest::ColorAt({0,40}, 450) == 0);
    assert(SpatialLedTest::ColorAt({0,40}, 525) != 0);
    Strip strip;
    for (unsigned step = 0; step < SpatialLedTest::StepCount; ++step) SpatialLedTest::Draw(strip, step);
    assert(strip.shown == SpatialLedTest::StepCount);
    SpatialLedTest::Draw(strip, SpatialLedTest::StepCount);
    assert(strip.pixels[32] != 0 && strip.pixels[0] == 0);
}
