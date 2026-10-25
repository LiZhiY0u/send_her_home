#pragma once
#include "Lighting/SpatialEffects.h"
namespace PanelEffect {
inline bool Selected(unsigned index,uint8_t region){
 if(!region)return true;
 if(region==1)return index<33;
 if(region==2)return index>=33;
 if(region>6)return false;
 // Wire order: center, UL, LL, LR, UR; protocol: all,center,corners,UL,UR,LL,LR.
 const unsigned starts[]={33,108,58,83};return index>=starts[region-3]&&index<starts[region-3]+25;
}
inline uint32_t Color(unsigned index,bool top,const uint8_t* s,float seconds,uint32_t color){
 if((s[7]==1&&!top)||(s[7]==2&&top)||!Selected(index,s[8]))return 0;
 const auto p=LedPhysicalMap::Get(index);
 const float strength=SpatialEffects::Strength(static_cast<SpatialEffects::Effect>(s[0]),p.x,p.y,seconds,top,static_cast<SpatialEffects::Link>(s[9]),s[1]/50.f);
 return (static_cast<uint32_t>(((color>>16)&255)*strength)<<16)|(static_cast<uint32_t>(((color>>8)&255)*strength)<<8)|static_cast<uint32_t>((color&255)*strength);
}
}
