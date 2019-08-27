#ifndef ATA_H
#define ATA_H

#include <stdint.h>
#include <stdbool.h>

#define ATA_MASTER  0xA0
#define ATA_SLAVE   0xB0

#define ATA_PRIMARY_BASE        0x1F0
#define ATA_PRIMARY_CONTROL     0x3F6
#define ATA_SECONDARY_BASE      0x170
#define ATA_SECONDARY_CONTROL   0x376

#define ATA_DATA        0
#define ATA_ERROR       1
#define ATA_FEATURES    1
#define ATA_SECTORCNT   2
#define ATA_LBA_LO      3
#define ATA_LBA_MID     4
#define ATA_LBA_HIGH    5
#define ATA_DRIVESEL    6
#define ATA_STATUS      7
#define ATA_COMMAND     7

#define ATA_BMCOMMAND   0
#define ATA_BMSTATUS    2
#define ATA_BMPRDT      4

#define ATA_CMD_IDENTIFY    0xEC
#define ATA_CMD_READ_PIO28  0x20
#define ATA_CMD_READ_DMA28  0xC8

typedef enum
{
    IDE_NONE,
    IDE_ATA,
    IDE_ATAPI,
    IDE_SATA
} ide_drive_t;

typedef struct 
{
    ide_drive_t type;
    uint16_t ioBase;
    uint16_t ctrlBase;
    uint16_t bmBase;
    bool dmaActive;
    bool udmaActive;
    bool lba48Supported;
    bool lba28Supported;
    uint64_t maxSector;
} ide_drive_info;

struct prd_entry_struct
{
    uint32_t base : 32;
    uint16_t byteCount : 16;
    uint16_t reserved : 15;
    uint16_t EOT : 1;
} __attribute__((packed));
typedef struct prd_entry_struct prd_entry;

void ideEnumerateDrives();
int ideDMARead(uint32_t driveIdx, uint32_t offset, uint32_t len, void *ptr);

#endif