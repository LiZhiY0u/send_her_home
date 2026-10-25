#pragma once
// Set to 0 to restore the original GPIO19 rainbow demonstration.
#ifndef ENABLE_LED_STRIP_TEST
#define ENABLE_LED_STRIP_TEST 1
#endif
// 1: 20-effect spatial playlist; 0: region and chain mapping verification.
#ifndef ENABLE_LED_SPATIAL_TEST
#define ENABLE_LED_SPATIAL_TEST 1
#endif
namespace LightingTestConfig {
constexpr int Pin = 17;
constexpr int TopPin = 5;
static_assert(Pin != TopPin,"Top and bottom strips require separate GPIOs");
}
