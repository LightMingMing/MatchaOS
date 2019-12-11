//
// Created by 赵明明 on 2019/12/1.
//

#ifndef _MEMORY_H
#define _MEMORY_H

#include "../lib/defs.h"

extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

struct address_range_descriptor {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
};

#define E820_MAX 20
#define E820_ARM 1  // address range memory
#define E820_ARR 2  // address range reserved

struct e820_memory_map {
    int length;
    struct {
        uint64_t addr;
        uint64_t length;
        uint32_t type;
    }__attribute__((packed)) map[E820_MAX];
};

struct e820_memory_map mem_map = {0, {}};

#define PAGE_OFFSET     (0xffff800000000000UL)

#define PAGE_SHIFT_2M   21u
#define PAGE_SHIFT_4K   12u

#define PAGE_SIZE_2M    (1UL << PAGE_SHIFT_2M)
#define PAGE_SIZE_4K    (1UL << PAGE_SHIFT_4K)

#define PAGE_MASK_2M    (~(PAGE_SIZE_2M - 1))
#define PAGE_MASK_4K    (~(PAGE_SIZE_4K - 1))

#define align_upper_2m(addr) (((unsigned long)addr + PAGE_SIZE_2M - 1u) & PAGE_MASK_2M)
#define align_upper_4k(addr) (((unsigned long)addr + PAGE_SIZE_4K - 1u) & PAGE_MASK_4K)
#define align_upper_byte(addr) (((unsigned long)addr + 7u) & (~7UL))
#define align_lower_2m(addr) ((unsigned long)addr & PAGE_MASK_2M)
#define align_lower_4k(addr) ((unsigned long)addr & PAGE_MASK_4K)
#define align_lower_byte(addr) ((unsigned long)addr & (~7UL))

#define vir_to_phy(addr) ((unsigned long)addr - PAGE_OFFSET)
#define phy_to_vir(addr) ((unsigned long*)((unsigned long)addr + PAGE_OFFSET))

struct Zone {
    struct Page *pages;
    uint64_t pages_length;

    uint64_t zone_start_addr;
    uint64_t zone_end_addr;
    uint64_t zone_length;

    uint64_t page_using_count;
    uint64_t page_free_count;
};

// page attr
#define PG_PTable_Mapped    (1U<<0U)
#define PG_Kernel_Init      (1U<<1U)
#define PG_Referenced       (1U<<2U)
#define PG_Dirty            (1U<<3U)
#define PG_Active           (1U<<4U)
#define PG_Up_To_Date       (1U<<5U)
#define PG_Device           (1U<<6U)
#define PG_Kernel           (1U<<7U)
#define PG_K_Share_To_U     (1U<<8U)
#define PG_Slab             (1U<<9U)

struct Page {
    struct Zone *zone;
    uint64_t phy_addr;
    uint32_t attr;
    uint32_t refcount;
};

struct Global_Memory_Descriptor {
    unsigned long *bits_map;
    uint64_t bits_size;
    uint64_t bits_length;

    struct Page *pages;
    uint64_t pages_size;
    uint64_t pages_length;

    struct Zone *zones;
    uint64_t zones_size;
    uint64_t zones_length;

    uint64_t start_code, end_code, end_data, end_brk;
    uint64_t end_of_struct;
};

struct Global_Memory_Descriptor mem_info = {};

void memory_init();

void page_init(struct Page *page, unsigned long flags);

struct Page *alloc_pages(unsigned int num, unsigned long flags);

#endif //_MEMORY_H
