#pragma once
#include <cstdint>
namespace PaintBridge {
struct Command { uint8_t session; uint16_t sequence; bool automatic; uint8_t pixels[399]; bool effect=false; uint8_t settings[6]={}; };
bool Init();
void Connect(uint16_t handle);
void Disconnect(uint16_t handle);
int Write(uint16_t handle,uint16_t attribute,uint16_t transaction,uint16_t offset,uint8_t* data,uint16_t size);
uint16_t ReadStatus(uint16_t handle,uint16_t attribute,uint16_t offset,uint8_t* data,uint16_t size);
void CanSend();
bool Read(Command& command);
void Applied(const Command& command);
constexpr uint16_t WriteHandle=0x000b, StatusHandle=0x000d, CccHandle=0x000e;
}
