#pragma once
#include <cstdint>

class StartupReleaseGuard {
public:
    // Ignore the power-on press until the button stays released for 50 ms.
    bool Accept(bool pressed, uint32_t now) {
        if (ready_) return true;
        if (pressed) {
            releasing_ = false;
            return false;
        }
        if (!releasing_) {
            releasing_ = true;
            releasedAt_ = now;
        }
        ready_ = static_cast<uint32_t>(now - releasedAt_) >= 50;
        return ready_;
    }
private:
    bool ready_ = false;
    bool releasing_ = false;
    uint32_t releasedAt_ = 0;
};
