//
// Created by 赵明明 on 2019/12/1.
//

#ifndef _MEMORY_H
#define _MEMORY_H

#include "../lib/defs.h"

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

struct Zone {
    struct Page *pages;
    uint64_t pages_length;

    uint64_t zone_start_addr;
    uint64_t zone_end_addr;
    uint64_t zone_length;
};

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

#endif //_MEMORY_H
