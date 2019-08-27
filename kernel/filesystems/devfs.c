#include "vfs.h"
#include "devfs.h"
#include "kheap.h"
#include "string.h"

struct device_struct;

struct device_struct
{
    dirent entry;
    vnode node;
    struct device_struct* next;
};
typedef struct device_struct device;

device* device_list;
vnode dev_root;

dirent* devfsReadDir(vnode* node, size_t i)
{
    device* dev = device_list;
    while(dev != NULL && i > 0)
    {
        dev = dev->next;
        i--;
    }
    if(dev == NULL)
        return NULL;
    else
        return &dev->entry;
}

void devfsAddDevice(const char* devName, read_t readCallback, uint32_t id)
{
    device* new = (device*)kheapAlloc(sizeof(device));
    new->next = device_list;
    strcpy(new->entry.name, devName);
    new->entry.node = &new->node;
    new->node.type = FS_FILE;
    new->node.length = 0;
    new->node.id = id;
    new->node.read = readCallback;
    new->node.readdir = NULL;

    device_list = new;
}

void devfsInit()
{
    device_list = NULL;

    dev_root.type = FS_DIR;
    dev_root.length = 0;
    dev_root.id = 0;
    dev_root.read = NULL;
    dev_root.readdir = devfsReadDir;

    vfsMount("/dev", &dev_root);
}