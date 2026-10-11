#include "UiPages.h"
#include "../UiContext.h"
#include "../MotorPageState.h"
#include "DRV8837.h"
#include <cstdio>

extern Motor motor;
namespace {
MotorPageState state;
int appliedDuty = 0;
void Apply() {
    const int duty = state.Duty();
    if (duty != appliedDuty) { motor.drive(duty); appliedDuty = duty; }
}
}
void UiPages::MotorStop() { state.Stop(); motor.stop(); appliedDuty = 0; }
void UiPages::MotorInit() {
    // Returning from speed adjustment preserves the session; entering from
    // the main menu is stopped because every exit stops the motor.
}
void UiPages::MotorProcess() {
    char run[24], direction[24], speed[24];
    std::snprintf(run, sizeof(run), "- Run       %s", state.running ? "ON" : "OFF");
    std::snprintf(direction, sizeof(direction), "- Dir       %s", state.reverse ? "REV" : "FWD");
    std::snprintf(speed, sizeof(speed), "- Speed     %d%%", state.speed);
    const M_SELECT menu[] = {{"[ Motor ]", MenuItemType::Back},
        {run, MenuItemType::Action}, {direction, MenuItemType::Action}, {speed, MenuItemType::Action}};
    OledUI::List_Show(menu, M_MOTOR);
    if (!btn.pressed) return;
    btn.pressed = false;
    if (btn.id == BTN_ID_CC || btn.id == BTN_ID_CW) RotateList();
    else if (btn.id == BTN_ID_LP) OledUI::Back();
    else if (btn.id == BTN_ID_SP) {
        switch (ui.select[ui.layer]) {
        case 0: OledUI::Back(); break;
        case 1: state.ToggleRun(); Apply(); break;
        case 2: state.ToggleDirection(); break;
        case 3: OledUI::NavigateTo(M_MOTOR_SPEED); break;
        }
    }
}
void UiPages::MotorSpeedProcess() {
    u8g2.setFont(LIST_FONT);
    u8g2.drawStr(8, 13, "Motor speed");
    char value[12]; std::snprintf(value, sizeof(value), "%d%%", state.speed);
    u8g2.drawStr(46, 32, value);
    u8g2.drawFrame(8, 38, 112, 8);
    u8g2.drawBox(10, 40, state.speed * 108 / 100, 4);
    u8g2.drawStr(8, 60, "UP/DOWN  OK:Back");
    if (!btn.pressed) return;
    btn.pressed = false;
    if (btn.id == BTN_ID_CW) state.AdjustSpeed(5);
    else if (btn.id == BTN_ID_CC) state.AdjustSpeed(-5);
    else if (btn.id == BTN_ID_SP || btn.id == BTN_ID_LP) OledUI::Back();
    Apply();
}
