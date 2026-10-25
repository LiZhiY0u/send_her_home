#include "InputQueue.h"
#include "App/WakeGate.h"
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace {
constexpr unsigned Capacity = 16;
QueueHandle_t queue = nullptr;
std::atomic<uint32_t> dropped(0);
struct QueuedEvent {UserInput::Event event;uint32_t epoch;};
}

bool InputQueue::Init()
{
    if (queue == nullptr)
        queue = xQueueCreate(Capacity, sizeof(QueuedEvent));
    return queue != nullptr;
}

bool InputQueue::Push(const UserInput::Event& event)
{
    return Push(event,WakeGate::Epoch());
}
bool InputQueue::Push(const UserInput::Event& event,uint32_t epoch)
{
    if (queue == nullptr || event.action == UserInput::Action::None || epoch!=WakeGate::Epoch())
        return false;
    const QueuedEvent queued={event,epoch};
    if (xQueueSend(queue, &queued, 0) == pdTRUE)
        return true;
    dropped.fetch_add(1, std::memory_order_relaxed);
    return false;
}

bool InputQueue::Read(UserInput::Event& event)
{
    QueuedEvent queued;
    for(unsigned i=0;queue&&i<Capacity&&xQueueReceive(queue,&queued,0)==pdTRUE;++i){
        if(queued.epoch==WakeGate::Epoch()){event=queued.event;return true;}
    }
    return false;
}

uint32_t InputQueue::DroppedCount()
{
    return dropped.load(std::memory_order_relaxed);
}
