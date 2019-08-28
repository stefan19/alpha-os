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
#include "framebuffer.h"
#include "psf.h"

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
 
void kmain(uint32_t bitmapAddr, uint32_t bitmapSize, multiboot_info_t* mbi, uint32_t loader_end)
{
    framebufferInit(mbi);
    consoleInit(0, 0xD0D0D0);
    consoleWriteStr("The system is now running\n");
    
    gdtInit();
    picInit(0x20, 0x28);
    idtInit();
    registerExceptionHandlers();
    timerInit(100);
    asm volatile("sti");
    
    initPaging(bitmapAddr, bitmapSize, loader_end);
    kheapInit(0xD0000000, 0xD8000000, 4);
 
    vfsInit();
    devmgrInit();

    pciCheckAllBuses();
    ideEnumerateDrives(); 

    ps2Initialise();
    ps2KbdInit();

    displayHeapStructure();

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
