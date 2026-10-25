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
    uint8_t effectSettings[6]={};
    unsigned effectFrame=0;
    bool randomColor=false;
    uint32_t colorStart=0,randomBase=0x57d9ff;
    static uint8_t paintedPixels[399]={};
    uint8_t outputBrightness=100;
    PaintBridge::Command command{};
    for (;;) {
        if(PaintBridge::Read(command)) {
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
                continue;
            }
            painting = !command.automatic&&!command.effect;
            if(command.effect) {
                selectedEffect=true;effectFrame=0;
                for(unsigned i=0;i<6;++i)effectSettings[i]=command.settings[i];
                if(command.session!=0){
                    outputBrightness=effectSettings[2];
                    testStrip.setBrightness(LedStripTest::Brightness*outputBrightness/100);
                }
                randomBase=(static_cast<uint32_t>(effectSettings[3])<<16)|(static_cast<uint32_t>(effectSettings[4])<<8)|effectSettings[5];
                if(command.session!=0){randomColor=command.settings[6]!=0;colorStart=millis();}
            } else if(command.automatic) selectedEffect=false;
            if(painting) {
                for(unsigned i=0;i<399;++i)paintedPixels[i]=command.pixels[i];
                for(unsigned i=0;i<133;++i) testStrip.setPixelColor(i,
                    (static_cast<uint32_t>(command.pixels[i*3])<<16)|
                    (static_cast<uint32_t>(command.pixels[i*3+1])<<8)|command.pixels[i*3+2]);
                testStrip.show();
            }
            if(!command.effect) PaintBridge::Applied(command);
        }
        if(painting) {vTaskDelay(pdMS_TO_TICKS(10));continue;}
        if(selectedEffect) {
            const float seconds=effectFrame*SpatialEffects::FrameMs/1000.f*effectSettings[1]/50.f;
            const uint32_t fixedColor=(static_cast<uint32_t>(effectSettings[3])<<16)|
                (static_cast<uint32_t>(effectSettings[4])<<8)|effectSettings[5];
            const uint32_t color=randomColor?RandomColor::At(millis()-colorStart,randomBase):fixedColor;
            for(unsigned i=0;i<LedPhysicalMap::Count;++i) {
                const auto p=LedPhysicalMap::Get(i);
                const float v=SpatialEffects::Strength(static_cast<SpatialEffects::Effect>(effectSettings[0]),p.x,p.y,seconds);
                testStrip.setPixelColor(i,(static_cast<uint32_t>(((color>>16)&255)*v)<<16)|
                    (static_cast<uint32_t>(((color>>8)&255)*v)<<8)|static_cast<uint32_t>((color&255)*v));
            }
            testStrip.show();
            if(command.effect){PaintBridge::Applied(command);command.effect=false;}
            ++effectFrame;vTaskDelay(pdMS_TO_TICKS(SpatialEffects::FrameMs));continue;
        }
#if ENABLE_LED_SPATIAL_TEST
        SpatialEffects::DrawPlaylist(testStrip, step,randomColor?RandomColor::At(millis()-colorStart,randomBase):0);
        vTaskDelay(pdMS_TO_TICKS(SpatialEffects::FrameMs));
        step = (step + 1) % (SpatialEffects::PlaylistCount * SpatialEffects::FramesPerEffect);
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
    for (;;) { OledUI::Task(); vTaskDelay(pdMS_TO_TICKS(TaskConfig::DisplayMs)); }
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
