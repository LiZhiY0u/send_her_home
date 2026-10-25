#include "../UiContext.h"
#include "UiPages.h"
#include "../MainPageState.h"
#include "../UiNavigation.h"
#include "HAL/HAL.h"
#include "Lighting/PaintBridge.h"
#include "Lighting/DeviceState.h"
#include <cstdio>
namespace {
uint8_t light_brightness = 100;  // Relative to the fixed 26/255 hardware cap.
uint8_t light_color_r = 255;       // RGB颜色，待用户实现
uint8_t light_color_g = 255;
uint8_t light_color_b = 255;
uint8_t light_effect_id = 0;       // 当前效果ID 0-6，待用户实现效果函数
// 效果名称表 (供显示使用)
const char* light_effect_name[] = {
    "Static",   // 0
    "Breath",   // 1
    "Rainbow",  // 2
    "Wave",     // 3
    "Strobe",   // 4
    "Comet",    // 5
    "Sparkle",  // 6
};
static uint8_t light_color_select = 0;
bool localRandomColor=false;
bool restoreRandomBusy=false;
} // namespace
void UiPages::Light_Main_Param_Init()
{
    // All entries are actions; restoration is handled by the lighting task.
}

void UiPages::Light_Bri_Param_Init()
{
    OledUI::CheckBox_Value_Init(&light_brightness);
}

void UiPages::Light_Color_Param_Init()
{
    OledUI::CheckBox_Single_Init(&light_color_select, &light_color_select);
}

void UiPages::Light_Effect_Param_Init()
{
    // Actions, not cached selection: BLE can independently change the effect.
}

void OledUI::Light_Main_Show()
{
    LightingState::Snapshot state;if(LightingState::Read(state)){light_brightness=state.settings[2];localRandomColor=state.settings[6]!=0;}
    const M_SELECT menu[]={{"[ Light ]",MenuItemType::Back},
        {"- Brightness",MenuItemType::Action},
        {localRandomColor?"- Random Color ON":"- Random Color OFF",MenuItemType::Action},
        {"- Effect",MenuItemType::Action},
        {restoreRandomBusy?"- Retry Random":"- Restore Random",MenuItemType::Action}};
    List_Show(menu, M_LIGHT_MAIN);
}

void OledUI::Light_Main_Proc()
{
    Light_Main_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  // Return
                Back();
                break;
            case 1:
                NavigateTo(M_LIGHT_BRI);
                break;
            case 2:
                if(PaintBridge::SetLocalRandomColor(!localRandomColor))localRandomColor=!localRandomColor;
                break;
            case 3:
                NavigateTo(M_LIGHT_EFFECT);
                break;
            case 4:
                restoreRandomBusy=!PaintBridge::SelectLocalEffect(0,true);
                break;
            }
            break;
        }
    }
}

void OledUI::Light_Bri_Show()
{
    LightingState::Snapshot state;if(LightingState::Read(state))light_brightness=state.settings[2];
    u8g2.setFont(LIST_FONT);
    u8g2.drawStr(8,13,"Light brightness");
    char value[12];std::snprintf(value,sizeof(value),"%u%%",light_brightness);
    u8g2.drawStr(46,32,value);
    u8g2.drawFrame(8,38,112,8);
    u8g2.drawBox(10,40,light_brightness*108/100,4);
    u8g2.drawStr(8,60,"UP/DOWN  OK:Back");
}

void OledUI::Light_Bri_Proc()
{
    Light_Bri_Show();
    static bool busy=false;
    if(busy)u8g2.drawStr(90,13,"Retry");
    if(!btn.pressed)return;
    btn.pressed=false;
    if(btn.id==BTN_ID_SP||btn.id==BTN_ID_LP){Back();return;}
    int value=light_brightness;
    if(btn.id==BTN_ID_CW)value+=5;
    else if(btn.id==BTN_ID_CC)value-=5;
    else return;
    if(value<0)value=0;
    if(value>100)value=100;
    busy=!PaintBridge::SetLocalBrightness(value);
    if(!busy)light_brightness=value;
}

void OledUI::Light_Color_Show()
{
    List_Show(light_color_menu, M_LIGHT_COLOR);
}

void OledUI::Light_Color_Proc()
{
    Light_Color_Show();

    if (btn.pressed)
    {
        btn.pressed = false;
        switch (btn.id)
        {
        case BTN_ID_CW:
        case BTN_ID_CC:
            UiPages::RotateList();
            break;
        case BTN_ID_LP:
            ui.select[ui.layer] = 0;
        case BTN_ID_SP:
            switch (ui.select[ui.layer])
            {
            case 0:  // Return
                Back();
                break;
            // ========== 用户实现区 ==========
            // case 1: light_color_r=255; light_color_g=0;   light_color_b=0;   ApplyColor(); break;  // Red
            // case 2: light_color_r=0;   light_color_g=255; light_color_b=0;   ApplyColor(); break;  // Green
            // case 3: light_color_r=0;   light_color_g=0;   light_color_b=255; ApplyColor(); break;  // Blue
            // case 4: light_color_r=255; light_color_g=255; light_color_b=255; ApplyColor(); break;  // White
            // case 5: NavigateTo(M_LIGHT_COLOR_CUSTOM); break; // Custom RGB 预留
            // ==================================
            }
            break;
        }
    }
}

void OledUI::Light_Effect_Show()
{
    List_Show(light_effect_menu, M_LIGHT_EFFECT);
}

void OledUI::Light_Effect_Proc()
{
    Light_Effect_Show();
    static bool busy=false;
    if(busy)u8g2.drawStr(72,63,"Retry");
    if(!btn.pressed)return;
    btn.pressed=false;
    if(btn.id==BTN_ID_CW||btn.id==BTN_ID_CC)UiPages::RotateList();
    else if(btn.id==BTN_ID_LP)Back();
    else if(btn.id==BTN_ID_SP) {
        const uint8_t row=ui.select[ui.layer];
        if(row==0){busy=false;Back();}
        else if(row<24)busy=!PaintBridge::SelectLocalEffect(row==1?0:row-2,row==1);
    }
}
