#pragma once
#include <cstdint>
#include <cmath>
#include "Diagnostics/LedPhysicalMap.h"
#include "Lighting/SpatialEffects.h"
namespace StartupSequence {
constexpr uint32_t FillMs=2000,HoldMs=0,DrainMs=2000,DarkMs=20;
constexpr uint32_t RippleMs=4000,RotateMs=8000,Color=0xffffff;
constexpr uint32_t TotalMs=FillMs+HoldMs+DrainMs+DarkMs+RippleMs+RotateMs;
// A 24mm transition band spans nearby LEDs; smoothstep removes hard ring jumps.
inline float SoftEdge(float edge,float radius){
    float t=(edge-radius+12.f)/24.f;
    if(t<=0)return 0;
    if(t>=1)return 1;
    return t*t*(3.f-2.f*t);
}
class State {
    bool started_=false,finished_=false;
    uint32_t start_=0;
public:
    bool Active(bool awake,uint32_t now){
        if(!awake||finished_)return false;
        if(!started_){started_=true;start_=now;}
        if(static_cast<uint32_t>(now-start_)>=TotalMs){finished_=true;return false;}
        return true;
    }
    void Finish(){finished_=true;}
    unsigned BrightnessPercent(uint32_t now)const{
        return static_cast<uint32_t>(now-start_)<FillMs+HoldMs+DrainMs?40u:100u;
    }
    float Strength(float x,float y,uint32_t now)const{
        uint32_t elapsed=now-start_;
        const float radius=std::sqrt(x*x+y*y);
        if(elapsed<FillMs)return SoftEdge(12.f+142.f*elapsed/FillMs,radius);
        elapsed-=FillMs;
        if(elapsed<DrainMs)return SoftEdge(154.f-166.f*elapsed/DrainMs,radius);
        elapsed-=DrainMs;
        if(elapsed<DarkMs)return 0;
        elapsed-=DarkMs;
        if(elapsed<RippleMs)return SpatialEffects::Strength(SpatialEffects::Effect::Ripple,x,y,elapsed/1000.f);
        elapsed-=RippleMs;
        if(elapsed<RotateMs)return SpatialEffects::Strength(SpatialEffects::Effect::Rotate,x,y,elapsed/1000.f);
        return 0;
    }
    template<class Strip>void Draw(Strip& strip,uint32_t now)const{
        for(unsigned i=0;i<LedPhysicalMap::Count;++i){
            const auto p=LedPhysicalMap::Get(i);
            const uint32_t level=static_cast<uint32_t>(255*Strength(p.x,p.y,now)*BrightnessPercent(now)/100.f);
            strip.setPixelColor(i,(level<<16)|(level<<8)|level);
        }
        strip.show();
    }
};
}
