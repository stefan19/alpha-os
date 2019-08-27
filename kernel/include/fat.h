#ifndef FAT_H
#define FAT_H

#define FAT_READ_ONLY   0x01
#define FAT_HIDDEN      0x02
#define FAT_SYSTEM      0x04
#define FAT_VOLUME_ID   0x08
#define FAT_DIRECTORY   0x10
#define FAT_ARCHIVE     0x20
#define FAT_LFN         0x0F

#include <stdint.h>

typedef struct
{
    uint32_t first_data_sector;
    uint32_t FAT_size;
} FAT_volume_t;

typedef struct
{
    uint8_t     BS_jmpBoot[3];
    uint8_t     BS_OEMName[8];
    uint16_t    BPB_BytesPerSec;
    uint8_t     BPB_SecPerClus;
    uint16_t    BPB_RsvdSecCnt;
    uint8_t     BPB_NumFATs;
    uint16_t    BPB_RootEntCnt;
    uint16_t    BPB_TotSec16;
    uint8_t     BPB_Media;
    uint16_t    BPB_FATSz16;
    uint16_t    BPB_SecPerTrk;
    uint16_t    BPB_NumHeads;
    uint32_t    BPB_HiddSec;
    uint32_t    BPB_TotSec32;
} __attribute__((packed)) BPB_t;

typedef struct
{
    uint8_t     BS_DrvNum;
    uint8_t     BS_Reserved1;
    uint8_t     BS_BootSig;
    uint32_t    BS_VolID;
    uint8_t     BS_VolLab[11];
    uint8_t     BS_FilSysType[8];
} __attribute__((packed)) BPB_fat16_t;

typedef struct
{
    uint32_t    BPB_FATSz32;
    uint16_t    BPB_ExtFlags;
    uint16_t    BPB_FSVer;
    uint32_t    BPB_RootClus;
    uint16_t    BPB_FSInfo;
    uint16_t    BPB_BkBootSec;
    uint8_t     BPB_Reserved[12];
    uint8_t     BPB_DrvNum;
    uint8_t     BPB_Reserved1;
    uint8_t     BS_BootSig;
    uint32_t    BS_VolID;
    uint8_t     BS_VolLab[11];
    uint8_t     BS_FilSysType[8];
} __attribute__((packed)) BPB_fat32_t;

typedef struct
{
    uint8_t     name[11];
    uint8_t     attr;
    uint8_t     nt_reserved;
    uint8_t     creation_tenthsec;
    uint16_t    creation_time;
    uint16_t    creation_date;
    uint16_t    accessed_date;
    uint16_t    hi_cluster;
    uint16_t    modif_time;
    uint16_t    modif_date;
    uint16_t    lo_cluster;
    uint32_t    size;
} __attribute__((packed)) FAT_dirent;

typedef struct
{
    uint8_t     order;
    uint16_t    name1[5];
    uint8_t     attr;
    uint8_t     type;
    uint8_t     chksum;
    uint16_t    name2[6];
    uint16_t    zero;
    uint16_t    name3[2];
} __attribute__((packed)) FAT_LFN_ent;

int fat32InitVolume(const char* part_name);
int fat32RootDir();

#endif