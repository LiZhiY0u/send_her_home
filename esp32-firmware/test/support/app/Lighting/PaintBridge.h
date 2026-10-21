#pragma once
#include <cstdint>
namespace PaintBridge {
struct Command {uint8_t session;uint16_t sequence;bool automatic;uint8_t pixels[399];bool effect=false;uint8_t settings[6]={};};
inline bool Read(Command&){return false;}
inline void Applied(const Command&){}
}
