#include "Lighting/SynchronizedStrips.h"
#include "Configs/LightingTestConfig.h"
#include <cassert>
struct Strip {
 unsigned pixels[133]={};unsigned shown=0,brightness=0,begun=0;
 void begin(){++begun;}void setBrightness(unsigned v){brightness=v;}
 void clear(){for(auto& p:pixels)p=0;}void show(){++shown;}
 void setPixelColor(unsigned i,unsigned color){assert(i<133);pixels[i]=color;}
};
int main(){
 static_assert(LightingTestConfig::Pin==17,"bottom pin");
 static_assert(LightingTestConfig::TopPin==5,"top pin");
 Strip bottom,top;SynchronizedStrips<Strip> pair(bottom,top);
 pair.begin();pair.setBrightness(26);pair.clear();
 for(unsigned i=0;i<133;++i)pair.setPixelColor(i,i+1);
 pair.show();assert(bottom.begun==1&&top.begun==1);
 assert(bottom.brightness==26&&top.brightness==26);
 assert(bottom.shown==1&&top.shown==1);
 for(unsigned i=0;i<133;++i)assert(bottom.pixels[i]==top.pixels[i]&&top.pixels[i]==i+1);
 pair.clear();assert(bottom.pixels[132]==0&&top.pixels[132]==0);
}
