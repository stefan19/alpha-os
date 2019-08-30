#include "process.h"
#include "kheap.h"
#include "paging.h"
#include "string.h"

static uint32_t PID_counter = 0;

thread_t* prNewThread(process_t* parent)
{
    thread_t* new = (thread_t*) kheapAlloc(sizeof(thread_t));

    // Insert new in process's thread list
    threat_t* k = parent->threadList;
    parent->threadList = new;
    parent->threadCount++;

    new->next = k;
    new->parent = parent;
    new->threadID = parent->threadCount++;

    return new;
}

//Starts a new user process
process_t* prSpawnProcess(const char* path, const char* name)
{
    process_t* process = (process_t*) kheapAlloc(sizeof(process_t));
    process->pageDir = newAddressSpace();
    strcpy(process->name, name);
    strcpy(process->imgPath, path);
    process->PID = PID_counter++;
    process->threadCount = 0;
    process->threadList = NULL;

    //Create the main thread
    process->main = prNewThread(process);

    uint32_t entry_point;
    if(elfLoadFromFile(path, &entry_point) != 0)
        return -1;

    

    return 0;
}