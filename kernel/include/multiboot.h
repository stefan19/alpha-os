#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

struct multiboot_info
{
  /* Multiboot info version number */
  uint32_t flags;

  /* Available memory from BIOS */
  uint32_t mem_lower;
  uint32_t mem_upper;

  /* "root" partition */
  uint32_t boot_device;

  /* Kernel command line */
  uint32_t cmdline;

  /* Boot-Module list */
  uint32_t mods_count;
  uint32_t mods_addr;

  uint8_t syms[16];

  uint32_t mmap_length;
  uint32_t mmap_addr;

  uint32_t drives_length;
  uint32_t drives_addr;

  uint32_t config_table;
  uint32_t boot_loader_name;
  uint32_t apm_table;

  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;

  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;
  uint8_t framebuffer_type;
};
typedef struct multiboot_info multiboot_info_t;

struct multiboot_mmap
{
  uint32_t size;
  uint64_t addr;
  uint64_t len;
  uint32_t type;
};
typedef struct multiboot_mmap multiboot_mmap_t;

typedef struct 
{
  uint32_t mod_start;
  uint32_t mod_end;
  uint32_t string;
  uint32_t reserved;
} multiboot_mods_t;

#endif