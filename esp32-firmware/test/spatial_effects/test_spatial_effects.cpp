#include "Lighting/SpatialEffects.h"
#include <cassert>
#include <cmath>
int main() {
    using namespace SpatialEffects;
    assert(Count == 22);
    assert(Strength(Effect::WaveX,-Pi/2/.045f,0,0)<.001f);
    assert(Strength(Effect::WaveX,0,0,0)<.1f);
    assert(Strength(Effect::WaveX,Pi/2/.045f,0,0)>.999f);
    for(unsigned e=0;e<Count;++e) {
        bool changed=false; float first=Strength(static_cast<Effect>(e),45,25,0);
        for(unsigned t=0;t<100;++t) {
            float v=Strength(static_cast<Effect>(e),45,25,t*.13f);
            assert(std::isfinite(v)&&v>=0&&v<=1);
            changed |= std::fabs(v-first)>.001f;
        }
        assert(e==0 || changed);
    }
    assert(PlaylistEffect(0)==Effect::WaveX);
    assert(PlaylistEffect(600)==Effect::WaveDiagonal);
    assert(PlaylistEffect(7200)==Effect::Inward);
    assert(PlaylistEffect(12000)==Effect::WaveX);
    const float a=Strength(Effect::WaveX,45,25,1.3f);
    assert(std::fabs(a-Strength(Effect::WaveX,45,25,1.8f,true,Link::Delay))<.00001f);
    assert(std::fabs(a-Strength(Effect::WaveX,45,25,1.3f,true,Link::Mirror))>.01f);
    assert(ColorAt(Effect::Solid,0,0,0,0x123456)==0x123456);
}
