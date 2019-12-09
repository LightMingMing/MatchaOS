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
