#include <cassert>
#include "OledUI/MotorPageState.h"
int main() {
    MotorPageState state;
    assert(!state.running && state.speed == 60 && state.Duty() == 0);
    state.ToggleRun(); assert(state.Duty() == 153);
    state.ToggleDirection(); assert(!state.reverse);
    state.AdjustSpeed(5); assert(state.speed == 65 && state.Duty() == 165);
    state.Stop(); state.ToggleDirection(); state.ToggleRun(); assert(state.Duty() == -165);
    state.Stop(); assert(state.Duty() == 0);
    for (int i=0;i<30;++i) state.AdjustSpeed(-5);
    assert(state.speed == 0); state.ToggleRun(); assert(state.Duty() == 0);
    for (int i=0;i<30;++i) state.AdjustSpeed(5);
    assert(state.speed == 100 && state.Duty() == -255);
}
