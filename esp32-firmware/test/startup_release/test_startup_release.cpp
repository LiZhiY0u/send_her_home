#include <cassert>
#include <cstdint>
#include "Encoder/StartupReleaseGuard.h"
#include "Encoder/ButtonEvent.h"

uint32_t test_now = 0;
static int confirmations = 0;
static void OnButton(ButtonEvent*, int event) {
    if (event == ButtonEvent::EVENT_SHORT_CLICKED) ++confirmations;
}

int main() {
    StartupReleaseGuard guard;
    assert(!guard.Accept(true, 0));
    assert(!guard.Accept(true, 1000));
    assert(!guard.Accept(false, 1010));
    assert(!guard.Accept(false, 1059));
    assert(guard.Accept(false, 1060));
    assert(guard.Accept(true, 1070));
    assert(guard.Accept(false, 1080));

    StartupReleaseGuard bounce;
    assert(!bounce.Accept(false, 0));
    assert(!bounce.Accept(true, 49));
    assert(!bounce.Accept(false, 50));
    assert(!bounce.Accept(false, 99));
    assert(bounce.Accept(false, 100));

    StartupReleaseGuard wrap;
    assert(!wrap.Accept(false, UINT32_MAX - 20));
    assert(!wrap.Accept(false, 28));
    assert(wrap.Accept(false, 29));

    // Use the real event detector: the boot release must not become a click.
    StartupReleaseGuard startup;
    ButtonEvent button(3000);
    button.EventAttach(OnButton);
    const auto scan = [&](bool pressed, uint32_t now) {
        test_now = now;
        if (startup.Accept(pressed, now)) button.EventMonitor(pressed);
    };
    scan(true, 0);
    scan(true, 1000);
    scan(false, 1010);
    scan(false, 1060);
    assert(confirmations == 0);
    scan(true, 1100);
    scan(false, 1200);
    scan(false, 1210);
    assert(confirmations == 1);
}
