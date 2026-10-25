#pragma once
#include <cstdint>
namespace RandomColor {
inline uint32_t Target(uint32_t index){
 uint32_t v=(index+1)*1664525u+1013904223u;v^=v>>16;
 static const uint32_t palette[]={0xff4682,0x3cc8ff,0xffb428,0x8c50ff,0x32ff82,0xff4628,0x4664ff};
 return palette[v%7];
}
inline uint32_t At(uint32_t ms,uint32_t base){
 const uint32_t cycle=ms/5000,phase=ms%5000;
 const uint32_t from=cycle?Target(cycle-1):base,to=Target(cycle);
 const int mix=phase>4000?phase-4000:0;
 uint32_t color=0;
 for(unsigned shift=0;shift<24;shift+=8){
  const int a=(from>>shift)&255,b=(to>>shift)&255;
  const unsigned channel=(a*(1000-mix)+b*mix)/1000;
  color|=channel<<shift;
 }
 return color;
}
}
