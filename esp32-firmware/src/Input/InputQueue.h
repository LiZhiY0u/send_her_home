#pragma once
#include "InputEvent.h"

// Task-context API only. Initialize before starting any producers.
namespace InputQueue {
bool Init();
bool Push(const UserInput::Event& event);
bool Push(const UserInput::Event& event,uint32_t epoch);
bool Read(UserInput::Event& event);
uint32_t DroppedCount();
}
