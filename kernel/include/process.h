#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "task.h"

struct process_struct;
struct thread_struct;

struct thread_struct
{
    struct process_struct* parent;
    uint32_t threadID;
    task_t* context;
    thread_t* next;
};
typedef struct thread_struct thread_t;

struct process_struct
{
    uint32_t PID;
    uint32_t pageDir;
    thread_t* main;
    uint32_t threadCount;
    thread_t* threadList;
    char name[40];
    char imgPath[256];
};
typedef struct process_struct process_t;

int procSpawnFromFile(const char* path);

#endif