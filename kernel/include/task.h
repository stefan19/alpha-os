#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>

#include "isr.h"

#define TASK_RUNNING    1
#define TASK_SLEEPING   2
#define TASK_PAUSED     3

#define TIME_SLICE_LENGTH 50000000

typedef struct task task_t;
struct task
{
    uint32_t esp;
    uint32_t esp0;
    uint32_t cr3;
    uint32_t entry;
    uint32_t pid;
    uint64_t time_used;
    char name[32];
    bool user;
    uint8_t state;
};

void schedule();
void initMultitasking();
void createNewKernelThread(uint32_t entryPoint, uint32_t stack);
void createNewUserTask(uint32_t entryPoint, uint32_t kernelStack);
void switchToUserMode(uint32_t stack, uint32_t entryPoint);

void blockTask();
void unblockTask(task_t* task);
uint32_t schedulerLockSwitch();
void schedulerUnlockSwitch();

const char* getTaskName();
uint32_t getPID();
uint64_t getTaskTime();

uint32_t schedulerLock();
void schedulerUnlock(uint32_t eflags);
void nanoSleepUntil(uint64_t wakeTime);

#endif