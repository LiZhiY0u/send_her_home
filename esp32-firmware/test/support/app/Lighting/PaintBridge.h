#pragma once
#include <cstdint>
#include "App/WakeGate.h"
namespace PaintBridge {
struct Command {uint8_t session;uint16_t sequence;bool automatic;uint8_t pixels[399];bool effect=false;uint8_t settings[10]={};bool colorModeOnly=false;bool brightnessOnly=false;uint32_t epoch=WakeGate::Epoch();};
inline bool Read(Command&){return false;}
inline void Applied(const Command&){}
}
