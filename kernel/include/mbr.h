#ifndef MBR_H
#define MBR_H

#include <stdint.h>
#include <stdbool.h>
#include "devmgr.h"

typedef struct
{
    bool present;
    uint32_t start_lba;
    uint32_t sectors;
    const generic_device_t* dev;
} mbr_partition;

void mbrInit();
void mbrList();

#endif