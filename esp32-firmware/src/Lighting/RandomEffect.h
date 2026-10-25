#pragma once
#include <cstdint>
#include "Lighting/SpatialEffects.h"
namespace RandomEffect {
class State {
    uint32_t seed_;
    unsigned last_=0;
public:
    explicit State(uint32_t seed):seed_(seed?seed:0x9e3779b9u){}
    unsigned Next(){
        seed_^=seed_<<13;seed_^=seed_>>17;seed_^=seed_<<5;
        unsigned next=1+seed_%(SpatialEffects::Count-1);
        if(next==last_)next=1+next%(SpatialEffects::Count-1);
        last_=next;return next;
    }
};
}
