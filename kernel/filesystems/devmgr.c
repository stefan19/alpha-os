#include "devmgr.h"
#include "kheap.h"
#include "string.h"

static generic_device_t* devlist;

void devmgrInit()
{
    devlist = NULL;
}

void devmgrAddDevice(generic_device_info_t* info)
{
    //Verify that this device doesn't exist
    generic_device_t* k = devlist;
    while(k != NULL)
    {
        if(strcmp(k->name, info->name) == 0)
        {
            printf("\nError: attempting to add already existing device");
            return;
        }
        k = k->next;
    }

    generic_device_t* new = (generic_device_t*) kheapAlloc(sizeof(generic_device_t));
    strncpy(new->name, info->name, 32);
    new->device_type = info->device_type;
    new->id = info->id;
    new->read = info->read;
    
    new->next = devlist;
    devlist = new;
}

void devmgrListDevices()
{
    generic_device_t* k = devlist;
    while(k != NULL)
    {
        printf("\nDevice: %s, type: %u, id: %u", k->name, k->device_type, k->id);
        k = k->next;
    }
}

const generic_device_t* devmgrFind(const char* name)
{
    generic_device_t* k = devlist;
    while(k != NULL)
    {
        if(strcmp(k->name, name) == 0)
            return k;
        k = k->next;
    }

    printf("\nDevice %s doesn't exist", name);
    return NULL;
}

const generic_device_t* devmgrRetrieve(uint8_t type, uint32_t index)
{
    generic_device_t* k = devlist;
    while(k != NULL)
    {
        if(k->device_type == type)
        {
            if(index == 0)
                return k;
            index--;
        }
        k = k->next;
    }

    return NULL;
}