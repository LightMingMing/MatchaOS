//
// Created by 赵明明 on 2019/12/29.
//

#ifndef _SLAB_H
#define _SLAB_H

#include "../lib/list.h"
#include "../lib/defs.h"
#include "memory.h"

struct Slab {
    list_t list;
    struct Page *page;
    void *vir_addr;

    uint64_t free_count;    // free chunk number
    uint64_t using_count;   // using chunk number

    uint64_t color_length;  // in bytes
    uint64_t color_count;   // free_count + using_count
    unsigned long *color_map;
};

struct Slab_cache {
    uint64_t size;          // chunk size in slabs
    uint64_t total_free;    // total free chunk number
    uint64_t total_using;   // total using chunk number
    struct Slab *cache_pool;
};

struct Slab_cache kmalloc_cache[16];

void slab_init();

void *kmalloc(unsigned long size);

#endif //_SLAB_H
