//
// Created by 赵明明 on 2019/12/21.
//

#ifndef _TEST_H
#define _TEST_H

#include "lib/stdio.h"
#include "lib/x86.h"

void test_format_print() {
    // Linear Address of Frame Buffer
    int *address = (int *) FB_vir_address;
    int i, j, ch;
    for (i = 0; i < 1440 * pos.y_char_size; i++) {
        *((char *) address + 0) = (char) 0xff;
        *((char *) address + 1) = (char) 0x00;
        *((char *) address + 2) = (char) 0x00;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }
    for (i = 0; i < 1440 * pos.y_char_size; i++) {
        *((char *) address + 0) = (char) 0x00;
        *((char *) address + 1) = (char) 0xff;
        *((char *) address + 2) = (char) 0x00;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }
    for (i = 0; i < 1440 * pos.y_char_size; i++) {
        *((char *) address + 0) = (char) 0x00;
        *((char *) address + 1) = (char) 0x00;
        *((char *) address + 2) = (char) 0xff;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }

    pos.x_position = 0;
    pos.y_position = 3;
    pos.cur_address = pos.FB_address + pos.y_position * pos.y_char_size * pos.x_resolution;
    print_color(GREEN, BLACK, "ascii table");
    for (i = 0; i < 4; i++) {
        pos.x_position = 0;
        pos.y_position++;
        pos.cur_address =
                pos.FB_address + pos.y_position * pos.y_char_size * pos.x_resolution + pos.x_position * pos.x_char_size;
        for (j = 0; j < 64; j++) {
            ch = (char) (64 * i + j);
            if (ch == '\n' || ch == '\t')
                ch = ' ';
            print_color(YELLOW, BLACK, "%-2c", ch);
        }
    }

    pos.x_position = 0;
    pos.y_position++;
    pos.cur_address = pos.FB_address + pos.y_position * pos.y_char_size * pos.x_resolution;

    print_color(GREEN, BLACK, "string\n");
    println("Hello, world");
    println("I am %s!", "ZhaoMingMing");
    println("I am %20s!", "ZhaoMingMing");
    println("I am %-20s!", "ZhaoMingMing");

    print_color(GREEN, BLACK, "unsigned int\n");
    println("[%u]", 19951026);
    println("[%012u]", 19951026);
    println("[%-12u]", 19951026);

    print_color(GREEN, BLACK, "positive int\n");
    println("[%d]", 19951026);
    println("[%+d]", 19951026);
    println("[%012d]", 19951026);
    println("[%+012d]", 19951026);
    println("[%-12d]", 19951026);
    println("[%+-12d]", 19951026);

    print_color(GREEN, BLACK, "negative int\n");
    println("[%d]", -19951026);
    println("[%+d]", -19951026);
    println("[%012d]", -19951026);
    println("[%+012d]", -19951026);
    println("[%-12d]", -19951026);
    println("[%+-12d]", -19951026);

    print_color(GREEN, BLACK, "octal\n");
    println("[%o]", 01026);
    println("[%+o]", 01026);
    println("[%012o]", 01026);
    println("[%+012o]", 01026);

    print_color(GREEN, BLACK, "hex\n");
    println("[%x]", 0xffff0000);
    println("[%lx]", 0xffff0000ffff0000);
    println("[%25lx]", 0xffff0000ffff0000);
    println("[%025lx]", 0xffff0000ffff0000);
    println("[%-25lx]", 0xffff0000ffff0000);
    println("[%#25lx]", 0xffff0000ffff0000);
    println("[%#-25lx]", 0xffff0000ffff0000);
    println("[%#025lx]", 0xffff0000ffff0000);
    println("[%#025lX]", 0xffff0000ffff0000);

    print_color(GREEN, BLACK, "Nice, very good!\n");
}

int test_DE() {
    return 1 / 0;
}

int test_read_PF() {
    return *(int *) 0xffff80000aa00000;
}

int test_write_PF() {
    *(int *) 0xffff80000aa00000 = 1;
}

void test_mem_info() {
    unsigned long total_available_memory = 0;
    unsigned long total_available_pages = 0;

    for (int i = 0; i < mem_map.length; i++) {
        println("address: %#018lx  length: %#018x  type: %d", mem_map.map[i].addr,
                mem_map.map[i].length, mem_map.map[i].type);
        if (mem_map.map[i].type == E820_ARM) {
            total_available_memory += mem_map.map[i].length;
            unsigned long start = align_upper_2m(mem_map.map[i].addr);
            unsigned long end = align_lower_2m(mem_map.map[i].addr + mem_map.map[i].length);
            total_available_pages = (end - start) >> PAGE_SHIFT_2M;
        }
    }
    println("Total available memory size: %uMB, total available page is: %u", total_available_memory >> 20u,
            total_available_pages);

    println("code:   [%#018lx, %#018lx)", mem_info.start_code, mem_info.end_code);
    println("data:   [%#018lx, %#018lx)", mem_info.start_data, mem_info.end_data);
    println("rodata: [%#018lx, %#018lx)", mem_info.start_rodata, mem_info.end_rodata);
    println("brk:    [%#018lx, %#018lx)", 0, mem_info.end_brk);
    println("end_of_struct: %#lx", mem_info.end_of_struct);
    println("bit_map addr:%#018lx, size:%04u, length:%u", mem_info.bit_map, mem_info.bit_count, mem_info.bit_length);
    println("page    addr:%#018lx, size:%04u, length:%u", mem_info.page, mem_info.page_count, mem_info.page_length);
    println("zone    addr:%#018lx, size:%04u, length:%u", mem_info.zone, mem_info.zone_count, mem_info.zone_length);
}

void test_get_CR3() {
    unsigned long *global_cr3 = get_CR3();
    print_color(INDIGO, BLACK, "global_cr3:   %#018lx\n", global_cr3);
    print_color(INDIGO, BLACK, "*global_cr3:  %#018lx\n", *phy_to_vir(global_cr3) & (~0xFFUL));
    print_color(INDIGO, BLACK, "**global_cr3: %#018lx\n", *phy_to_vir(*phy_to_vir(global_cr3) & (~0xFFUL)) & (~0xFFUL));
}

void test_alloc_pages(int n) {
    struct Page *head = NULL, *page = NULL;

    print_color(GREEN, BLACK, "Before alloc page...\n");
    for (int i = 0; i < mem_info.zone_count; i++) {
        print_color(GREEN, BLACK, "zone[%d] using_count:%d", i, mem_info.zone[i].page_using_count);
        print_color(GREEN, BLACK, " free_count:%d\n", mem_info.zone[i].page_free_count);
    }
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map, *(mem_info.bit_map + 1));

    for (int i = 0; i < n; i++) {
        page = alloc_pages(1, PG_PTable_Mapped | PG_Kernel);
        if (i == 0) head = page;
        // print_color(INDIGO, BLACK, "Page[%02d]  addr:%#018lx%c", i, page->phy_addr, i % 3 == 2 ? '\n' : '\t');
    }

    // print_color(GREEN, BLACK, "%sAfter alloc %d page...\n", n % 3 == 0 ? "" : "\n", n);
    print_color(GREEN, BLACK, "After alloc %d page...\n", n);
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map, *(mem_info.bit_map + 1));

    free_pages(head, n);
    print_color(GREEN, BLACK, "After free %d page...\n", n);
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map, *(mem_info.bit_map + 1));
    for (int i = 0; i < mem_info.zone_count; i++) {
        print_color(GREEN, BLACK, "zone[%d] using_count:%d", i, mem_info.zone[i].page_using_count);
        print_color(GREEN, BLACK, " free_count:%d\n", mem_info.zone[i].page_free_count);
    }
}

void test_kmalloc() {
    unsigned long *chunk1 = NULL, *chunk2 = NULL;
    unsigned long *chunk = NULL, *chunk3, *chunk4, *chunk5, *chunk6;

    print_color(GREEN, BLACK, "Before kmalloc : bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map,
                *(mem_info.bit_map + 1));
    print_color(GREEN, BLACK, "total free: %d, total using: %d\n", kmalloc_cache[3].total_free,
                kmalloc_cache[3].total_using);
    // test kmalloc small chunk
    int chunk_size = 256; // 256 bytes
    chunk1 = kmalloc(chunk_size);
    chunk2 = kmalloc(chunk_size);
    if (chunk2 - chunk1 != (chunk_size / 8)) {
        print_color(RED, BLACK, "kmalloc error: chunk1 addr: %#018lx, chunk2 addr: %#018lx\n", chunk1, chunk2);
    }
    for (int i = 0; i < PAGE_SIZE_2M / chunk_size; i++) {
        chunk = kmalloc(chunk_size);
    }
    print_color(YELLOW, BLACK, "chunk1 addr: %#018lx\n", chunk1);
    print_color(YELLOW, BLACK, "chunk  addr: %#018lx\n", chunk);
    print_color(GREEN, BLACK, "After kmalloc more then one page size small chunk: bit_map[0:1]= [%#018lx, %#018lx]\n",
                *mem_info.bit_map,
                *(mem_info.bit_map + 1));
    print_color(GREEN, BLACK, "total free: %d, total using: %d\n", kmalloc_cache[3].total_free,
                kmalloc_cache[3].total_using);
    for (int i = 0; i < PAGE_SIZE_2M / chunk_size; i++) {
        kfree((unsigned long *) ((unsigned char *) chunk1 + i * chunk_size));
    }
    kfree(chunk);
    kfree(chunk - chunk_size / 8);

    print_color(GREEN, BLACK, "After kfree: total free=%d, total using: %d\n", kmalloc_cache[3].total_free,
                kmalloc_cache[3].total_using);

    // test kmalloc large chunk
    chunk_size = 0x100000;  // 1MB
    chunk1 = kmalloc(chunk_size);
    chunk2 = kmalloc(chunk_size);
    chunk3 = kmalloc(chunk_size);
    chunk4 = kmalloc(chunk_size);
    chunk5 = kmalloc(chunk_size);
    chunk6 = kmalloc(chunk_size);
    if (chunk2 - chunk1 != (chunk_size / 8)) {
        print_color(RED, BLACK, "kmalloc error: chunk1 addr: %#018lx, chunk2 addr: %#018lx\n", chunk1, chunk2);
    }
    print_color(GREEN, BLACK, "After kmalloc more then one page size large chunk: bit_map[0:1]= [%#018lx, %#018lx]\n",
                *mem_info.bit_map,
                *(mem_info.bit_map + 1));

    kfree(chunk6);
    kfree(chunk5);
    kfree(chunk4);
    if (kmalloc_cache[15].total_free != 3) {
        print_color(RED, BLACK, "After kfree three 1MB size chunks: total free=%d, total using: %d\n",
                    kmalloc_cache[15].total_free,
                    kmalloc_cache[15].total_using);
    }
    kfree(chunk3); // free a slab, total_free should equals 2.
    if (kmalloc_cache[15].total_free != 2) {
        print_color(RED, BLACK, "After kfree four 1MB size chunks: total free=%d, total using: %d\n",
                    kmalloc_cache[15].total_free,
                    kmalloc_cache[15].total_using);
    }
    kfree(chunk2);
    kfree(chunk1);

    if ((chunk = kmalloc(chunk_size)) != chunk1) {
        print_color(RED, BLACK, "Kfree error: chunk addr is %#018lx, chunk1 addr is %#018lx\n", chunk, chunk1);
    }
    if ((chunk = kmalloc(chunk_size)) != chunk2) {
        print_color(RED, BLACK, "Kfree error: chunk addr is %#018lx, chunk2 addr is %#018lx\n", chunk, chunk2);
    }
    kfree(chunk1);
    kfree(chunk2);
}

void *test_ctor(void *chunk, unsigned long arg) {
    *((unsigned long *) chunk) = arg;
    return chunk;
}

void *test_dtor(void *chunk, unsigned long arg) {
    *((unsigned long *) chunk) = arg;
    return chunk;
}

void test_create_and_destroy_slab_cache() {
    struct Slab_cache *cache = NULL;
    struct Slab *slab = NULL;

    cache = slab_cache_create(0x100000, test_ctor, test_dtor);
    print_color(GREEN, BLACK, "create a slab cache\n");
    print_color(GREEN, BLACK, "slab_cache->size=        %d\n", cache->size);
    print_color(GREEN, BLACK, "slab_cache->total_using= %d\n", cache->total_using);
    print_color(GREEN, BLACK, "slab_cache->total_free=  %d\n", cache->total_free);
    print_color(GREEN, BLACK, "slab_cache->ctor=        %#018lx\n", cache->ctor);
    print_color(GREEN, BLACK, "slab_cache->dtor=        %#018lx\n", cache->dtor);

    slab = cache->cache_pool;
    print_color(GREEN, BLACK, "slab->page->phy_addr= %#018lx\n", slab->page->phy_addr);
    print_color(GREEN, BLACK, "slab->using_count=    %d\n", slab->using_count);
    print_color(GREEN, BLACK, "slab->free_count=     %d\n", slab->free_count);
    print_color(GREEN, BLACK, "slab->colormap=       %#018lx\n", *slab->color_map);
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map,
                *(mem_info.bit_map + 1));

    void *chunk[6] = {};
    for (int i = 0; i < 6; i++) {
        chunk[i] = slab_malloc(cache, 1);
        print_color(GREEN, BLACK, "slab_malloc chunk=%#018lx, arg=%d\n", chunk[i], *(unsigned long *) chunk[i]);
    }
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map,
                *(mem_info.bit_map + 1));
    print_color(GREEN, BLACK, "slab_cache->total_using= %d\n", cache->total_using);
    print_color(GREEN, BLACK, "slab_cache->total_free=  %d\n", cache->total_free);

    int ret = 0;
    for (int i = 0; i < 6; i++) {
        ret = slab_free(cache, chunk[i], 0);
        print_color(GREEN, BLACK, "slab_free ret=%d, arg=%d\n", ret, *(unsigned long *) chunk[i]);
    }
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map,
                *(mem_info.bit_map + 1));
    print_color(GREEN, BLACK, "slab_cache->total_using= %d\n", cache->total_using);
    print_color(GREEN, BLACK, "slab_cache->total_free=  %d\n", cache->total_free);

    slab_cache_destroy(cache);
    print_color(GREEN, BLACK, "After destroy\n");
    print_color(GREEN, BLACK, "slab_cache->size=        %d\n", cache->size);
    print_color(GREEN, BLACK, "slab_cache->total_using= %d\n", cache->total_using);
    print_color(GREEN, BLACK, "slab_cache->total_free=  %d\n", cache->total_free);
    print_color(GREEN, BLACK, "slab_cache->ctor=        %#018lx\n", cache->ctor);
    print_color(GREEN, BLACK, "slab_cache->dtor=        %#018lx\n", cache->dtor);
    print_color(GREEN, BLACK, "*slab_cache->cache_pool= %#018lx\n", cache->cache_pool);
    print_color(GREEN, BLACK, "bit_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bit_map,
                *(mem_info.bit_map + 1));
}

#endif //_TEST_H
