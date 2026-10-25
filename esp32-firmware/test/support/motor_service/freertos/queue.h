#pragma once
#include <deque>
#include <vector>
#include <cstring>
struct MotorTestQueue {unsigned capacity,size;std::deque<std::vector<unsigned char>> entries;};
using QueueHandle_t=MotorTestQueue*;
inline QueueHandle_t xQueueCreate(unsigned capacity,unsigned size){return new MotorTestQueue{capacity,size,{}};}
inline void vQueueDelete(QueueHandle_t q){delete q;}
inline int xQueueSend(QueueHandle_t q,const void* p,unsigned){if(q->entries.size()>=q->capacity)return 0;const auto b=static_cast<const unsigned char*>(p);q->entries.emplace_back(b,b+q->size);return 1;}
inline int xQueueReceive(QueueHandle_t q,void* p,unsigned){if(q->entries.empty())return 0;std::memcpy(p,q->entries.front().data(),q->size);q->entries.pop_front();return 1;}
