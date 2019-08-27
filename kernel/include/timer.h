#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "task.h"

struct sleeping_task_struct;

struct sleeping_task_struct
{
    task_t* associated_task;
    uint64_t wake_time;
    struct sleeping_task_struct* next;
};
typedef struct sleeping_task_struct sleeping_task_t;

void timerInit(uint32_t frequency);
uint64_t timerGetNano();

#endif