#pragma once
#include <atomic>
// One latch per boot, shared by task-context input and Bluetooth callbacks.
namespace WakeGate {
inline std::atomic<bool>& Latch(){static std::atomic<bool> awake(false);return awake;}
inline bool Awake(){return Latch().load(std::memory_order_acquire);}
inline bool Wake(){return !Latch().exchange(true,std::memory_order_acq_rel);}
}
