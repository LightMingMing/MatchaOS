//
// Created by 赵明明 on 2019/12/9.
//

#include "memory.h"

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
        unsigned int tmp;

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