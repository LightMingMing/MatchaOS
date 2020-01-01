//
// Created by 赵明明 on 2019/12/29.
//

#include "slab.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "memory.h"

struct Slab *kmalloc_create(unsigned long size) {
    struct Page *page = NULL;
    struct Slab *slab = NULL;
    unsigned long *vir_addr = NULL;
    unsigned long free_mem_size;

    page = alloc_pages(1, PG_Kernel);
    if (page == NULL) {
        print_color(RED, BLACK, "kmalloc_create error: page == NULL\n");
        return NULL;
    }
    switch (size) {
        case 0x20:  // 32
        case 0x40:  // 64
        case 0x80:  // 128
        case 0x100: // 256
        case 0x200: // 512
            free_mem_size = PAGE_SIZE_2M - sizeof(struct Slab) - ((PAGE_SIZE_2M / size) >> 3U);
            vir_addr = phy_to_vir(page->phy_addr);
            slab = (struct Slab *) ((unsigned char *) vir_addr + free_mem_size);

            list_init(&slab->list);
            slab->free_count = free_mem_size / size;
            slab->using_count = 0;
            slab->page = page;
            slab->vir_addr = vir_addr;

            slab->color_count = slab->free_count;
            slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6UL) << 3UL;
            slab->color_map = (unsigned long *) ((unsigned char *) slab + sizeof(struct Slab));
            memset(slab->color_map, 0xff, slab->color_length);

            for (unsigned int i = 0; i < slab->color_count; i++) {
                *(slab->color_map + (i >> 6UL)) ^= (1UL << (i & 63UL));
            }
            return slab;
        case 0x400:     // 1KB
        case 0x800:     // 2KB
        case 0x1000:    // 4KB
        case 0x2000:    // 8KB
        case 0x4000:    // 16KB
        case 0x8000:    // 32KB
        case 0x10000:   // 64KB
        case 0x20000:   // 128KB
        case 0x40000:   // 256KB
        case 0x80000:   // 512KB
        case 0x100000:   // 1MB
            slab = kmalloc(sizeof(struct Slab));

            list_init(&slab->list);
            slab->free_count = PAGE_SIZE_2M / size;
            slab->using_count = 0;
            slab->page = page;
            slab->vir_addr = phy_to_vir(page->phy_addr);

            slab->color_count = slab->free_count;
            slab->color_length = ((slab->color_count + sizeof(unsigned long) * 8 - 1) >> 6UL) << 3UL;
            slab->color_map = (unsigned long *) kmalloc(slab->color_length);
            memset(slab->color_map, 0xff, slab->color_length);
            for (unsigned int i = 0; i < slab->color_count; i++) {
                *(slab->color_map + (i >> 6UL)) ^= 1UL << (i & 63UL);
            }
            return slab;
        default:
            print_color(RED, BLACK, "kmalloc_create error: wrong size '%#010lx'\n", size);
            free_pages(page, 1);
            return NULL;
    }
}

void slab_init() {
    struct Slab *slab = NULL;
    struct Slab_cache *cache = NULL;
    unsigned long page_nr;
    unsigned long temp_addr = mem_info.end_of_struct;

    cache = kmalloc_cache;
    for (int i = 0; i < 16; i++, cache++) {
        unsigned long size = 1UL << (5UL + (unsigned) i); // 32 bytes - 1 MB

        slab = (struct Slab *) mem_info.end_of_struct;
        list_init(&slab->list);
        slab->free_count = PAGE_SIZE_2M / size;
        slab->using_count = 0;

        mem_info.end_of_struct += sizeof(struct Slab);

        slab->color_map = (unsigned long *) mem_info.end_of_struct;
        slab->color_count = slab->free_count;
        slab->color_length = ((slab->free_count + sizeof(unsigned long) * 8 - 1) >> 6UL) << 3UL;
        memset(slab->color_map, 0xff, slab->color_length);
        for (unsigned int j = 0; j < slab->color_count; j++) {
            *(slab->color_map + (j >> 6UL)) ^= 1UL << ((j & 63UL));
        }

        cache->size = size;
        cache->total_using = 0;
        cache->total_free = slab->free_count;
        cache->cache_pool = slab;

        mem_info.end_of_struct = (mem_info.end_of_struct + slab->color_length + sizeof(unsigned long) * 10) &
                                 (~(sizeof(long) - 1UL));
    }

    page_nr = vir_to_phy(mem_info.end_of_struct) >> PAGE_SHIFT_2M;
    for (int i = vir_to_phy(temp_addr) >> PAGE_SHIFT_2M; i <= page_nr; i++) {
        struct Page *page = mem_info.pages + i;
        page_init(page, PG_PTable_Mapped | PG_Kernel_Init | PG_Kernel);
    }

    page_nr += 1;
    for (int i = 0; i < 16; i++, page_nr++) {
        struct Page *page = mem_info.pages + page_nr;
        page_init(page, PG_PTable_Mapped | PG_Kernel_Init | PG_Kernel);

        kmalloc_cache[i].cache_pool->vir_addr = phy_to_vir(page->phy_addr);
        kmalloc_cache[i].cache_pool->page = page;
    }
}

void *kmalloc(unsigned long size) {
    struct Slab_cache *cache = NULL;
    struct Slab *slab = NULL;

    for (int i = 0; i < 16; i++) {
        if (kmalloc_cache[i].size >= size) {
            cache = &kmalloc_cache[i];
            break;
        }
    }
    if (cache == NULL) {
        print_color(RED, BLACK, "kmalloc error: size '%#010lx' is too long\n", size);
        return NULL;
    }
    slab = cache->cache_pool;

    if (cache->total_free != 0) {
        // 索引有空闲块的Slab
        for (;;) {
            if (slab->free_count == 0) {
                slab = container_of((&slab->list)->next, struct Slab, list);
            } else {
                break;
            }
        }
    } else {
        // 创建一个新的slab
        slab = kmalloc_create(cache->size);
        if (slab == NULL) {
            print_color(RED, BLACK, "kmalloc error: slab = NULL\n");
            return NULL;
        }
        cache->total_free += slab->free_count;
        list_add_to_before(&cache->cache_pool->list, &slab->list);
    }
    for (unsigned int i = 0; i < slab->color_count; i++) {
        if (*(slab->color_map + (i >> 6U)) == 0xffffffffffffffff) {
            i += 63;
            continue;
        }
        if (!(*(slab->color_map + (i >> 6U)) & (1UL << (i & 63UL)))) {
            *(slab->color_map + (i >> 6U)) |= (1UL << (i & 63UL));
            slab->free_count--;
            slab->using_count++;

            cache->total_free--;
            cache->total_using++;

            return (void *) ((char *) slab->vir_addr + cache->size * i);
        }
    }
    print_color(RED, BLACK, "kmalloc error: no available memory\n");
    return NULL;
}
