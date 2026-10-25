#include "AppTasks.h"
#include <Arduino.h>
#include "Configs/BoardConfig.h"
#include "Configs/TaskConfig.h"
#include "Encoder/HAL_Encoder.h"
#include "OledUI/OledUI.h"
#include "RGB.h"
#include "DRV8837.h"
#include "Configs/LightingTestConfig.h"
#include "Diagnostics/LedStripTest.h"
#include "Lighting/PaintBridge.h"
#include "Lighting/SpatialEffects.h"
#include "Lighting/SynchronizedStrips.h"
#include "Lighting/RandomColor.h"
#include "MotorControl/MotorService.h"
#include "App/WakeGate.h"
#include "Lighting/StartupSequence.h"
#include "Lighting/RandomEffect.h"
#include "Lighting/AutoColor.h"
#include "Lighting/DeviceState.h"
#include "Lighting/PanelEffect.h"
static_assert(configTICK_RATE_HZ == 1000, "Task migration requires original 1ms RTOS tick");
// Existing RGB implementation references this statically owned object.
Adafruit_NeoPixel RGB3(BoardConfig::RgbCount, BoardConfig::RgbPin, NEO_GRB + NEO_KHZ800);
// Constructor has no hardware side effects; initialize to coast on startup.
Motor motor(BoardConfig::MotorIn1, BoardConfig::MotorIn2,
            BoardConfig::MotorFrequency, BoardConfig::MotorChannel1, BoardConfig::MotorChannel2);
namespace {
TaskHandle_t handles[4] = {};
enum class State { NotStarted, Failed, Ready };
State state = State::NotStarted;
void Sampling(void*) {
    for (;;) { (void)analogRead(CONFIG_BAT_DET_PIN); vTaskDelay(pdMS_TO_TICKS(TaskConfig::SamplingMs)); }
}
void Lighting(void*) {
#if ENABLE_LED_STRIP_TEST
    Adafruit_NeoPixel bottomStrip(LedStripTest::PixelCount, LightingTestConfig::Pin, NEO_GRB + NEO_KHZ800);
    Adafruit_NeoPixel topStrip(LedStripTest::PixelCount, LightingTestConfig::TopPin, NEO_GRB + NEO_KHZ800);
    SynchronizedStrips<Adafruit_NeoPixel> testStrip(bottomStrip,topStrip);
    testStrip.begin(); testStrip.setBrightness(LedStripTest::Brightness);
    testStrip.clear(); testStrip.show();
    unsigned step = 0;
    bool painting = false;
    bool selectedEffect = false;
    uint8_t effectSettings[10]={1,50,100,0x57,0xd9,0xff,1,0,0,0};
    uint32_t effectStart=0,autoStart=0;
    bool autoStarted=false;
    bool randomColor=true;
    uint32_t colorStart=0,randomBase=0x57d9ff;
    static uint8_t paintedPixels[399]={};
    uint8_t outputBrightness=100;
    PaintBridge::Command command{};
    StartupSequence::State startup;
    RandomEffect::State randomEffects(millis());
    AutoColor::State automaticColors(millis()^0x85ebca6bu);
    unsigned automaticEffect=0;
    uint32_t shownColor=randomBase;
    for (;;) {
        if(!WakeGate::Awake()){vTaskDelay(pdMS_TO_TICKS(10));continue;}
        LightingState::Snapshot snapshot;
        snapshot.mode=painting?4:(selectedEffect?3:(autoStarted?2:1));
        for(unsigned i=0;i<10;++i)snapshot.settings[i]=effectSettings[i];
        snapshot.settings[2]=outputBrightness;snapshot.settings[6]=randomColor;
        if(!selectedEffect&&!painting){
            snapshot.settings[0]=automaticEffect;snapshot.settings[1]=50;
            snapshot.settings[3]=shownColor>>16;snapshot.settings[4]=shownColor>>8;snapshot.settings[5]=shownColor;
            snapshot.settings[7]=snapshot.settings[8]=snapshot.settings[9]=0;
        }
        snapshot.elapsed=static_cast<uint16_t>((millis()-(selectedEffect?effectStart:autoStart))/10);
        LightingState::Publish(snapshot);
        if(PaintBridge::Read(command)) {
            // An explicit user command takes priority and must receive its ACK promptly.
            startup.Finish();
            if(command.brightnessOnly){
                outputBrightness=command.settings[2];
                testStrip.setBrightness(LedStripTest::Brightness*outputBrightness/100);
                if(painting){
                    for(unsigned i=0;i<133;++i)testStrip.setPixelColor(i,
                        (static_cast<uint32_t>(paintedPixels[i*3])<<16)|
                        (static_cast<uint32_t>(paintedPixels[i*3+1])<<8)|paintedPixels[i*3+2]);
                    testStrip.show();
                }
                continue;
            }
            if(command.colorModeOnly){
                randomColor=command.settings[6]!=0;colorStart=millis();command.colorModeOnly=false;
                if(selectedEffect)effectStart=colorStart;
                if(!randomColor&&!selectedEffect)randomBase=shownColor;
                continue;
            }
            painting = !command.automatic&&!command.effect;
            if(command.effect) {
                selectedEffect=true;effectStart=millis();
                colorStart=effectStart;
                for(unsigned i=0;i<10;++i)effectSettings[i]=command.settings[i];
                if(command.session!=0){
                    outputBrightness=effectSettings[2];
                    testStrip.setBrightness(LedStripTest::Brightness*outputBrightness/100);
                }
                randomBase=(static_cast<uint32_t>(effectSettings[3])<<16)|(static_cast<uint32_t>(effectSettings[4])<<8)|effectSettings[5];
                if(command.session!=0){randomColor=command.settings[6]!=0;colorStart=millis();}
            } else if(command.automatic){selectedEffect=false;autoStarted=false;}
            if(painting) {
                for(unsigned i=0;i<399;++i)paintedPixels[i]=command.pixels[i];
                for(unsigned i=0;i<133;++i) testStrip.setPixelColor(i,
                    (static_cast<uint32_t>(command.pixels[i*3])<<16)|
                    (static_cast<uint32_t>(command.pixels[i*3+1])<<8)|command.pixels[i*3+2]);
                testStrip.show();
            }
            if(!command.effect) PaintBridge::Applied(command);
        }
        if(startup.Active(true,millis())){
            startup.Draw(testStrip,millis());
            vTaskDelay(pdMS_TO_TICKS(SpatialEffects::FrameMs));continue;
        }
        if(painting) {vTaskDelay(pdMS_TO_TICKS(10));continue;}
        if(selectedEffect) {
            const float seconds=static_cast<uint32_t>(millis()-effectStart)/1000.f;
            const uint32_t fixedColor=(static_cast<uint32_t>(effectSettings[3])<<16)|
                (static_cast<uint32_t>(effectSettings[4])<<8)|effectSettings[5];
            const uint32_t color=randomColor?RandomColor::At(millis()-colorStart,randomBase):fixedColor;
            for(unsigned i=0;i<LedPhysicalMap::Count;++i) {
                testStrip.setPanelPixelColor(i,PanelEffect::Color(i,false,effectSettings,seconds,color),PanelEffect::Color(i,true,effectSettings,seconds,color));
            }
            testStrip.show();
            if(command.effect){PaintBridge::Applied(command);command.effect=false;}
            vTaskDelay(pdMS_TO_TICKS(SpatialEffects::FrameMs));continue;
        }
#if ENABLE_LED_SPATIAL_TEST
        if(!autoStarted||static_cast<uint32_t>(millis()-autoStart)>=6000){autoStarted=true;autoStart=millis();automaticEffect=randomEffects.Next();}
        step=static_cast<uint32_t>(millis()-autoStart)/SpatialEffects::FrameMs;
        shownColor=randomColor?automaticColors.At(millis()):randomBase;
        SpatialEffects::DrawPlaylist(testStrip, step,shownColor,automaticEffect);
        vTaskDelay(pdMS_TO_TICKS(SpatialEffects::FrameMs));
#else
        LedStripTest::Draw(testStrip, step);
        vTaskDelay(pdMS_TO_TICKS(LedStripTest::DelayMs(step)));
        step = (step + 1) % LedStripTest::StepCount;
#endif
    }
#else
    RGB3.begin(); RGB3.show(); RGB3.setBrightness(255);
    for (;;) rainbow(5);
#endif
}
void Encoder(void*) {
    ENCODER::Encoder_Init();
    for (;;) { ENCODER::Encoder_Update(); vTaskDelay(pdMS_TO_TICKS(TaskConfig::EncoderMs)); }
}
void Display(void*) {
    OledUI::Init();
    for (;;) { MotorService::Tick(); OledUI::Task(); vTaskDelay(pdMS_TO_TICKS(TaskConfig::DisplayMs)); }
}
}
bool AppTasks::Start()
{
    if (state != State::NotStarted) return state == State::Ready;
    state = State::Failed;
    motor.begin();
    const TaskFunction_t functions[] = {Sampling, Lighting, Encoder, Display};
    const TaskConfig::Specification specifications[] = {
        TaskConfig::Sampling, TaskConfig::Lighting, TaskConfig::Encoder, TaskConfig::Display};
    for (unsigned i = 0; i < 4; ++i) {
        const auto& spec = specifications[i];
        if (xTaskCreatePinnedToCore(functions[i], spec.name, spec.stack, nullptr,
                                   spec.priority, &handles[i], spec.core) != pdPASS) {
            Serial.print("Task creation failed: "); Serial.println(spec.name);
            for (unsigned j = 0; j < i; ++j) { vTaskDelete(handles[j]); handles[j] = nullptr; }
            return false;
        }
    }
    state = State::Ready;
    return true;
}
