#include "elf.h"
#include "task.h"
#include "kheap.h"

//Starts a new user process
int procSpawnFromFile(const char* path)
{
    uint32_t entry_point;
    if(elfLoadFromFile(path, &entry_point) != 0)
        return -1;

    //Create a kernel stack for the task
    void* kernel_stack = kheapAlloc_a(1);
    createNewUserTask(entry_point, (uint32_t)(kernel_stack+0x1000));

    return 0;
}