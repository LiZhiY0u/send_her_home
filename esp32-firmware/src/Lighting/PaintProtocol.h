#pragma once
#include <cstdint>
#include <cstring>
namespace PaintProtocol {
inline uint16_t Crc(const uint8_t* bytes,unsigned count) {
    uint16_t crc=0xffff;
    for(unsigned i=0;i<count;++i) { crc^=static_cast<uint16_t>(bytes[i])<<8;
        for(unsigned bit=0;bit<8;++bit) crc=(crc&0x8000)?static_cast<uint16_t>((crc<<1)^0x1021):static_cast<uint16_t>(crc<<1); }
    return crc;
}
class Receiver {
public:
    uint8_t Accept(const uint8_t* p,unsigned n,uint32_t now) {
        if(!p||n<8||p[0]!=0xa7||p[1]!=1) return 1;
        const uint16_t seq=p[4]|(static_cast<uint16_t>(p[5])<<8);
        if(p[2]==1) {
            if(n!=8||p[3]==0) return 1;
            Disconnect(); session_=p[3]; return 0;
        }
        if(session_==0||p[3]!=session_) return 2;
        if(p[2]==6) {
            if((n!=14&&n!=15)||p[6]||p[7]||p[8]>=22||p[9]>100||p[10]>100||(n==15&&p[14]>1)) return 1;
            if(haveSequence_&&static_cast<int16_t>(seq-sequence_)<=0) return 2;
            sequence_=seq;haveSequence_=true;active_=ready_=auto_=false;effect_=true;
            std::memcpy(effectSettings_,p+8,6);effectSettings_[6]=n==15?p[14]:0;return 0;
        }
        if(p[2]==5) { if(n!=8) return 1; active_=false;ready_=false;auto_=true;sequence_=seq;return 0; }
        if(p[2]==2) {
            if(n!=8||ready_) return 3;
            if(haveSequence_&&static_cast<int16_t>(seq-sequence_)<=0) return 2;
            sequence_=seq;haveSequence_=true;active_=true;stamp_=now;
            std::memset(seen_,0,sizeof(seen_));return 0;
        }
        if(!active_||seq!=sequence_||static_cast<uint32_t>(now-stamp_)>5000) {active_=false;return 2;}
        if(p[2]==3) {
            const unsigned start=p[6], count=p[7];
            if(count<1||count>4||start+count>133||n!=8+count*3) return 1;
            for(unsigned i=0;i<count;++i) {
                if(seen_[start+i]&&std::memcmp(pixels_+(start+i)*3,p+8+i*3,3)!=0) return 1;
                std::memcpy(pixels_+(start+i)*3,p+8+i*3,3);seen_[start+i]=true;
            }
            stamp_=now;return 0;
        }
        if(p[2]==4) {
            if(n!=10) return 1;
            for(bool seen:seen_) if(!seen) return 4;
            if(Crc(pixels_,399)!=(p[8]|(static_cast<uint16_t>(p[9])<<8))) return 5;
            active_=false;ready_=true;return 0;
        }
        return 1;
    }
    bool Ready() const { return ready_; }
    const uint8_t* Pixels() const { return pixels_; }
    uint8_t Session() const { return session_; }
    uint16_t Sequence() const { return sequence_; }
    bool AutoRequested() const { return auto_; }
    bool EffectRequested() const {return effect_;}
    const uint8_t* EffectSettings() const {return effectSettings_;}
    void ClearReady() { ready_=false;auto_=false;effect_=false; }
    void Disconnect() {active_=ready_=auto_=effect_=haveSequence_=false;session_=0;}
private:
    uint8_t pixels_[399]={},session_=0;
    uint8_t effectSettings_[7]={};bool effect_=false;
    bool seen_[133]={},active_=false,ready_=false,auto_=false,haveSequence_=false;
    uint16_t sequence_=0;uint32_t stamp_=0;
};
}
