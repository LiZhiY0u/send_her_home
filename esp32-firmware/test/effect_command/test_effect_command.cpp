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
}
