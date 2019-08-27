#include "task.h"
#include "kheap.h"
#include "paging.h"
#include "ll_io.h"
#include "string.h"
#include "gdt.h"
#include "queue.h"
#include "timer.h"

extern void _switchTask(task_t* current, task_t* next);
extern void createStack(task_t* new, uint32_t entry, uint32_t stack);

task_t* current_task;
task_t kernel_task;

queue_t ready_tasks;
uint32_t nextPID = 0;

uint64_t last_time = 0;
uint64_t CPU_idle_time = 0;

uint32_t postpone_switches_counter = 0;
uint32_t switch_postponed_flag = 0;

uint64_t time_slice_remaining = 0;

//Initially, threading is disabled
bool tasking_enabled = false;

void switchTask(task_t* next)
{
    if(postpone_switches_counter != 0)
    {
        switch_postponed_flag = 1;
        return;
    }
    
    if(ready_tasks.size == 0 && current_task->state != TASK_RUNNING)
    {
        //Only one task is ready
        time_slice_remaining = 0;
    }
    else
    {
        //There are at least 2 tasks ready
        time_slice_remaining = TIME_SLICE_LENGTH;
    }

    task_t* old = current_task;
    next->state = TASK_RUNNING;
    current_task = next;
    _switchTask(old, next);
}

uint32_t schedulerLock()
{
    uint32_t eflags = readFlags();
    asm volatile("cli");
    return eflags;
}

void schedulerUnlock(uint32_t eflags)
{
    restoreFlags(eflags);
}

uint32_t schedulerLockSwitch()
{
    uint32_t eflags = readFlags();
    asm volatile("cli");
    postpone_switches_counter++;
    return eflags;
}

void schedulerUnlockSwitch(uint32_t eflags)
{
    postpone_switches_counter--;
    if(postpone_switches_counter == 0)
    {
        //If a switch was postponed, schedule
        if(switch_postponed_flag != 0)
        {
            switch_postponed_flag = 0;
            schedule();
        }
    }

    restoreFlags(eflags);
}

void updateTaskTime(void)
{
    uint64_t current = timerGetNano();
    uint64_t elapsed = current - last_time;
    last_time = current;
    if(current_task == NULL)
        CPU_idle_time += elapsed;
    else 
        current_task->time_used += elapsed;
}

void schedule()
{
    if(tasking_enabled == false)
        return;
    if(postpone_switches_counter != 0)
    {
        switch_postponed_flag = 1;
        return;
    }

    updateTaskTime();

    if(ready_tasks.size != 0)
    {
        task_t* task = (task_t*)queueRemove(&ready_tasks);
        switchTask(task);
    } else if (current_task->state == TASK_RUNNING)
    {
        //Let the current task run
    } else {
        printf("\nidle...");
        task_t* task = current_task;
        current_task = NULL;
        uint64_t idle_start_time = timerGetNano();

        do
        {
            asm volatile("sti");
            asm volatile("hlt");
            asm volatile("cli");
        } while (ready_tasks.size == 0);

        current_task = task;
        task = (task_t*) queueRemove(&ready_tasks);
        if(task != current_task)
        {
            switchTask(task);
        }
        else 
            current_task->state = TASK_RUNNING;
    }
}

uint32_t getPID()
{
    return current_task->pid;
}

const char* getTaskName()
{
    return current_task->name;
}

uint64_t getTaskTime()
{
    return current_task->time_used;
}

static void prepareUserTask()
{
    uint32_t fl = schedulerLock();
    uint32_t userStack = (uint32_t)kheapAlloc_a(1);
    schedulerUnlock(fl);

    switchToUserMode(userStack+0x1000, current_task->entry);
}

extern void jmpToKernelThread(uint32_t entry);

static void prepareKernelTask()
{
    jmpToKernelThread(current_task->entry);
}

void createNewUserTask(uint32_t entryPoint, uint32_t kernelStack)
{
    uint32_t fl = schedulerLock();

    task_t* new = (task_t*)kheapAlloc(sizeof(task_t));
    new->esp0 = kernelStack;
    new->pid = nextPID++;
    new->cr3 = readCR3();
    new->user = true;
    strcpy(new->name, "no name");
    new->entry = entryPoint;
    createStack(new, (uint32_t)prepareUserTask, kernelStack);
    
    queueAdd(&ready_tasks, (uint32_t)new);

    schedulerUnlock(fl);
}

void createNewKernelThread(uint32_t entryPoint, uint32_t stack)
{
    uint32_t fl = schedulerLock();

    task_t* new = (task_t*)kheapAlloc(sizeof(task_t));
    new->esp0 = 0;
    new->pid = nextPID++;
    new->cr3 = readCR3();
    new->user = false;
    strcpy(new->name, "i386-krnl thread");
    new->entry = entryPoint;
    createStack(new, (uint32_t)prepareKernelTask, stack);

    queueAdd(&ready_tasks, (uint32_t)new);

    schedulerUnlock(fl);
}

void initMultitasking()
{
    uint32_t fl = schedulerLock();

    current_task = &kernel_task;
    current_task->esp0 = 0;
    current_task->pid = nextPID++;
    current_task->cr3 = readCR3();
    current_task->user = false;
    current_task->entry = 0;
    current_task->state = TASK_RUNNING;
    strcpy(current_task->name, "i386-krnl");

    tasking_enabled = true;
    queueInit(&ready_tasks);

    time_slice_remaining = 0;

    schedulerUnlock(fl);
}

void blockTask(uint32_t reason)
{
    uint32_t fl = schedulerLock();
    current_task->state = reason;
    schedule();
    schedulerUnlock(fl);
}

void unblockTask(task_t* task)
{
    uint32_t fl = schedulerLock();
    if(ready_tasks.size == 0)
    {
        if(postpone_switches_counter != 0)
        {
            if(current_task != NULL)
                switch_postponed_flag = 1;
            
            queueAdd(&ready_tasks, (uint32_t)task);
        }
        else
            switchTask(task);
    }
    else
        queueAdd(&ready_tasks, (uint32_t)task);
    schedulerUnlock(fl);
}

extern sleeping_task_t* sleeping_tasks;

void nanoSleepUntil(uint64_t wakeTime)
{
    //Check whether wakeTime already happened
    if(wakeTime < timerGetNano())
        return;

    uint32_t fl = schedulerLock();

    sleeping_task_t* new = (sleeping_task_t*) kheapAlloc(sizeof(sleeping_task_t));
    new->associated_task = current_task;
    new->wake_time = wakeTime;

    sleeping_task_t* k = sleeping_tasks;
    sleeping_task_t* prev = NULL;
    while(k != NULL && wakeTime >= k->wake_time)
    {
        prev = k;
        k = k->next;
    }

    //Insert new task before k
    if(prev != NULL)
        prev->next = new;
    else
        sleeping_tasks = new;

    new->next = k;

    schedulerUnlock(fl);

    blockTask(TASK_SLEEPING);
}