#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MBOOT_INFO_END      0
#define MBOOT_INFO_CMDLINE  1
#define MBOOT_INFO_LOADER   2
#define MBOOT_INFO_MODS     3
#define MBOOT_INFO_MEM      4
#define MBOOT_INFO_BOOTDEV  5
#define MBOOT_INFO_MMAP     6
#define MBOOT_INFO_VBE      7
#define MBOOT_INFO_FRAMEBUF 8
#define MBOOT_INFO_ELFSYM   9
#define MBOOT_INFO_APM      10
#define MBOOT_INFO_EFI32TBL 11
#define MBOOT_INFO_EFI64TBL 12
#define MBOOT_INFO_SMBIOS   13
#define MBOOT_INFO_ACPI1    14
#define MBOOT_INFO_ACPI2    15
#define MBOOT_INFO_NETWORK  16
#define MBOOT_INFO_EFIMMAP  17
#define MBOOT_INFO_EFISERV  18
#define MBOOT_INFO_EFI32IMG 19
#define MBOOT_INFO_EFI64IMG 20
#define MBOOT_INFO_IMGPHYS  21

#define MBOOT_FRAMEBUF_IDX      0
#define MBOOT_FRAMEBUF_DIRECT   1
#define MBOOT_FRAMEBUF_EGATEXT  2

typedef struct
{
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed)) multiboot_info_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) mtag_generic_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} __attribute__((packed)) mtag_mem_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t partition;
    uint32_t sub_partition;
} __attribute__((packed)) mtag_bootdev_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint8_t  str;
} __attribute__((packed)) mtag_cmdline_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    uint8_t  string;
} __attribute__((packed)) mtag_mods_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    uint8_t  entries;
} __attribute__((packed)) mtag_mmap_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint8_t  vbe_control_info[512];
    uint8_t  vbe_mode_info[256];
} __attribute__((packed)) mtag_vbe_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t  reserved;

    //Only valid for direct color
    uint8_t  framebuffer_red_field_pos;
    uint8_t  framebuffer_red_mask_size;
    uint8_t  framebuffer_green_field_pos;
    uint8_t  framebuffer_green_mask_size;
    uint8_t  framebuffer_blue_field_pos;
    uint8_t  framebuffer_blue_mask_size;
} __attribute__((packed)) mtag_framebuf_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) mtag_acpi1_t;

typedef struct
{
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) mtag_acpi2_t;

#endif
