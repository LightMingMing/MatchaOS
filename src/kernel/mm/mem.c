//
// Created by 赵明明 on 2019/12/9.
//

#include "mem.h"
#include "slab.h"
#include "../lib/x86.h"
#include "../lib/bit.h"
#include "../lib/stdio.h"

static void init_mem_map() {
    struct address_range_descriptor *map_point = NULL;
    map_point = (struct address_range_descriptor *) 0xffff800000007e00;
    for (int i = 0; i < E820_MAX; i++) {
        if (map_point->type == 0 || map_point->type > 3) {
            mem_map.length = i;
            break;
        }
        mem_map.map[i].addr = ((unsigned long) map_point->addr_high << 32u) + map_point->addr_low;
        mem_map.map[i].length = ((unsigned long) map_point->length_high << 32u) + map_point->length_low;
        mem_map.map[i].type = map_point->type;
        map_point++;
    }
}

void frame_buffer_init() {
    unsigned long *pml4t = NULL, *pdpt = NULL, *pdt = NULL; // base
    unsigned long *pml4e = NULL, *pdpe = NULL, *pde = NULL; // base + offset
    unsigned long *addr = NULL;

    pml4t = phy_to_vir(get_CR3());
    pml4e = pml4t + pml4t_off((unsigned long) phy_to_vir(FB_phy_address));
    if (*pml4e == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pml4t(pml4e, mk_pml4t(vir_to_phy(addr), PAGE_KERNEL_PML4T));
    }

    pdpt = phy_to_vir(*pml4e & (~0xFFUL));
    pdpe = pdpt + pdpt_off(FB_phy_address);
    if (*pdpe == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pdpt(pdpe, mk_pdpt(vir_to_phy(addr), PAGE_KERNEL_PDPT));
    }

    pdt = phy_to_vir(*pdpe & (~0xFFUL));
    for (int i = 0; i < FB_length; i += PAGE_SIZE_2M) {
        pde = pdt + pdt_off(FB_phy_address + i);
        set_pdt(pde, mk_pdt(FB_phy_address + i, PAGE_KERNEL_PDT | PAGE_PWT | PAGE_PCD));
    }

    pos.FB_address = (unsigned int *) phy_to_vir(FB_phy_address);
    pos.cur_address = pos.FB_address + (pos.cur_address - (unsigned int *) FB_vir_address);

    flush_TLB();
}

void memory_init() {
    unsigned long total_memory = 0;

    init_mem_map();

    mem_info.start_code = (unsigned long) &_text;
    mem_info.end_code = (unsigned long) &_etext;
    mem_info.start_data = (unsigned long) &_data;
    mem_info.end_data = (unsigned long) &_edata;
    mem_info.start_rodata = (unsigned long) &_rodata;
    mem_info.end_rodata = (unsigned long) &_erodata;
    mem_info.end_brk = (unsigned long) &_end;

    total_memory = mem_map.map[mem_map.length - 1].addr + mem_map.map[mem_map.length - 1].length;

    mem_info.bit_map = (unsigned long *) align_upper_4k(mem_info.end_brk);
    mem_info.bit_count = total_memory >> PAGE_SHIFT_2M;
    mem_info.bit_length = align_upper_byte((mem_info.bit_count + 7) / 8); // bytes
    // init bits map memory
    memset(mem_info.bit_map, 0xff, mem_info.bit_length);

    mem_info.page = (struct Page *) align_upper_4k(mem_info.bit_map + mem_info.bit_length);
    mem_info.page_count = total_memory >> PAGE_SHIFT_2M;
    mem_info.page_length = align_upper_byte(mem_info.page_count * sizeof(struct Page)); // bytes
    // init page memory
    memset(mem_info.page, 0x00, mem_info.page_length);

    mem_info.zone = (struct Zone *) align_upper_4k(mem_info.page + mem_info.page_length);
    mem_info.zone_count = mem_map.length;
    mem_info.zone_length = align_upper_byte(mem_info.zone_count * sizeof(struct Zone)); // bytes
    // init zone memory
    memset(mem_info.zone, 0x00, mem_info.zone_length);

    int z_idx = 0;
    for (int i = 0; i < mem_map.length; i++) {
        unsigned long start_addr, end_addr;
        struct Zone *z;
        struct Page *p;

        if (mem_map.map[i].type != E820_ARM) {
            continue;
        }
        start_addr = align_upper_2m(mem_map.map[i].addr);
        end_addr = align_upper_2m(mem_map.map[i].addr + mem_map.map[i].length);
        if (start_addr == end_addr) {
            continue;
        }

        // zone init
        z = mem_info.zone + z_idx++;
        z->zone_start_addr = start_addr;
        z->zone_end_addr = end_addr;
        z->zone_length = end_addr - start_addr;
        z->page = (struct Page *) (mem_info.page + (start_addr >> PAGE_SHIFT_2M));
        z->page_count = (end_addr - start_addr) >> PAGE_SHIFT_2M;
        z->page_using_count = 0;
        z->page_free_count = z->page_count;

        // page int
        p = z->page;
        for (int j = 0; j < z->page_count; j++, p++) {
            p->zone = z;
            p->attr = 0;
            p->refcount = 0;
            p->phy_addr = start_addr + PAGE_SIZE_2M * j;

            reset(mem_info.bit_map, p->phy_addr >> PAGE_SHIFT_2M);
        }
    }

    mem_info.page->zone = mem_info.zone;
    mem_info.page->attr = 0;
    mem_info.page->refcount = 0;
    mem_info.page->phy_addr = 0;

    mem_info.zone_count = z_idx;
    mem_info.zone_length = align_upper_byte(z_idx * sizeof(struct Zone)); // bytes

    mem_info.end_of_struct = align_upper_byte(mem_info.zone + mem_info.zone_length + sizeof(long) * 32);

    unsigned int max_used_page = (vir_to_phy(mem_info.end_of_struct) >> PAGE_SHIFT_2M) + 1;
    for (int i = 0; i < max_used_page; i++) {
        struct Page *p = mem_info.page + i;
        p->zone->page_free_count--;
        p->zone->page_using_count++;
        set(mem_info.bit_map, i);
        page_init(p, PG_PTable_Mapped | PG_Kernel_Init | PG_Kernel);
    }

    unsigned long *global_cr3 = get_CR3();
    for (int i = 0; i < 10; i++) {
        *(phy_to_vir(global_cr3) + i) = 0UL;
    }
    flush_TLB();
}

void page_table_init() {
    struct Zone *zone = NULL;
    struct Page *page = NULL;

    unsigned long *pml4t = NULL, *pdpt = NULL, *pdt = NULL; // base
    unsigned long *pml4e = NULL, *pdpe = NULL, *pde = NULL; // base + offset
    unsigned long *addr = NULL;

    pml4t = phy_to_vir(get_CR3());

    for (int i = 0; i < mem_info.zone_count; i++) {
        zone = mem_info.zone + i;
        for (int j = 0; j < zone->page_count; j++) {
            page = zone->page + j;

            pml4e = pml4t + pml4t_off((unsigned long) phy_to_vir(page->phy_addr));
            if (*pml4e == 0) {
                addr = kmalloc(PAGE_SIZE_4K);
                set_pml4t(pml4e, mk_pml4t(vir_to_phy(addr), PAGE_KERNEL_PML4T));
            }

            pdpt = phy_to_vir(*pml4e & (~0xFFUL));
            pdpe = pdpt + pdpt_off(page->phy_addr);
            if (*pdpe == 0) {
                addr = kmalloc(PAGE_SIZE_4K);
                set_pdpt(pdpe, mk_pdpt(vir_to_phy(addr), PAGE_KERNEL_PDPT));
            }

            pdt = phy_to_vir(*pdpe & (~0xFFUL));
            pde = pdt + pdt_off(page->phy_addr);
            set_pdt(pde, mk_pdt(page->phy_addr, PAGE_KERNEL_PDT));

            if (j % 48 == 0) {
                print_color(GREEN, BLACK, "\n%#018lx %#018lx %#018lx\n", *pml4e, *pdpe, *pde);
            }
        }
    }

    pml4e = pml4t + pml4t_off(0);
    if (*pml4e == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pml4t(pml4e, mk_pml4t(vir_to_phy(addr), PAGE_KERNEL_PML4T));
    }

    pdpt = phy_to_vir(*pml4e & (~0xFFUL));
    pdpe = pdpt + pdpt_off(0);
    if (*pdpe == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pdpt(pdpe, mk_pdpt(vir_to_phy(addr), PAGE_KERNEL_PDPT));
    }

    pdt = phy_to_vir(*pdpe & (~0xFFUL));
    pde = pdt + pdt_off(0);
    set_pdt(pde, mk_pdt(0, PAGE_KERNEL_PDT));

    flush_TLB();
}

int page_init(struct Page *page, unsigned long flags) {
    page->attr |= flags;
    if (!page->refcount || (page->attr & PG_Shared)) {
        page->refcount++;
    }
    return 1;
}

struct Page *alloc_pages(unsigned int num, unsigned long flags) {
    unsigned long page = 0;
    int zone_end = mem_info.zone_count;
    for (int i = 0; i < zone_end; i++) {
        struct Zone *z;
        unsigned long start, end; // page

        z = mem_info.zone + i;
        if (z->page_free_count < num) {
            continue;
        }
        // 当前最多仅支持64个连续物理页的申请
        if (num > 64) {
            return NULL;
        }

        start = z->zone_start_addr >> PAGE_SHIFT_2M;
        end = z->zone_end_addr >> PAGE_SHIFT_2M;
        for (unsigned int j = start; j < end; j += 64) {
            unsigned long *p = mem_info.bit_map + (j >> 6U);
            unsigned long shift = j & 63U;
            for (unsigned int k = shift; k < 64; k++) {
                if (!((k ? ((*p >> k) | (*(p + 1) << (64U - k))) : *p) &
                      (num == 64 ? 0xffffffffffffffffUL : (1UL << num) - 1))) {
                    page = j + k - shift;
                    break;
                }
            }
            if (page != 0) {
                break;
            }
            for (unsigned int k = 0; k < shift; k++) {
                if (!((k ? ((*(p + 1) >> k) | (*(p + 2) << (64U - k))) : *(p + 1)) &
                      (num == 64 ? 0xffffffffffffffffUL : (1UL << num) - 1))) {
                    page = j + k - shift + 64;
                    break;
                }
            }
            if (page != 0) {
                break;
            }
        }
        if (page > 0) {
            for (int j = 0; j < num; j++) {
                struct Page *p = mem_info.page + page + j;
                p->zone->page_free_count--;
                p->zone->page_using_count++;
                page_init(p, flags);
                set(mem_info.bit_map, page + j);
            }
            return mem_info.page + page;
        }
    }
    return NULL;
}

void free_pages(struct Page *page, unsigned int num) {
    if (page == NULL || num > 64) {
        return;
    }
    for (unsigned int i = 0; i < num; i++, page++) {
        reset(mem_info.bit_map, page->phy_addr >> PAGE_SHIFT_2M);
        page->attr = 0;
        page->refcount = 0;
        page->zone->page_free_count++;
        page->zone->page_using_count--;
    }
}