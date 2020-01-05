//
// Created by 赵明明 on 2019/12/1.
//

#ifndef _MEMORY_H
#define _MEMORY_H

#include "../lib/defs.h"

extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
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
    uint64_t zone_start_addr;
    uint64_t zone_end_addr;
    size_t zone_length; // in bytes

    struct Page *page;
    uint64_t page_count;
    uint64_t page_using_count;
    uint64_t page_free_count;
};

// page attr
#define PG_PTable_Mapped    (1U<<0U)
#define PG_Kernel_Init      (1U<<1U)
#define PG_Device           (1U<<2U)
#define PG_Kernel           (1U<<3U)
#define PG_Shared           (1U<<4U)

struct Page {
    struct Zone *zone;
    uint64_t phy_addr;
    uint32_t attr;
    uint32_t refcount;
};

// PML4 [39:48)
#define PAGE_PML4T_OFFSET   (39UL)
#define PAGE_PML4T_LENGTH   (9UL)

// PDPT [30:39)
#define PAGE_PDPT_OFFSET    (30UL)
#define PAGE_PDPT_LENGTH    (9UL)

// PDT  [21:30)
#define PAGE_PDT_OFFSET     (21UL)
#define PAGE_PDT_LENGTH     (9UL)

// PT   [12:21)
#define PAGE_PT_OFFSET      (12UL)
#define PAGE_PT_LENGTH      (9UL)


static inline unsigned long __offset(unsigned long addr, unsigned long offset, unsigned long length) {
    return ((((unsigned long) phy_to_vir(addr)) >> offset) & ((1UL << length) - 1UL));
}

// Page table attribute

// bit 0:   1 present, 0 not present
#define PAGE_PRESENT    (1UL << 0UL)

// bit 1:   1 read and write, 0 read only
#define PAGE_RW         (1UL << 1UL)

// bit 2:   1 user and supervisor, 0 supervisor
#define PAGE_US         (1UL << 2UL)

// bit 3:   page level write through
#define PAGE_PWT        (1UL << 3UL)

// bit 4:   page level cache disable
#define PAGE_PCD        (1UL << 4UL)

// bit 5:   1 visited, 0 unvisited
#define PAGE_ACCESSED   (1UL << 5UL)

// bit 6:   1 dirty, 0 clean
#define PAGE_DIRTY      (1UL << 6UL)

// bit 7:   1 big page, 0 small page
#define PAGE_PS         (1UL << 7UL)

// bit 8:   1 global, 0 part
#define PAGE_GLOBAL     (1UL << 8UL)

#define PAGE_KERNEL_PML4T   (PAGE_RW | PAGE_PRESENT)
#define PAGE_KERNEL_PDPT    (PAGE_RW | PAGE_PRESENT)
#define PAGE_KERNEL_PDT     (PAGE_PS | PAGE_RW | PAGE_PRESENT)

// Page Map Level 4 Table
typedef struct {
    uint64_t pml4t;
} pml4t_t;
#define mk_pml4t(addr, attr) ((unsigned long)addr|(unsigned long)attr)
#define set_pml4t(ptr, val) (*ptr = val)
#define pml4t_off(addr) (__offset(addr, PAGE_PML4T_OFFSET, PAGE_PML4T_LENGTH))

// Page Directory Point Table
typedef struct {
    uint64_t pdpt;
} pdpt_t;
#define mk_pdpt(addr, attr) ((unsigned long)addr|(unsigned long)attr)
#define set_pdpt(ptr, val) (*ptr = val)
#define pdpt_off(addr) (__offset(addr, PAGE_PDPT_OFFSET, PAGE_PDPT_LENGTH))

// Page Directory Table
typedef struct {
    uint64_t pdt;
} pdt_t;
#define mk_pdt(addr, attr) ((unsigned long)addr|(unsigned long)attr)
#define set_pdt(ptr, val) (*ptr = val)
#define pdt_off(addr) (__offset(addr, PAGE_PDT_OFFSET, PAGE_PDT_LENGTH))

// Page Table
typedef struct {
    uint64_t pt;
} pt_t;
#define mk_pt(addr, attr) ((unsigned long)addr|(unsigned long)attr)
#define set_pt(ptr, val) (*ptr = val)
#define pt_off(page) (__offset(page, PAGE_PT_OFFSET, PAGE_PT_LENGTH))

struct Global_Memory_Descriptor {
    unsigned long *bit_map;
    uint64_t bit_count;
    size_t bit_length;

    struct Page *page;
    uint64_t page_count;
    size_t page_length;

    struct Zone *zone;
    uint64_t zone_count;
    size_t zone_length;

    uint64_t start_code, end_code;
    uint64_t start_data, end_data;
    uint64_t start_rodata, end_rodata;
    uint64_t end_brk;
    uint64_t end_of_struct;
};

struct Global_Memory_Descriptor mem_info = {};

void memory_init();

void frame_buffer_init();

void page_table_init();

int page_init(struct Page *page, unsigned long flags);

struct Page *alloc_pages(unsigned int num, unsigned long flags);

void free_pages(struct Page *page, unsigned int num);

#endif //_MEMORY_H
