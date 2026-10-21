#pragma once
#include <cmath>
#include <cstdint>
#include "Diagnostics/LedPhysicalMap.h"

// Pure spatial sampler: no allocation, GPIO access or RTOS dependency.
namespace SpatialEffects {
constexpr unsigned Count=14, FrameMs=20, FramesPerEffect=600;
constexpr float Pi=3.14159265359f, Tau=2*Pi;
enum class Effect { Solid, Rotate, Ripple, WaveX, WaveDiagonal, Sine, Ocean,
    SquareMove, SquareExpand, SquareRotate, Radar, Spiral, CornerRelay, Aurora };
enum class Link { Sync, Reverse, Mirror, Delay };
inline float Clamp(float v) { return v<0?0:(v>1?1:v); }
inline float Band(float d,float width=22) { return Clamp(1-std::fabs(d)/width); }
inline float Square(float x,float y,float size) {
    return Band((std::fabs(x)>std::fabs(y)?std::fabs(x):std::fabs(y))-size);
}
inline float Strength(Effect effect,float x,float y,float seconds,
                      bool top=false,Link link=Link::Sync,float speed=1) {
    if(!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(seconds)||!std::isfinite(speed)) return 0;
    if(top&&link==Link::Mirror) x=-x;
    const float t=(seconds-(top&&link==Link::Delay?.5f:0))*speed;
    const float direction=top&&link==Link::Reverse?-1.f:1.f;
    const float u=t*direction,r=std::hypot(x,y),angle=std::atan2(y,x),a=u*.6f;
    float v=1;
    switch(effect) {
    case Effect::Solid: break;
    case Effect::Rotate: v=Band(-x*std::sin(u*Pi/2)+y*std::cos(u*Pi/2),14); break;
    case Effect::Ripple: {
        const float phase=t/2-std::floor(t/2);
        v=Band(r-(direction<0?1-phase:phase)*160,16); break;
    }
    case Effect::WaveX: {float w=.5f+.5f*std::sin(x*.045f-u*1.8f);v=.08f+.92f*w*w;break;}
    case Effect::WaveDiagonal: {float w=.5f+.5f*std::sin((x+y)*.035f-u*1.8f);v=.08f+.92f*w*w;break;}
    case Effect::Sine: v=Band(y-38*std::sin(x*.035f-u),26);break;
    case Effect::Ocean: v=.12f+.48f*(.5f+.5f*std::sin(x*.035f+y*.022f-u))+
        .4f*(.5f+.5f*std::sin(y*.05f-x*.018f-u*1.4f));break;
    case Effect::SquareMove: v=Square(x-105*std::sin(u*.65f),y,40);break;
    case Effect::SquareExpand: v=Square(x,y,(u/4-std::floor(u/4))*150);break;
    case Effect::SquareRotate: v=Square(x*std::cos(a)+y*std::sin(a),-x*std::sin(a)+y*std::cos(a),62);break;
    case Effect::Radar: {float d=a-angle;d-=std::floor(d/Tau)*Tau;
        v=std::exp(-d*3)*(.25f+.75f*Clamp(r/40));break;}
    case Effect::Spiral: {float phase=angle-r*.045f+u;
        v=Band(std::atan2(std::sin(phase),std::cos(phase)),.3f*Pi);break;}
    case Effect::CornerRelay: {
        static const float centers[4][2]={{-78,78},{-78,-78},{78,-78},{78,78}};
        v=0;
        for(unsigned i=0;i<4;++i) {
            const float dx=x-centers[i][0],dy=y-centers[i][1];
            const float w=Clamp(std::cos(u*1.5f-i*Pi/2));
            const float light=std::exp(-(dx*dx+dy*dy)/3200)*w*w*w;
            if(light>v) v=light;
        }break;
    }
    case Effect::Aurora: v=.12f+.55f*(.5f+.5f*std::sin(x*.024f+u*.35f+1.8f*std::sin(y*.018f-u*.25f)))+
        .3f*(.5f+.5f*std::sin(y*.04f-x*.015f+u*.45f));break;
    }
    return Clamp(v);
}
inline uint32_t ColorAt(Effect effect,float x,float y,float seconds,uint32_t color,
                        bool top=false,Link link=Link::Sync) {
    const float v=Strength(effect,x,y,seconds,top,link);
    return (static_cast<uint32_t>(((color>>16)&255)*v)<<16)|
        (static_cast<uint32_t>(((color>>8)&255)*v)<<8)|static_cast<uint32_t>((color&255)*v);
}
inline Effect PlaylistEffect(unsigned frame) {
    static const Effect effects[]={Effect::WaveX,Effect::WaveDiagonal,Effect::Sine,Effect::Ocean,
        Effect::Ripple,Effect::SquareMove,Effect::SquareExpand,Effect::SquareRotate,
        Effect::Radar,Effect::Spiral,Effect::CornerRelay,Effect::Aurora};
    return effects[(frame/FramesPerEffect)%12];
}
template<class Strip> void DrawPlaylist(Strip& strip,unsigned frame) {
    static const uint32_t colors[]={0x57d9ff,0xa78bfa,0xff7096,0x55cbb8,0x57d9ff,0xffc46b,
        0xa78bfa,0xff7096,0x66e3a4,0xa78bfa,0xffc46b,0x66e3a4};
    const unsigned index=(frame/FramesPerEffect)%12;
    const float seconds=(frame%FramesPerEffect)*FrameMs/1000.f;
    for(unsigned i=0;i<LedPhysicalMap::Count;++i) {
        const auto p=LedPhysicalMap::Get(i);
        strip.setPixelColor(i,ColorAt(PlaylistEffect(frame),p.x,p.y,seconds,colors[index]));
    }
    strip.show();
}
}
