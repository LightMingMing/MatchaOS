//
// Created by 赵明明 on 2019/11/18.
//
#include "lib/stdio.h"
#include "lib/x86.h"
#include "trap/gate.h"
#include "trap/trap.h"
#include "trap/intr.h"
#include "mm/memory.h"
#include "proc/proc.h"

void Start_Kernel() {
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

    load_TR(8u);
    setup_TSS(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);
    trap_init();
    // test
    // i = 1 / 0;
    // println("1/0 = %d", i);
    // i = *(int *) 0xffff80000aa00000;
    // *(int *) 0xffff80000aa00000 = 1;

    p.x_position = 0;
    p.y_position = 0;
    p.cur_address = p.FB_address;

    print_color(GREEN, BLACK, "Memory Init\n");
    memory_init();

    unsigned long total_available_memory = 0;
    unsigned long total_available_pages = 0;
    for (i = 0; i < mem_map.length; i++) {
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
    println("start_code   : %#lx\t end_code     : %#lx", mem_info.start_code, mem_info.end_code);
    println("end_data     : %#lx\t end_brk      : %#lx", mem_info.end_data, mem_info.end_brk);
    println("end_of_struct: %#lx", mem_info.end_of_struct);
    println("bits_map : %#018lx, size:%u, length:%u", mem_info.bits_map, mem_info.bits_size, mem_info.bits_length);
    println("pages    : %#018lx, size:%u, length:%u", mem_info.pages, mem_info.pages_size, mem_info.pages_length);
    println("zones    : %#018lx, size:%u, length:%u", mem_info.zones, mem_info.zones_size, mem_info.zones_length);

//    struct Page *page = NULL;
//    for (i = 0; i < 64; i++) {
//        page = alloc_pages(1, PG_PTable_Mapped | PG_Active | PG_Kernel);
//        print_color(INDIGO, BLACK, "Page[%d] \tattr:%#018lx\taddr:%#018lx\t\t", i, page->attr,
//                    page->phy_addr);
//        if ((unsigned) i & 1u)
//            println("");
//    }
    print_color(GREEN, BLACK, "*(mem_info.bits_map)  : %#018lx\n", *mem_info.bits_map);
    print_color(GREEN, BLACK, "*(mem_info.bits_map+1): %#018lx\n", *(mem_info.bits_map + 1));

    intr_init();

    print_color(GREEN, BLACK, "\nproc init\n");
    proc_init();

    __asm__ __volatile__ ("hlt":: :);
}