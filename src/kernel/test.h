//
// Created by 赵明明 on 2019/12/21.
//

#ifndef _TEST_H
#define _TEST_H

#include "lib/stdio.h"
#include "lib/x86.h"

void test_format_print() {
    // Linear Address of Frame Buffer
    int *address = (int *) 0xffff800000a00000;
    int i, j, ch;
    for (i = 0; i < 1440 * p.y_char_size; i++) {
        *((char *) address + 0) = (char) 0xff;
        *((char *) address + 1) = (char) 0x00;
        *((char *) address + 2) = (char) 0x00;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }
    for (i = 0; i < 1440 * p.y_char_size; i++) {
        *((char *) address + 0) = (char) 0x00;
        *((char *) address + 1) = (char) 0xff;
        *((char *) address + 2) = (char) 0x00;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }
    for (i = 0; i < 1440 * p.y_char_size; i++) {
        *((char *) address + 0) = (char) 0x00;
        *((char *) address + 1) = (char) 0x00;
        *((char *) address + 2) = (char) 0xff;
        *((char *) address + 3) = (char) 0x00;
        address++;
    }

    p.x_position = 0;
    p.y_position = 3;
    p.cur_address = p.FB_address + p.y_position * p.y_char_size * p.x_resolution;
    print_color(GREEN, BLACK, "ascii table");
    for (i = 0; i < 4; i++) {
        p.x_position = 0;
        p.y_position++;
        p.cur_address = p.FB_address + p.y_position * p.y_char_size * p.x_resolution + p.x_position * p.x_char_size;
        for (j = 0; j < 64; j++) {
            ch = (char) (64 * i + j);
            if (ch == '\n' || ch == '\t')
                ch = ' ';
            print_color(YELLOW, BLACK, "%-2c", ch);
        }
    }

    p.x_position = 0;
    p.y_position++;
    p.cur_address = p.FB_address + p.y_position * p.y_char_size * p.x_resolution;

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
    println("Total available memory size: %uMB, total available pages is: %u", total_available_memory >> 20u,
            total_available_pages);

    println("code:   [%#018lx, %#018lx)", mem_info.start_code, mem_info.end_code);
    println("data:   [%#018lx, %#018lx)", mem_info.start_data, mem_info.end_data);
    println("rodata: [%#018lx, %#018lx)", mem_info.start_rodata, mem_info.end_rodata);
    println("brk:    [%#018lx, %#018lx)", 0, mem_info.end_brk);
    println("end_of_struct: %#lx", mem_info.end_of_struct);
    println("bits_map addr:%#018lx, size:%04u, length:%u", mem_info.bits_map, mem_info.bits_size, mem_info.bits_length);
    println("pages    addr:%#018lx, size:%04u, length:%u", mem_info.pages, mem_info.pages_size, mem_info.pages_length);
    println("zones    addr:%#018lx, size:%04u, length:%u", mem_info.zones, mem_info.zones_size, mem_info.zones_length);
}

void test_get_CR3() {
    unsigned long *global_cr3 = get_CR3();
    print_color(INDIGO, BLACK, "global_cr3:   %#018lx\n", global_cr3);
    print_color(INDIGO, BLACK, "*global_cr3:  %#018lx\n", *phy_to_vir(global_cr3) & (~0xFFUL));
    print_color(INDIGO, BLACK, "**global_cr3: %#018lx\n", *phy_to_vir(*phy_to_vir(global_cr3) & (~0xFFUL)) & (~0xFFUL));
}

void test_alloc_pages(int n) {
    struct Page *page = NULL;

    print_color(GREEN, BLACK, "Before alloc page...\n");
    print_color(GREEN, BLACK, "bits_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bits_map, *(mem_info.bits_map + 1));

    for (int i = 0; i < n; i++) {
        page = alloc_pages(1, PG_PTable_Mapped | PG_Active | PG_Kernel);
        print_color(INDIGO, BLACK, "Page[%02d]  addr:%#018lx%c", i, page->phy_addr, i % 3 == 2 ? '\n' : '\t');
    }

    print_color(GREEN, BLACK, "%sAfter alloc %d page...\n", n % 3 == 0 ? "" : "\n", n);
    print_color(GREEN, BLACK, "bits_map[0:1]= [%#018lx, %#018lx]\n", *mem_info.bits_map, *(mem_info.bits_map + 1));
}

#endif //_TEST_H