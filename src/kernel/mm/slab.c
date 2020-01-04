//
// Created by 赵明明 on 2019/12/29.
//

#include "slab.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "memory.h"
#include "../lib/bit.h"

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
                reset(slab->color_map, i);
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
                reset(slab->color_map, i);
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
            reset(slab->color_map, j);
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

    if (cache->total_free == 0) {
        // 创建一个新的slab
        slab = kmalloc_create(cache->size);
        if (slab == NULL) {
            print_color(RED, BLACK, "kmalloc error: slab = NULL\n");
            return NULL;
        }
        cache->total_free += slab->free_count;
        list_add_to_before(&cache->cache_pool->list, &slab->list);
    } else {
        // 索引有空闲块的Slab
        for (;;) {
            if (slab->free_count == 0) {
                slab = container_of((&slab->list)->next, struct Slab, list);
            } else {
                break;
            }
        }
    }
    for (unsigned int i = 0; i < slab->color_count; i++) {
        if (*(slab->color_map + (i >> 6U)) == 0xffffffffffffffff) {
            i += 63;
            continue;
        }
        if (!get(slab->color_map, i)) {
            set(slab->color_map, i);
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

int kfree(void *chunk_addr) {
    struct Slab *head = NULL, *slab = NULL;
    struct Slab_cache *cache = NULL;
    void *page_addr = (void *) ((unsigned long) chunk_addr & PAGE_MASK_2M);

    for (int i = 0; i < 16; i++) {
        cache = &kmalloc_cache[i];
        slab = head = cache->cache_pool;
        for (;;) {
            if (slab->vir_addr == page_addr) {
                unsigned int chunk_idx = (chunk_addr - page_addr) / cache->size;
                if (!get(slab->color_map, chunk_idx)) {
                    print_color(RED, BLACK, "kfree error: the chunk %#018lx is not referenced", chunk_addr);
                    return 0;
                }
                reset(slab->color_map, chunk_idx);

                slab->free_count++;
                slab->using_count--;

                cache->total_free++;
                cache->total_using--;

                if (slab->using_count == 0 && cache->total_free > slab->color_count * 3 / 2 &&
                    slab != cache->cache_pool) {
                    unsigned long *tmp_color_map = slab->color_map;

                    list_del(&slab->list);
                    cache->total_free -= slab->color_count;

                    memset(slab->color_map, 0, slab->color_length);
                    slab->color_count = 0;
                    slab->color_length = 0;
                    slab->color_map = NULL;

                    free_pages(slab->page, 1);
                    slab->page = NULL;
                    slab->vir_addr = NULL;

                    if (cache->size > 0x200) { // 1KB ~ 1M
                        kfree(slab);
                        kfree(tmp_color_map);
                    }
                }
                return 1;
            }
            if ((slab = container_of((&slab->list)->next, struct Slab, list)) == head) {
                break;
            }

        }
    }
    print_color(RED, BLACK, "kfree error: the chunk %#018lx is not exist", chunk_addr);
    return 0;
}

struct Slab *slab_create(unsigned long size) {
    struct Slab *slab = NULL;

    slab = kmalloc(sizeof(struct Slab));
    if (slab == NULL) {
        print_color(RED, BLACK, "slab_create error: slab == null\n");
        return NULL;
    }
    list_init(&slab->list);
    slab->using_count = 0;
    slab->free_count = PAGE_SIZE_2M / size;

    // slab->page
    slab->page = alloc_pages(1, PG_Kernel);
    if (slab->page == NULL) {
        print_color(RED, BLACK, "slab_create error: alloc pages == NULL\n");
        kfree(slab);
        return NULL;
    }
    slab->vir_addr = phy_to_vir(slab->page->phy_addr);

    // slab->color_map
    slab->color_count = slab->free_count;
    slab->color_length = (slab->color_count + sizeof(long) * 8 - 1) >> 6UL << 3UL;
    slab->color_map = kmalloc(slab->color_length);
    if (slab->color_map == NULL) {
        print_color(RED, BLACK, "slab_create error: color_map == null\n");
        free_pages(slab->page, 1);
        kfree(slab);
        return NULL;
    }
    memset(slab->color_map, 0xff, slab->color_length);
    for (unsigned int i = 0; i < slab->color_count; i++) {
        reset(slab->color_map, i);
    }
    return slab;
}

struct Slab_cache *slab_cache_create(unsigned long size,
                                     void *(*ctor)(void *, unsigned long),
                                     void *(*dtor)(void *, unsigned long)) {
    struct Slab_cache *cache = NULL;
    struct Slab *slab = NULL;

    // slab_cache
    cache = kmalloc(sizeof(struct Slab_cache));
    if (cache == NULL) {
        print_color(RED, BLACK, "slab_cache_create error: kmalloc Slab_cache == null\n");
        return NULL;
    }
    cache->size = (size + sizeof(long) - 1) & (~(sizeof(long) - 1));
    cache->total_using = 0;
    cache->total_free = 0;
    cache->ctor = ctor;
    cache->dtor = dtor;

    // slab
    slab = slab_create(size);
    if (slab == NULL) {
        print_color(RED, BLACK, "slab_cache_create error: slab == null\n");
        kfree(cache);
        return NULL;
    }
    cache->cache_pool = slab;
    cache->total_free = slab->free_count;

    return cache;
}

int slab_cache_destroy(struct Slab_cache *cache) {
    struct Slab *head = NULL, *slab = NULL, *next = NULL;

    if (cache->total_using != 0) {
        print_color(RED, BLACK, "can't destroy: slab_cache->total_using != 0\n");
        return 0;
    }
    head = slab = cache->cache_pool;
    for (;;) {
        next = container_of((&slab->list)->next, struct Slab, list);

        list_del(&slab->list);
        free_pages(slab->page, 1);
        kfree(slab->color_map);
        kfree(slab);
        memset(slab, 0, sizeof(struct Slab));

        if (next == head || next == NULL) {
            break;
        }
        slab = next;
    }

    kfree(cache);
    memset(cache, 0, sizeof(struct Slab_cache));
    return 1;
}

void *slab_malloc(struct Slab_cache *cache, unsigned long arg) {
    struct Slab *slab = NULL;

    if (cache->total_free == 0) {
        slab = slab_create(cache->size);
        if (slab == NULL) {
            print_color(RED, BLACK, "slab_malloc error: slab = NULL\n");
            return NULL;
        }
        list_add_to_before(&cache->cache_pool->list, &slab->list);
        cache->total_free += slab->free_count;
    } else {
        slab = cache->cache_pool;
        for (;;) {
            if (slab->free_count == 0) {
                slab = container_of((&slab->list)->next, struct Slab, list);
            } else {
                break;
            }
        }
    }

    for (unsigned int i = 0; i < slab->color_count; i++) {
        if (*(slab->color_map + (i >> 6UL)) == 0xffffffffffffffff) {
            i += 63;
            continue;
        }
        if (!get(slab->color_map, i)) {
            set(slab->color_map, i);
            slab->using_count++;
            slab->free_count--;

            cache->total_using++;
            cache->total_free--;

            void *chunk_addr = slab->vir_addr + cache->size * i;
            return cache->ctor == NULL ? chunk_addr : cache->ctor(chunk_addr, arg);
        }
    }
    return NULL;
}

int slab_free(struct Slab_cache *cache, void *addr, unsigned long arg) {
    struct Slab *head = NULL, *slab = NULL;

    head = slab = cache->cache_pool;
    for (;;) {
        if (slab->vir_addr <= addr && addr < slab->vir_addr + PAGE_SIZE_2M) {
            unsigned int idx = (addr - slab->vir_addr) / cache->size;
            slab->using_count--;
            slab->free_count++;
            reset(slab->color_map, idx);

            cache->total_using--;
            cache->total_free++;
            if (cache->dtor != NULL) {
                cache->dtor((char *) slab->vir_addr + cache->size * idx, arg);
            }

            if (slab->using_count == 0 && cache->total_free > slab->color_count * 3 / 2) {
                list_del(&slab->list);
                free_pages(slab->page, 1);
                kfree(slab->color_map);
                kfree(slab);

                cache->total_free -= slab->color_count;
                memset(slab, 0, sizeof(struct Slab));
            }
            return 1;
        }
        if ((slab = container_of((&slab->list)->next, struct Slab, list)) == head) {
            break;
        }
    }
    print_color(RED, BLACK, "slab_free error: the chunk %018lx is not exist\n", addr);
    return 0;
}