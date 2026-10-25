#include <cassert>
#include "Configs/BluetoothConfig.h"
int main(){
    const uint8_t xbox[]={0xc0,0xd6,0xd5,0xee,0xdb,0x8a};
    const uint8_t ps5[]={0x14,0x3a,0x9a,0xd8,0x18,0x89};
    const uint8_t other[]={0xf4,0x6a,0xd7,0xa0,0x5c,0x59};
    const uint8_t reversed[]={0x89,0x18,0xd8,0x9a,0x3a,0x14};
    assert(BluetoothConfig::IsAllowedGamepad(xbox));
    assert(BluetoothConfig::IsAllowedGamepad(ps5));
    assert(!BluetoothConfig::IsAllowedGamepad(other));
    assert(!BluetoothConfig::IsAllowedGamepad(reversed));
    assert(!BluetoothConfig::IsAllowedGamepad(nullptr));
    for(unsigned i=0;i<6;++i){uint8_t changed[6];for(unsigned j=0;j<6;++j)changed[j]=ps5[j];changed[i]^=1;assert(!BluetoothConfig::IsAllowedGamepad(changed));}
}
