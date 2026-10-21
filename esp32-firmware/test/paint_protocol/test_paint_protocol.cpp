#include <cassert>
#include <cstdint>
#include "Lighting/PaintProtocol.h"
int main() {
    const uint8_t known[]={'1','2','3','4','5','6','7','8','9'};
    assert(PaintProtocol::Crc(known,9)==0x29b1);
    static_assert(sizeof(PaintProtocol::Receiver)<1024,"Fixed receiver memory budget");
    PaintProtocol::Receiver rx;
    uint8_t hello[] = {0xa7,1,1,7,0,0,0,0};
    assert(rx.Accept(hello,8,0) == 0);
    uint8_t begin[] = {0xa7,1,2,7,1,0,0,0};
    assert(rx.Accept(begin,8,1) == 0);
    uint8_t data[20] = {0xa7,1,3,7,1,0,0,4};
    for(unsigned i=8;i<20;++i) data[i]=123;
    assert(rx.Accept(data,20,2) == 0);
    uint8_t commit[] = {0xa7,1,4,7,1,0,0,0,0,0};
    assert(rx.Accept(commit,10,3) != 0); // incomplete cannot become visible
    for(unsigned start=0;start<133;start+=4) {
        data[6]=start;data[7]=start+4<=133?4:1;
        assert(rx.Accept(data,8+data[7]*3,4) == 0);
    }
    uint8_t frame[399];for(auto& b:frame)b=123;
    const auto crc=PaintProtocol::Crc(frame,399);commit[8]=crc;commit[9]=crc>>8;
    assert(rx.Accept(commit,10,5) == 0 && rx.Ready());
    assert(rx.Pixels()[398] == 123);
    rx.ClearReady(); assert(!rx.Ready());
    assert(rx.Accept(commit,10,6) != 0);
    begin[4]=2;assert(rx.Accept(begin,8,7)==0);
    data[4]=1;assert(rx.Accept(data,11,8)!=0);
    rx.Disconnect();assert(rx.Accept(begin,8,9)!=0);
    assert(rx.Accept(hello,8,10)==0);begin[4]=3;assert(rx.Accept(begin,8,11)==0);
    data[4]=3;data[6]=132;data[7]=4;assert(rx.Accept(data,20,12)!=0);
    data[6]=0;data[7]=4;assert(rx.Accept(data,20,5012)!=0);
    begin[4]=4;assert(rx.Accept(begin,8,5013)==0);
    uint8_t automatic[]={0xa7,1,5,7,5,0,0,0};
    assert(rx.Accept(automatic,8,5014)==0&&rx.AutoRequested());
}
