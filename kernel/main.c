#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "exceptions.h"
#include "paging.h"
#include "kheap.h"
#include "multiboot.h"
#include "task.h"
#include "timer.h"
#include "pci.h"
#include "ide.h"
#include "pic.h"
#include "vfs.h"
#include "syscall.h"
#include "mbr.h"
#include "fat.h"
#include "devmgr.h"
#include "ps2.h"
#include "ps2kbd.h"
#include "process.h"

#include "stdio.h"
#include "string.h"

extern uint8_t* buffer;

/* void userApp()
{
    printf("\nGetPID() returned: %u, task name is: %s, time: %u us", getPID(), getTaskName(), getTaskTime()/1000);
    //Wasting time:
    for(;;);
    syscall_exit();
}*/

void kthread()
{
    printf("\nGetPID() returned: %u, task name is: %s", getPID(), getTaskName());
    printf("Returned to thread");
    for(;;);
}
 
void kmain(multiboot_info_t* multibootInfo)
{
    consoleClr();
    printf("Multiboot structure: %x\nFlags: %x", (uint32_t)multibootInfo, multibootInfo->flags);
    
    gdtInit();
    picInit(0x20, 0x28);
    idtInit();
    registerExceptionHandlers();
    timerInit(100);
    asm volatile("sti");
    
    printf("The system is now running. Lower memory: %u kb, upper memory: %u kb", 
        multibootInfo->mem_lower, multibootInfo->mem_upper);

    multiboot_mmap_t* mmap = (multiboot_mmap_t*)multibootInfo->mmap_addr;
    initPagingStructures(mmap, multibootInfo->mmap_addr, multibootInfo->mmap_length);
    initPaging();
    kheapInit(0xD0000000, 0xD8000000, 5);

    vfsInit();
    devmgrInit();

    /* pciCheckAllBuses();
    ideEnumerateDrives();
    mbrInit();
    fat32InitVolume("ata0p0"); */

    ps2Initialise();
    ps2KbdInit();

    /* vnode* root = vfsOpen("/");
    printf("Listing root directory: \n");

    size_t i = 0;
    dirent* entry = root->readdir(root, 0);
    while(entry != NULL)
    {
        printf(" %s\n", entry->name);
        entry = root->readdir(root, ++i);
    } */

    /* vnode* file = vfsOpen("/menu.lst");

    uint8_t data[100];
    file->read(file, 0, file->length, data);
    printf("Text file: %s", data); */

    //displayHeapStructure();
    //fat32RootDir();

    /* initMultitasking();

    procSpawnFromFile("/int");
    blockTask(TASK_PAUSED); */
    //createNewKernelThread(kthread, (uint32_t)kernelStack + 0x1000);
    
    /* consoleWriteStr("\nOriginal kernel");
    nanoSleepUntil(3000000000); */
    //printf("\nGetPID() returned: %u, task name is: %s", getPID(), getTaskName());

    //asm volatile("sti");
}
