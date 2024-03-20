/*
 * Ryan Jacoby <ryjacoby@calpoly.edu>
 * fragaria/src/multiboot.h
 *
 * Header for mulitboot struct definitions
 * 
 */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H                             1

#include <stdint.h>

#define MULTIBOOT_MAGIC                         0x36D76289

struct multiboot_table_header {
        uint32_t total_size;
        uint32_t reserved;
} __attribute__((packed));

struct multiboot_header {
        uint32_t type;
        uint32_t size;
} __attribute__((packed));

#define MULTIBOOT_TYPE_BASIC_MEM                4
struct multiboot_basic_memory {
        struct multiboot_header header;
        uint32_t mem_lower;
        uint32_t mem_upper;
} __attribute__((packed));

#define MULTIBOOT_TYPE_BOOT_DEV                 5
struct multiboot_BIOS_boot_dev {
        struct multiboot_header header;
        uint32_t biosdev;
        uint32_t partition;
        uint32_t sub_partition;
} __attribute__((packed));

#define MULTIBOOT_CMDLINE                       1
struct multiboot_cmd_line {
        struct multiboot_header header;
        char string[];
} __attribute__((packed));

#define MULTIBOOT_MODULES                       3
struct multiboot_modules {
        struct multiboot_header header;
        uint32_t mod_start;
        uint32_t mod_end;
        char string[];
} __attribute__((packed));

#define MULTIBOOT_ELF_SYMBOLS                   9

#define ELF_SHT_NULL                            0x00
#define ELF_SHT_PROGBITS                        0x01
#define ELF_SHT_SYMTAB                          0x02
#define ELF_SHT_STRTAB                          0x03
#define ELF_SHT_RELA                            0x04
#define ELF_SHT_HASH                            0x05
#define ELF_SHT_DYNAMIC                         0x06
#define ELF_SHT_NOTE                            0x07
#define ELF_SHT_NOBITS                          0x08
#define ELF_SHT_REL                             0x09
#define ELF_SHT_SHLIB                           0x0A
#define ELF_SHT_DYNSYM                          0x0B
#define ELF_SHT_INIT_ARRAY                      0x0E
#define ELF_SHT_FINI_ARRAY                      0x0F
#define ELF_SHT_PREINIT_ARRAY                   0x10
#define ELF_SHT_GROUP                           0x11
#define ELF_SHT_SYMTAB_SHNDX                    0x12
#define ELF_SHT_NUM                             0x13

#define ELF_SHF_WRITE                           0x01
#define ELF_SHF_ALLOC                           0x02
#define ELF_SHF_EXECINSTR                       0x04
#define ELF_SHF_MERGE                           0x10
#define ELF_SHF_STRINGS                         0x20
#define ELF_SHF_INFO_LINK                       0x40
#define ELF_SHF_LINK_ORDER                      0x80
#define ELF_SHF_OS_NONCONFORMING                0x100
#define ELF_SHF_GROUP                           0x200
#define ELF_SHF_TLS                             0x400
#define ELF_SHF_MASKOS                          0x0FF00000
#define ELF_SHF_MASKPROC                        0xF0000000
#define ELF_SHF_ORDERED                         0x04000000
#define ELF_SHF_EXCLUDE                         0x08000000

struct elf_section_header {
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        uint64_t sh_addr;
        uint64_t sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
} __attribute__((packed));

struct multiboot_elf_symbols {
        struct multiboot_header header;
        uint16_t num;
        uint16_t entsize;
        uint16_t shndx;
        uint16_t reserved;
        uint32_t padding;                       /* TODO: why do we need???? */
        struct elf_section_header headers[];
} __attribute__((packed));

#define MULTIBOOT_MEM_MAP                       6
#define MULTIBOOT_MM_TYPE_RAM                   1
#define MULTIBOOT_MM_TYPE_ACPI                  3
#define MULTIBOOT_MM_TYPE_HIBERNATE             4
struct multiboot_mm_entry {
        uint64_t base_addr;
        uint64_t length;
        uint32_t type;
        uint32_t reserved;
} __attribute__((packed));

struct multiboot_mem_map {
        struct multiboot_header header;
        uint32_t entry_size;
        uint32_t entry_version;
        struct multiboot_mm_entry entries[];
} __attribute__((packed));

#define MULTIBOOT_BL_NAME                       2
struct multiboot_bl_name {
        struct multiboot_header header;
        char string[];
} __attribute__((packed));

#define MULTIBOOT_APM_TABLE                     10
struct multiboot_apm_table {
        struct multiboot_header header;
        uint16_t version;
        uint16_t cseg;
        uint32_t offset;
        uint16_t cseg_16;
        uint16_t dseg;
        uint16_t flags;
        uint16_t cseg_len;
        uint16_t cseg_16_len;
        uint16_t dseg_len;
} __attribute__((packed));

#define MULTIBOOT_VBE_INFO                      7
struct multiboot_vbe_info {
        struct multiboot_header header;
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;
        uint8_t vbe_cotrol_info[512];
        uint8_t vbe_mode_info[256];
} __attribute__((packed));

#define MULTIBOOT_FRAMEBUFF_INFO                8
#define MULTIBOOT_FRAMEBUFF_TYPE_INDEXED        0
#define MULITBOOT_FRAMEBUFF_TYPE_RGB            1
#define MULTIBOOT_FRAMEBUFF_TYPE_EGA            2
struct multiboot_color_descriptor {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
} __attribute__((packed));

struct multiboot_framebuff_palette {
        uint32_t framebuff_palette_num_colors;
        struct multiboot_color_descriptor palette[];
} __attribute__((packed));

struct multiboot_framebuff_rgb {
        uint8_t framebuff_red_field_position;
        uint8_t framebuff_red_mask_size;
        uint8_t framebuff_green_field_position;
        uint8_t framebuff_green_mask_size;
        uint8_t framebuff_blue_field_position;
        uint8_t framebuff_blue_mask_size;
} __attribute__((packed));

struct multiboot_framebuff_info {
        struct multiboot_header header;
        uint64_t framebuffer_addr;
        uint32_t framebuffer_pitch;
        uint32_t framebuffer_width;
        uint32_t framebuffer_height;
        uint8_t framebuffer_bpp;
        uint8_t framebuffer_type;
        uint8_t reserved;
        union {
                struct multiboot_framebuff_palette palette;
                struct multiboot_framebuff_rgb rgb;
        } color_info;
        
} __attribute__((packed));

#endif /* #ifndef MULTIBOOT_H */
