#include <cassert>
#include "Lighting/PanelEffect.h"
#include "Lighting/DeviceState.h"
int main(){
 uint8_t s[10]={0,50,65,255,0,0,0,0,0,0};
 for(unsigned i=0;i<133;++i){assert(PanelEffect::Color(i,false,s,0,0xff0000)==0xff0000);assert(PanelEffect::Color(i,true,s,0,0xff0000)==0xff0000);}
 s[7]=1;assert(PanelEffect::Color(0,false,s,0,0xff0000)==0);assert(PanelEffect::Color(0,true,s,0,0xff0000)==0xff0000);
 s[7]=2;assert(PanelEffect::Color(0,true,s,0,0xff0000)==0);assert(PanelEffect::Color(0,false,s,0,0xff0000)==0xff0000);
 const unsigned counts[]={133,33,100,25,25,25,25};
 for(unsigned r=0;r<7;++r){unsigned count=0;for(unsigned i=0;i<133;++i)count+=PanelEffect::Selected(i,r);assert(count==counts[r]);}
 assert(PanelEffect::Selected(33,3));assert(PanelEffect::Selected(108,4));assert(PanelEffect::Selected(58,5));assert(PanelEffect::Selected(83,6));
 LightingState::Snapshot state;state.mode=3;for(unsigned i=0;i<10;++i)state.settings[i]=i;state.elapsed=1234;LightingState::Publish(state);
 LightingState::Snapshot read;assert(LightingState::Read(read));assert(read.mode==3&&read.elapsed==1234);for(unsigned i=0;i<10;++i)assert(read.settings[i]==i);
 s[7]=0;s[0]=3;s[8]=0;s[9]=0;
 assert(PanelEffect::Color(40,true,s,1,0xffffff)==PanelEffect::Color(40,false,s,1,0xffffff));
 s[9]=2;bool differs=false;for(unsigned i=0;i<133;++i)differs|=PanelEffect::Color(i,true,s,1,0xffffff)!=PanelEffect::Color(i,false,s,1,0xffffff);assert(differs);
}
