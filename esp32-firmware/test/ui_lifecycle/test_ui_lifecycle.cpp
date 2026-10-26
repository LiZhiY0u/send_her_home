#include <cassert>
#include "App/WakeGate.h"
#include "OledUI/UiContext.h"
#include "Input/InputQueue.h"
#include "Bluetooth/ConnectionEvents.h"
#include "DRV8837.h"
#include "OledUI/UiPages/UiPages.h"
#include "Lighting/PaintBridge.h"
#include "MotorControl/MotorService.h"
Motor motor;
uint32_t test_now=0;SerialFake Serial;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
namespace { uint8_t buffer[1024]={};unsigned displaySends=0,displayClears=0; }
namespace OledDriver {
void Init(){} void SendBuffer(){++displaySends;} void ClearBuffer(){++displayClears;}
void SetContrast(uint8_t){}
uint8_t* GetBufferPtr(){return buffer;} uint16_t GetBufferLen(){return 1024;}
}
namespace InputQueue {
bool Push(const UserInput::Event&){return true;}
bool Read(UserInput::Event&){return false;}
}
namespace ConnectionEvents { bool Read(Event&){return false;} }
namespace ENCODER { int16_t Encoder_GetDiff(){return 0;} }
int main() {
    WakeGate::Wake();
    OledUI::Init();
    assert(light_main_menu_count==6);
    OledUI::OpenRoot(M_LIGHT_MAIN);ui.select[0]=4;btn.id=BTN_ID_SP;btn.pressed=true;
    OledUI::Light_Main_Proc();assert(PaintBridge::LocalTest().automatic);assert(ui.index==M_LIGHT_MAIN);
    PaintBridge::LocalTest().accept=false;btn.pressed=true;
    OledUI::Light_Main_Proc();assert(ui.index==M_LIGHT_MAIN);
    PaintBridge::LocalTest().accept=true;btn.pressed=true;OledUI::Light_Main_Proc();
    assert(light_effect_menu_count==24);
    OledUI::OpenRoot(M_LIGHT_MAIN);ui.select[0]=3;btn.id=BTN_ID_SP;btn.pressed=true;
    OledUI::Light_Main_Proc();OledUI::Layer_Init_In();assert(ui.index==M_LIGHT_EFFECT);
    for(unsigned row=2;row<24;++row){
        ui.select[ui.layer]=row;btn.id=BTN_ID_SP;btn.pressed=true;
        OledUI::Light_Effect_Proc();assert(PaintBridge::LocalTest().effect==row-2);
        assert(!PaintBridge::LocalTest().automatic);
    }
    ui.select[ui.layer]=1;btn.pressed=true;OledUI::Light_Effect_Proc();assert(PaintBridge::LocalTest().automatic);
    OledUI::OpenRoot(M_MAIN);
    OledUI::OpenRoot(M_LIGHT_MAIN);ui.select[0]=1;btn.id=BTN_ID_SP;btn.pressed=true;
    OledUI::Light_Main_Proc();OledUI::Layer_Init_In();assert(ui.index==M_LIGHT_BRI);
    btn.id=BTN_ID_CC;btn.pressed=true;OledUI::Light_Bri_Proc();assert(PaintBridge::BrightnessTest()==95);
    for(int i=0;i<30;++i){btn.pressed=true;OledUI::Light_Bri_Proc();}
    assert(PaintBridge::BrightnessTest()==0);
    btn.id=BTN_ID_CW;btn.pressed=true;OledUI::Light_Bri_Proc();assert(PaintBridge::BrightnessTest()==5);
    OledUI::OpenRoot(M_MAIN);
    assert(ui.index==M_MAIN && ui.layer==0 && ui.state==S_NONE && tile.icon_x_trg==36);
    ui.select[0]=3;
    assert(OledUI::NavigateTo(M_SETTING));OledUI::Layer_Init_In();
    assert(ui.layer==1 && ui.index==M_SETTING && check_box.v==ui.param && check_box.m==ui.param);
    ui.param[DISP_BRI]=91;
    assert(OledUI::NavigateTo(M_ANIM_CURVE));OledUI::Layer_Init_In();
    assert(ui.layer==2 && check_box.s==&ui.param[ANIM_CURVE] && check_box.s_p==&ui.param[ANIM_CURVE_POS]);
    assert(OledUI::Back());OledUI::Layer_Init_Out();
    assert(ui.layer==1 && ui.index==M_SETTING && ui.param[DISP_BRI]==91);
    assert(OledUI::Back());OledUI::Layer_Init_Out();
    assert(ui.layer==0 && ui.index==M_MAIN && ui.select[0]==3);
    OledUI::OpenRoot(M_SLEEP);assert(ui.sleep && ui.index==M_SLEEP);
    btn.pressed=true;OledUI::Sleep_Proc();
    assert(!ui.sleep && !btn.pressed && ui.index==M_MAIN && ui.select[0]==0);
    OledUI::OpenRoot(M_LIGHT_COLOR);auto* selection=check_box.s;
    assert(selection && selection==check_box.s_p);
    *selection=3;
    OledUI::OpenRoot(M_MAIN);OledUI::OpenRoot(M_LIGHT_COLOR);
    assert(check_box.s==selection && *selection==3);
    OledUI::OpenRoot(M_MAIN);
    assert(main_menu_count == 7 && main_icon_count == 7);
    ui.select[0]=6;btn.id=BTN_ID_SP;btn.pressed=true;
    OledUI::Main_Proc();OledUI::Layer_Init_In();assert(ui.index==M_CREATOR);
    ui.state=S_NONE;btn.id=BTN_ID_LP;btn.pressed=true;UiPages::CreatorProcess();
    OledUI::Layer_Init_Out();assert(ui.index==M_MAIN);
    OledUI::OpenRoot(M_MAIN);
    assert(main_icon_pic[2][0]==0xff); // Motor is directly after Light.
    assert(main_icon_pic[2][32]==0xef); // Preserve the inverted motor outline.
    ui.select[0]=0;btn.id=BTN_ID_SP;btn.pressed=true;
    const auto offCalls=PaintBridge::OffCalls();
    OledUI::Main_Proc();assert(ui.index==M_MAIN&&PaintBridge::OffCalls()==offCalls+1);
    ui.select[0]=1;btn.pressed=true;
    const auto starts=PaintBridge::StartCalls();
    OledUI::Main_Proc();assert(PaintBridge::StartCalls()==starts+1);
    OledUI::OpenRoot(M_MAIN);
    ui.select[0]=2;btn.pressed=true;
    OledUI::Main_Proc();OledUI::Layer_Init_In();assert(ui.index==M_MOTOR);
    ui.state = S_NONE; ui.init = true;
    ui.select[ui.layer] = 1; btn.id = BTN_ID_SP; btn.pressed = true;
    UiPages::MotorProcess(); assert(motor.duty == 153);
    ui.select[ui.layer] = 2; btn.pressed = true;
    UiPages::MotorProcess(); assert(motor.duty == 153);
    assert(OledUI::NavigateTo(M_MOTOR_SPEED)); OledUI::Layer_Init_In();
    btn.id = BTN_ID_CW; btn.pressed = true;
    UiPages::MotorSpeedProcess(); assert(motor.duty == 165);
    btn.id = BTN_ID_SP; btn.pressed = true;
    UiPages::MotorSpeedProcess(); OledUI::Layer_Init_Out();
    assert(ui.index == M_MOTOR && motor.duty == 165);
    assert(MotorService::State().keepRunning);
    assert(OledUI::Back()); assert(motor.duty == 165);
    OledUI::Layer_Init_Out();
    assert(OledUI::NavigateTo(M_MOTOR)); OledUI::Layer_Init_In();
    assert(motor.duty==165&&ui.num[M_MOTOR]==5);
    ui.select[ui.layer]=4;btn.id=BTN_ID_SP;btn.pressed=true;UiPages::MotorProcess();
    assert(!MotorService::State().keepRunning&&motor.duty==165);
    assert(OledUI::Back());assert(motor.duty==0);OledUI::Layer_Init_Out();
    assert(OledUI::NavigateTo(M_MOTOR));OledUI::Layer_Init_In();
    assert(motor.duty == 0);
    assert(OledUI::NavigateTo(M_MOTOR_SPEED));OledUI::Layer_Init_In();
    MotorService::State().remote=true;MotorService::State().running=true;
    motor.drive(MotorService::State().Duty());
    btn.id=BTN_ID_SP;btn.pressed=true;UiPages::MotorSpeedProcess();
    assert(MotorService::State().remote);OledUI::Layer_Init_Out();
    assert(OledUI::Back());assert(motor.duty!=0);
    OledUI::OpenRoot(M_LIGHT_MAIN);ui.select[0]=5;btn.id=BTN_ID_SP;btn.pressed=true;
    const unsigned sends=displaySends,clears=displayClears;const auto epoch=WakeGate::Epoch();
    OledUI::Light_Main_Proc();
    assert(!WakeGate::Awake()&&WakeGate::Epoch()==epoch+1);
    assert(ui.index==M_MAIN&&ui.layer==0&&!btn.pressed);
    assert(displaySends==sends+1&&displayClears==clears+1);
    assert(motor.duty!=0); // Lighting reset must not stop remote-owned motor motion.
    OledUI::Task();assert(displaySends==sends+1);
    WakeGate::Wake();OledUI::Task();assert(displaySends==sends+2);
}
