#include "isr.h"
#include "ll_io.h"
#include "console.h"
#include "task.h"
#include "kheap.h"
#include "pic.h"
#include "timer.h"

static volatile uint64_t counter = 0;
static uint64_t nanoPerTick;

sleeping_task_t* sleeping_tasks;

extern uint64_t time_slice_remaining;
extern bool tasking_enabled;

static void removeTaskListElement()
{
    sleeping_task_t* tmp = sleeping_tasks;
    sleeping_tasks = sleeping_tasks->next;
    kheapFree(tmp);
}

void timerIRQ(const interrupt_frame_t* frame)
{
    counter += nanoPerTick;

    outb(0x20, 0x20);
    if(!tasking_enabled)
        return;

    uint32_t fl = schedulerLockSwitch();
    while(sleeping_tasks != NULL && sleeping_tasks->wake_time <= counter)
    {
        printf("\nUnblocking task %u", sleeping_tasks->associated_task->pid);
        unblockTask(sleeping_tasks->associated_task);
        removeTaskListElement();
    }

    if(time_slice_remaining != 0)
    {
        if(time_slice_remaining <= nanoPerTick)
        {
            schedule();
        } else {
            time_slice_remaining -= nanoPerTick;
        }
    }

    schedulerUnlockSwitch(fl); 
    //schedule();
}

void timerInit(uint32_t frequency)
{
    nanoPerTick = 1000000000 / frequency;
    
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);

    sleeping_tasks = NULL;

    registerInterruptHandler(32, timerIRQ);

    picUnmask(0);
}

uint64_t timerGetNano()
{
    return counter;
}