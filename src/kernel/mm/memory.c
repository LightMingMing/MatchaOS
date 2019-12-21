//
// Created by 赵明明 on 2019/12/9.
//

#include "memory.h"
#include "../lib/x86.h"

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

    mem_info.bits_map = (unsigned long *) align_upper_4k(mem_info.end_brk);
    mem_info.bits_size = total_memory >> PAGE_SHIFT_2M;
    mem_info.bits_length = align_upper_byte((mem_info.bits_size + 7) / 8); // bytes
    // init bits map memory
    memset(mem_info.bits_map, 0xff, mem_info.bits_length);

    mem_info.pages = (struct Page *) align_upper_4k(mem_info.bits_map + mem_info.bits_length);
    mem_info.pages_size = total_memory >> PAGE_SHIFT_2M;
    mem_info.pages_length = align_upper_byte(mem_info.pages_size * sizeof(struct Page)); // bytes
    // init pages memory
    memset(mem_info.pages, 0x00, mem_info.pages_length);

    mem_info.zones = (struct Zone *) align_upper_4k(mem_info.pages + mem_info.pages_length);
    mem_info.zones_size = mem_map.length;
    mem_info.zones_length = align_upper_byte(mem_info.zones_size * sizeof(struct Zone)); // bytes
    // init zones memory
    memset(mem_info.zones, 0x00, mem_info.zones_length);

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
        z = mem_info.zones + z_idx++;
        z->zone_start_addr = start_addr;
        z->zone_end_addr = end_addr;
        z->zone_length = end_addr - start_addr;
        z->pages = (struct Page *) (mem_info.pages + (start_addr >> PAGE_SHIFT_2M));
        z->pages_length = (end_addr - start_addr) >> PAGE_SHIFT_2M;
        z->page_using_count = 0;
        z->page_free_count = z->pages_length;

        // page int
        p = z->pages;
        for (int j = 0; j < z->pages_length; j++, p++) {
            p->zone = z;
            p->attr = 0;
            p->refcount = 0;
            p->phy_addr = start_addr + PAGE_SIZE_2M * j;

            *(mem_info.bits_map + ((p->phy_addr >> PAGE_SHIFT_2M) >> 6u)) ^=
                    1UL << ((p->phy_addr >> PAGE_SHIFT_2M) & (64u - 1u));
        }
    }

    mem_info.pages->zone = mem_info.zones;
    mem_info.pages->attr = 0;
    mem_info.pages->refcount = 0;
    mem_info.pages->phy_addr = 0;

    mem_info.zones_size = z_idx;
    mem_info.zones_length = align_upper_byte(z_idx * sizeof(struct Zone)); // bytes

    mem_info.end_of_struct = align_upper_byte(mem_info.zones + mem_info.zones_length + sizeof(long) * 32);

    unsigned int max_used_page = (vir_to_phy(mem_info.end_of_struct) >> PAGE_SHIFT_2M) + 1;
    for (int i = 0; i < max_used_page; i++) {
        page_init(mem_info.pages + i, PG_PTable_Mapped | PG_Kernel_Init | PG_Active | PG_Kernel);
    }

    // unsigned long *global_cr3 = get_CR3();

    // *(int *) 0xffff80000aa00000 = 1; // Will print "Page Fault"
    // clear PML4 Entry
    for (int i = 0; i < 10; i++) {
        // *(phy_to_vir(global_cr3) + i) = 0UL;
    }
    // *(int *) 0xffff80000aa00000 = 1; // Will not print "Page Fault". TODO Why ?
    flush_TLB();
    // *(int *) 0xffff80000aa00000 = 1; // will not print "Page Fault"

}

void page_init(struct Page *page, unsigned long flags) {
    if (!page->attr) {
        *(mem_info.bits_map + ((page->phy_addr >> PAGE_SHIFT_2M) >> 6u)) |=
                1UL << ((page->phy_addr >> PAGE_SHIFT_2M) & (64UL - 1UL));
        page->attr = flags;
        page->refcount++;
        page->zone->page_free_count--;
        page->zone->page_using_count++;
    } else if ((page->attr & PG_Referenced) || (page->attr & PG_K_Share_To_U) || (flags & PG_Referenced) ||
               (flags & PG_K_Share_To_U)) {
        page->attr |= flags;
        page->refcount++;
    } else {
        *(mem_info.bits_map + ((page->phy_addr >> PAGE_SHIFT_2M) >> 6u)) |=
                1UL << ((page->phy_addr >> PAGE_SHIFT_2M) & (64UL - 1UL));
        page->attr |= flags;
    }
}

struct Page *alloc_pages(unsigned int num, unsigned long flags) {
    unsigned long page = 0;
    int zone_end = mem_info.zones_size;
    for (int i = 0; i < zone_end; i++) {
        struct Zone *z;
        unsigned long start, end; // page

        z = mem_info.zones + i;
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
            unsigned long *p = mem_info.bits_map + (j >> 6U);
            unsigned long shift = j & 63U;
            for (unsigned int k = shift; k < 64; k++) {
                if (!(((*p >> k) | (*(p + 1) << (64U - k))) &
                      (num == 64 ? 0xffffffffffffffffUL : (1UL << num) - 1))) {
                    page = j + k - shift;
                    break;
                }
            }
            if (page != 0) {
                break;
            }
            for (unsigned int k = 0; k < shift; k++) {
                if (!(((*(p + 1) >> k) | (*(p + 2) << (64U - k))) &
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
                struct Page *p = mem_info.pages + page + j;
                page_init(p, flags);
            }
            return mem_info.pages + page;
        }
    }
    return NULL;
}