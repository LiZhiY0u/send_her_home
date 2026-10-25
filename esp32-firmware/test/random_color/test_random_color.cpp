#include "Lighting/RandomColor.h"
#include <cassert>
int main(){
 const uint32_t base=0xff0000;
 assert(RandomColor::At(0,base)==base);
 assert(RandomColor::At(1000,base)==base);
 assert(RandomColor::At(5000,base)!=base);
 assert(RandomColor::At(6000,base)==RandomColor::At(5000,base));
 for(unsigned i=0;i<3;++i){unsigned shift=i*8;unsigned a=(base>>shift)&255,b=(RandomColor::At(5000,base)>>shift)&255,m=(RandomColor::At(3000,base)>>shift)&255;assert(m==(a+b)/2);}
}
