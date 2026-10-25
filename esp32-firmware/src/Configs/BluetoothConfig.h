#pragma once
#include <stdint.h>

namespace BluetoothConfig {
constexpr uint8_t AllowedGamepadAddresses[][6] = {
    {0xC0, 0xD6, 0xD5, 0xEE, 0xDB, 0x8A}, // Xbox
    {0x14, 0x3A, 0x9A, 0xD8, 0x18, 0x89}, // PS5 DualSense
};
inline bool IsAllowedGamepad(const uint8_t* address){
    if(!address)return false;
    for(const auto& allowed:AllowedGamepadAddresses){
        bool matches=true;
        for(unsigned i=0;i<6;++i)if(address[i]!=allowed[i]){matches=false;break;}
        if(matches)return true;
    }
    return false;
}
constexpr uint8_t AdvertisingData[] = {
    0x02, 0x01, 0x06,
    0x0b, 0x09, 'F', 'i', 'g', 'u', 'r', 'e', 'C', 'a', 's', 'e'
};
// ATT service FF10 / write FF11 and handle are defined in MiniAppProfile.h.
}
