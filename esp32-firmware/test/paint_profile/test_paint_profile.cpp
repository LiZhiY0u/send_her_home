#include <cassert>
#include "Bluetooth/MiniAppProfile.h"
int main(){
    unsigned pos=1;unsigned count=0;bool seen[15]={};
    while(kBleRemoteProfileData[pos]||kBleRemoteProfileData[pos+1]) {
        const unsigned size=kBleRemoteProfileData[pos]|(kBleRemoteProfileData[pos+1]<<8);
        assert(size>=8&&pos+size<sizeof(kBleRemoteProfileData));
        const unsigned handle=kBleRemoteProfileData[pos+4]|(kBleRemoteProfileData[pos+5]<<8);
        assert(handle<15&&!seen[handle]);seen[handle]=true;++count;pos+=size;
    }
    assert(count==14&&pos+2==sizeof(kBleRemoteProfileData));
    assert(seen[9]&&seen[11]&&seen[13]&&seen[14]);
}
