#include "Lighting/PaintProtocol.h"
#include <cassert>
int main(){
 PaintProtocol::Receiver r;
 uint8_t hello[]={0xa7,1,1,7,0,0,0,0};assert(r.Accept(hello,8,0)==0);
 uint8_t p[]={0xa7,1,6,7,1,0,0,0,5,50,80,255,128,0};
 assert(r.Accept(p,14,0)==0);assert(r.EffectRequested());assert(r.EffectSettings()[0]==5);
 r.ClearReady();assert(!r.EffectRequested());
 assert(r.Accept(p,14,0)==2);p[4]=2;p[8]=21;assert(r.Accept(p,14,0)==0);
 p[4]=3;p[8]=22;assert(r.Accept(p,14,0)==1);
 p[8]=5;p[9]=101;assert(r.Accept(p,14,0)==1);
 uint8_t random[]={0xa7,1,6,7,4,0,0,0,5,50,65,255,0,0,1};
 assert(r.Accept(random,15,0)==0);assert(r.EffectSettings()[6]==1);r.ClearReady();
 random[4]=5;random[14]=2;assert(r.Accept(random,15,0)==1);
 random[14]=0;assert(r.Accept(random,14,0)==0);assert(r.EffectSettings()[6]==0);
 uint8_t extended[]={0xa7,1,6,7,6,0,0,0,3,50,80,255,128,0,1,2,6,3};
 assert(r.Accept(extended,18,0)==0);assert(r.EffectSettings()[7]==2&&r.EffectSettings()[8]==6&&r.EffectSettings()[9]==3);
 extended[4]=7;extended[15]=3;assert(r.Accept(extended,18,0)==1);
 extended[15]=0;extended[16]=7;assert(r.Accept(extended,18,0)==1);
 extended[16]=0;extended[17]=4;assert(r.Accept(extended,18,0)==1);
}
