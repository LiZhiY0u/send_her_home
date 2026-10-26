#include <cassert>
#include "App/WakeGate.h"
#include "Lighting/StartupSequence.h"
#include "Lighting/RandomEffect.h"
int main(){
    assert(!WakeGate::Awake());assert(WakeGate::Wake());assert(!WakeGate::Wake());
    StartupSequence::State state;
    assert(!state.Active(false,100));assert(state.Active(true,100));
    assert(state.BrightnessPercent(100)==40);
    assert(state.BrightnessPercent(100+StartupSequence::FillMs+StartupSequence::DrainMs-1)==40);
    assert(state.BrightnessPercent(100+StartupSequence::FillMs+StartupSequence::DrainMs)==100);
    assert(state.Strength(0,0,100)==1);assert(state.Strength(100,100,100)==0);
    assert(state.Active(true,1100));assert(state.Strength(30,0,1100)==1);
    assert(state.Strength(90,90,1100)==0);
    assert(state.Active(true,2100));assert(state.Strength(100,100,2100)==1);
    assert(StartupSequence::Color==0xffffff);
    assert(StartupSequence::HoldMs==0);
    const float fading=state.Strength(100,100,2200);
    assert(fading>0&&fading<1); // Outer LEDs fade instead of switching off abruptly.
    const float entering=state.Strength(80,0,1100);
    assert(entering>0&&entering<1);
    for(uint32_t t=120;t<2100;t+=20){
        const float a=state.Strength(80,0,t-20),b=state.Strength(80,0,t);
        assert(b>=a&&b-a<.12f);
    }
    for(uint32_t t=2120;t<4100;t+=20){
        const float a=state.Strength(80,0,t-20),b=state.Strength(80,0,t);
        assert(b<=a&&a-b<.12f);
    }
    assert(state.Active(true,3100));
    assert(state.Strength(30,0,3100)==1);assert(state.Strength(100,100,3100)==0);
    assert(state.Active(true,4100));assert(state.Strength(0,0,4100)==0); // All dark frame.
    const uint32_t ripple=100+StartupSequence::FillMs+StartupSequence::HoldMs+StartupSequence::DrainMs+StartupSequence::DarkMs;
    assert(state.Strength(0,0,ripple)==1);
    assert(state.Strength(80,0,ripple+1000)==1);
    assert(state.Strength(0,0,ripple+2000)==1); // Second expansion.
    assert(state.Strength(80,0,ripple+3000)==1);
    const uint32_t rotate=ripple+StartupSequence::RippleMs;
    assert(state.Strength(80,0,rotate)==1);
    assert(state.Strength(0,80,rotate+1000)>.99f);
    assert(state.Strength(80,0,rotate+4000)>.99f); // Second full turn.
    assert(state.Active(true,100+StartupSequence::TotalMs-1));
    assert(!state.Active(true,100+StartupSequence::TotalMs));
    assert(!state.Active(true,100+StartupSequence::TotalMs+1000));
    StartupSequence::State command;command.Finish();assert(!command.Active(true,0));
    StartupSequence::State wrap;assert(wrap.Active(true,0xffffff00u));
    assert(wrap.Active(true,0xffffff00u+StartupSequence::TotalMs-1));
    assert(!wrap.Active(true,0xffffff00u+StartupSequence::TotalMs));
    RandomEffect::State random(123);unsigned previous=0;bool seen[SpatialEffects::Count]={};
    for(unsigned i=0;i<1000;++i){const auto effect=random.Next();assert(effect>0&&effect<SpatialEffects::Count);assert(effect!=previous);seen[effect]=true;previous=effect;}
    for(unsigned i=1;i<SpatialEffects::Count;++i)assert(seen[i]);
    const auto epoch=WakeGate::Epoch();WakeGate::Reset();
    assert(!WakeGate::Awake()&&WakeGate::Epoch()==epoch+1);
    assert(!WakeGate::WakeIfEpoch(epoch));assert(!WakeGate::Awake());
    bool first=false;assert(WakeGate::WakeIfEpoch(epoch+1,&first)&&first);
    assert(WakeGate::WakeIfEpoch(epoch+1,&first)&&!first);
    WakeGate::Reset();assert(!WakeGate::Awake());
    assert(WakeGate::Wake());assert(!WakeGate::Wake());
    state=StartupSequence::State{};assert(state.Active(true,30000));
    assert(state.Strength(0,0,30000)==1&&state.Strength(100,100,30000)==0);
}
