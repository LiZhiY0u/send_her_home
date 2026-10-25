#include <cassert>
#include "Lighting/AutoColor.h"
int main(){
    AutoColor::State color(123);
    const uint32_t first=color.At(100);
    assert(first!=0);assert(color.At(2099)==first);
    assert(color.At(2100)==first);
    const auto middle=color.At(2300),second=color.At(2500);
    assert(second!=first);assert(middle!=first&&middle!=second);
    for(unsigned shift=0;shift<24;shift+=8)
        assert(((middle>>shift)&255)==((((first>>shift)&255)+((second>>shift)&255))/2));
    assert(color.At(4099)==second);assert(color.At(4100)==second);
    assert(color.At(4500)!=second);
    AutoColor::State wrap(123);
    const auto initial=wrap.At(0xffffff00u);
    assert(wrap.At(0xffffff00u+2000u)==initial);
    assert(wrap.At(0xffffff00u+2400u)!=initial);
}
