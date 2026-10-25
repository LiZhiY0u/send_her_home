#pragma once
#include <cstdint>
#include "Lighting/DeviceState.h"
namespace PaintBridge {
struct LocalTestState {unsigned calls=0;uint8_t effect=255;bool automatic=false;bool accept=true;};
inline LocalTestState& LocalTest(){static LocalTestState state;return state;}
inline bool SelectLocalEffect(uint8_t effect,bool automatic=false){auto& s=LocalTest();++s.calls;s.effect=effect;s.automatic=automatic;return s.accept;}
inline bool SetLocalRandomColor(bool){return true;}
inline int& BrightnessTest(){static int v=-1;return v;}
inline bool SetLocalBrightness(uint8_t value){BrightnessTest()=value;if(LocalTest().accept){LightingState::Snapshot s;LightingState::Read(s);s.settings[2]=value;LightingState::Publish(s);}return LocalTest().accept;}
}
