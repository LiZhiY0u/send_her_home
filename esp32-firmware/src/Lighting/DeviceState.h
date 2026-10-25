#pragma once
#include <atomic>
#include <cstdint>
namespace LightingState {
// Single writer: lighting task. Readers never block a real-time task.
struct Snapshot {uint8_t mode=0,settings[10]={1,50,100,0x57,0xd9,0xff,1,0,0,0};uint16_t elapsed=0;};
struct Storage {std::atomic<uint32_t> version{0},words[4];Storage(){Snapshot s;words[0]=s.mode|(s.settings[0]<<8)|(s.settings[1]<<16)|(s.settings[2]<<24);words[1]=0x01ffd957;words[2]=0;words[3]=0;}};
inline Storage& Data(){static Storage data;return data;}
inline void Publish(const Snapshot& s){auto& d=Data();d.version.fetch_add(1);d.words[0]=s.mode|(s.settings[0]<<8)|(s.settings[1]<<16)|(static_cast<uint32_t>(s.settings[2])<<24);d.words[1]=s.settings[3]|(s.settings[4]<<8)|(s.settings[5]<<16)|(static_cast<uint32_t>(s.settings[6])<<24);d.words[2]=s.settings[7]|(s.settings[8]<<8)|(s.settings[9]<<16);d.words[3]=s.elapsed;d.version.fetch_add(1);}
inline bool Read(Snapshot& s){auto& d=Data();for(unsigned attempt=0;attempt<3;++attempt){const auto v=d.version.load();if(v&1)continue;uint32_t w[4];for(unsigned i=0;i<4;++i)w[i]=d.words[i].load();if(d.version.load()!=v)continue;s.mode=w[0]&255;for(unsigned i=0;i<3;++i)s.settings[i]=(w[0]>>(8*(i+1)))&255;for(unsigned i=0;i<4;++i)s.settings[3+i]=(w[1]>>(8*i))&255;for(unsigned i=0;i<3;++i)s.settings[7+i]=(w[2]>>(8*i))&255;s.elapsed=w[3];return true;}return false;}
}
