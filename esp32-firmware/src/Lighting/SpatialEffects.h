#pragma once
#include <cmath>
#include <cstdint>
#include "Diagnostics/LedPhysicalMap.h"

// Pure spatial sampler: no allocation, GPIO access or RTOS dependency.
namespace SpatialEffects {
constexpr unsigned Count=22, FrameMs=20, FramesPerEffect=600;
constexpr unsigned PlaylistCount=20;
constexpr float Pi=3.14159265359f, Tau=2*Pi;
enum class Effect { Solid, Rotate, Ripple, WaveX, WaveDiagonal, Sine, Ocean,
    SquareMove, SquareExpand, SquareRotate, Radar, Spiral, CornerRelay, Aurora,
    Inward, Collision, OffCenter, Diamond, Bounce, Windmill, Petals, DiagonalBreathe };
enum class Link { Sync, Reverse, Mirror, Delay };
inline float Clamp(float v) { return v<0?0:(v>1?1:v); }
inline float Band(float d,float width=22) { return Clamp(1-std::fabs(d)/width); }
inline float Contrast(float value) {const float v=Clamp((value-.22f)/.78f);return v*v*v;}
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
    case Effect::Inward: v=Band(r-(1-(u/3-std::floor(u/3)))*160,20);break;
    case Effect::Collision: v=Band(std::fabs(x)-std::fabs(100*std::cos(u*.8f)),20);break;
    case Effect::OffCenter: v=Band(std::hypot(x+65,y-65)-(u/4-std::floor(u/4))*250,22);break;
    case Effect::Diamond: v=Band(std::fabs(x)+std::fabs(y)-(u/3-std::floor(u/3))*200,24);break;
    case Effect::Bounce: {
        const float dx=std::fabs(x-65*std::sin(u*.8f)),dy=std::fabs(y-65*std::sin(u*1.1f));
        v=Clamp(1-(dx>dy?dx:dy)/32);break;
    }
    case Effect::Windmill: {const float phase=2*(angle-u)-r*.025f;
        v=Band(std::atan2(std::sin(phase),std::cos(phase)),.45f*Pi);break;}
    case Effect::Petals: v=Band(r-(60+35*std::cos(angle*4+u)),20)*Contrast(.5f+.5f*std::sin(u*.7f));break;
    case Effect::DiagonalBreathe: v=Contrast(.5f+.5f*std::sin(u*1.4f+(x*y>=0?0:Pi)));break;
    case Effect::Solid: break;
    case Effect::Rotate: v=Band(-x*std::sin(u*Pi/2)+y*std::cos(u*Pi/2),14); break;
    case Effect::Ripple: {
        const float phase=t/2-std::floor(t/2);
        v=Band(r-(direction<0?1-phase:phase)*160,16); break;
    }
    case Effect::WaveX: v=Contrast(.5f+.5f*std::sin(x*.045f-u*1.8f));break;
    case Effect::WaveDiagonal: v=Contrast(.5f+.5f*std::sin((x+y)*.035f-u*1.8f));break;
    case Effect::Sine: v=Band(y-38*std::sin(x*.035f-u),26);break;
    case Effect::Ocean: v=Contrast(.55f*(.5f+.5f*std::sin(x*.035f+y*.022f-u))+
        .45f*(.5f+.5f*std::sin(y*.05f-x*.018f-u*1.4f)));break;
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
    case Effect::Aurora: v=Contrast(.65f*(.5f+.5f*std::sin(x*.024f+u*.35f+1.8f*std::sin(y*.018f-u*.25f)))+
        .35f*(.5f+.5f*std::sin(y*.04f-x*.015f+u*.45f)));break;
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
        Effect::Radar,Effect::Spiral,Effect::CornerRelay,Effect::Aurora,
        Effect::Inward,Effect::Collision,Effect::OffCenter,Effect::Diamond,
        Effect::Bounce,Effect::Windmill,Effect::Petals,Effect::DiagonalBreathe};
    return effects[(frame/FramesPerEffect)%PlaylistCount];
}
template<class Strip> void DrawPlaylist(Strip& strip,unsigned frame) {
    static const uint32_t colors[]={0x57d9ff,0xa78bfa,0xff7096,0x55cbb8,0x57d9ff,0xffc46b,
        0xa78bfa,0xff7096,0x66e3a4,0xa78bfa,0xffc46b,0x66e3a4};
    const unsigned index=(frame/FramesPerEffect)%12; // Cycle the palette independently of effects.
    const float seconds=(frame%FramesPerEffect)*FrameMs/1000.f;
    for(unsigned i=0;i<LedPhysicalMap::Count;++i) {
        const auto p=LedPhysicalMap::Get(i);
        strip.setPixelColor(i,ColorAt(PlaylistEffect(frame),p.x,p.y,seconds,colors[index]));
    }
    strip.show();
}
}
