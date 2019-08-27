#ifndef DEVMGR_H
#define DEVMGR_H

#define DEVICE_STORAGE  0x1
#define DEVICE_DISKPART 0x2
#define DEVICE_KEYBOARD 0x3

#include <stddef.h>
#include <stdint.h>

struct generic_device_struct;
typedef int (*devread_t)(const struct generic_device_struct* dev, size_t offset, size_t length, void* buffer);

struct generic_device_struct
{
    //Device information -- values are implementation dependant
    char name[32];
    uint8_t device_type;
    uint8_t id;

    //Device methods
    devread_t read;

    //Next device
    struct generic_device_struct* next;
};
typedef struct generic_device_struct generic_device_t;

typedef struct
{
    char name[32];
    uint8_t device_type;
    uint8_t id;

    devread_t read;
} generic_device_info_t;

void devmgrInit();
void devmgrAddDevice(generic_device_info_t* info);
void devmgrListDevices();
const generic_device_t* devmgrFind(const char* name);
const generic_device_t* devmgrRetrieve(uint8_t type, uint32_t index);

#endif