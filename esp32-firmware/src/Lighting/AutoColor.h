#pragma once
#include <cstdint>
// Automatic playlist color clock; independent from the Mini App's random-color mode.
namespace AutoColor {
constexpr uint32_t PeriodMs=2000,FadeMs=400;
class State {
    uint32_t seed_,start_=0,from_=0,to_=0;
    bool started_=false;
    uint32_t Next(){
        static const uint32_t colors[]={0xff4682,0x3cc8ff,0xffb428,0x8c50ff,0x32ff82,0xff4628,0x4664ff};
        seed_^=seed_<<13;seed_^=seed_>>17;seed_^=seed_<<5;
        unsigned index=seed_%7;
        if(colors[index]==to_)index=(index+1)%7;
        return colors[index];
    }
public:
    explicit State(uint32_t seed):seed_(seed?seed:0x9e3779b9u){}
    uint32_t At(uint32_t now){
        if(!started_){started_=true;start_=now;to_=Next();from_=to_;}
        if(static_cast<uint32_t>(now-start_)>=PeriodMs){from_=to_;to_=Next();start_=now;}
        uint32_t mix=now-start_;if(mix>FadeMs)mix=FadeMs;
        uint32_t result=0;
        for(unsigned shift=0;shift<24;shift+=8){
            const uint32_t a=(from_>>shift)&255,b=(to_>>shift)&255;
            result|=((a*(FadeMs-mix)+b*mix)/FadeMs)<<shift;
        }
        return result;
    }
};
}
