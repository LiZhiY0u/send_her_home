#pragma once
#include <cstdint>

// Share one logical frame without duplicating effect or painting state.
// Each physical strip retains its own driver buffer and GPIO output.
template<class Strip> class SynchronizedStrips {
public:
    SynchronizedStrips(Strip& bottom,Strip& top):bottom_(bottom),top_(top){}
    void begin(){bottom_.begin();top_.begin();}
    void setBrightness(uint8_t value){bottom_.setBrightness(value);top_.setBrightness(value);}
    void clear(){bottom_.clear();top_.clear();}
    void setPixelColor(unsigned index,uint32_t color){bottom_.setPixelColor(index,color);top_.setPixelColor(index,color);}
    // Sequential wire transfers, not electrically simultaneous refresh.
    void show(){bottom_.show();top_.show();}
private:
    Strip& bottom_;
    Strip& top_;
};
