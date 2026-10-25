#pragma once
#include <atomic>
#include <cstdint>
// Awake latch plus standby generation, shared by task-context inputs and Bluetooth.
namespace WakeGate {
inline std::atomic<uint32_t>& Latch(){static std::atomic<uint32_t> state(0);return state;}
inline bool Awake(){return (Latch().load(std::memory_order_acquire)&1)!=0;}
inline uint32_t Epoch(){return Latch().load(std::memory_order_acquire)>>1;}
inline bool Wake(){auto& state=Latch();auto old=state.load();while(!(old&1)){if(state.compare_exchange_weak(old,old|1))return true;}return false;}
inline bool WakeIfEpoch(uint32_t epoch,bool* first=nullptr){
    if(first)*first=false;
    auto& state=Latch();auto old=state.load();
    while((old>>1)==epoch){
        if(old&1)return true;
        if(state.compare_exchange_weak(old,old|1)){if(first)*first=true;return true;}
    }
    return false;
}
inline void Reset(){auto& state=Latch();auto old=state.load();while(!state.compare_exchange_weak(old,(old+2)&~uint32_t(1))){} }
}
