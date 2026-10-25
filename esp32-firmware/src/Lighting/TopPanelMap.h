#pragma once
#include <cstdint>
#include "Diagnostics/LedPhysicalMap.h"
namespace TopPanelMap {
// Exact x -> -x, y -> y permutation of the verified 133-point physical map.
// Map logical bottom-view positions to top-strip wire indices.
static const uint8_t MirrorX[LedPhysicalMap::Count]={
0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,27,26,25,24,23,22,21,20,19,18,17,16,31,30,29,28,32,
108,111,110,109,116,115,114,113,112,125,124,123,122,121,120,119,118,117,132,131,130,129,128,127,126,
83,86,85,84,91,90,89,88,87,100,99,98,97,96,95,94,93,92,107,106,105,104,103,102,101,
58,61,60,59,66,65,64,63,62,75,74,73,72,71,70,69,68,67,82,81,80,79,78,77,76,
33,36,35,34,41,40,39,38,37,50,49,48,47,46,45,44,43,42,57,56,55,54,53,52,51
};
static const uint8_t Rotate180[LedPhysicalMap::Count]={
8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,22,23,24,25,26,27,16,17,18,19,20,21,30,31,28,29,32,
83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,
116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82
};
inline unsigned Index(unsigned logical,bool mirror=true,bool rotate=true){
 if(logical>=LedPhysicalMap::Count)return logical;
 const unsigned reflected=mirror?MirrorX[logical]:logical;
 return rotate?Rotate180[reflected]:reflected;
}
}
