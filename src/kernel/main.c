//
// Created by 赵明明 on 2019/11/18.
//
#include "lib/stdio.h"
#include "lib/x86.h"
#include "trap/gate.h"
#include "trap/trap.h"
#include "mm/memory.h"

extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

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

    struct address_range_descriptor *map_point = NULL;
    unsigned long total_memory = 0;
    unsigned long total_pages = 0;
    map_point = (struct address_range_descriptor *) 0xffff800000007e00;
    for (i = 0; i < E820_MAX; i++) {
        if (map_point->type == E820_ARM) {
            total_memory += map_point->length_low;
            total_memory += (unsigned long) map_point->length_high << 32u;
        } else if (map_point->type == 0 || map_point->type > 3) {
            mem_map.length = i;
            break;
        }
        mem_map.map[i].addr = ((unsigned long) map_point->addr_high << 32u) + map_point->addr_low;
        mem_map.map[i].length = ((unsigned long) map_point->length_high << 32u) + map_point->length_low;
        mem_map.map[i].type = map_point->type;
        map_point++;
    }
    for (i = 0; i < mem_map.length; i++) {
        println("address: %#018lx  length: %#018x  type: %d", mem_map.map[i].addr,
                mem_map.map[i].length, mem_map.map[i].type);
        unsigned long start, end;
        start = align_upper_2m(mem_map.map[i].addr);
        end = align_lower_2m(mem_map.map[i].addr + mem_map.map[i].length);
        if (mem_map.map[i].type == E820_ARM) {
            if (end > start) {
                total_pages += (end - start) >> PAGE_SHIFT_2M;
            }
        }
    }
    println("Total memory size: %uMB, total pages is: %u", total_memory >> 20u, total_pages);
    mem_info.start_code = (unsigned long) &_text;
    mem_info.end_code = (unsigned long) &_etext;
    mem_info.end_data = (unsigned long) &_edata;
    mem_info.end_brk = (unsigned long) &_end;
    println("start_code: %#lx", mem_info.start_code);
    println("end_code  : %#lx", mem_info.end_code);
    println("end_data  : %#lx", mem_info.end_data);
    println("end_brk   : %#lx", mem_info.end_brk);

    total_memory = mem_map.map[mem_map.length - 1].addr + mem_map.map[mem_map.length - 1].length;

    mem_info.bits_map = (unsigned long *) align_upper_4k(mem_info.end_brk);
    mem_info.bits_size = total_memory >> PAGE_SHIFT_2M;
    mem_info.bits_length = align_upper_byte((mem_info.bits_size + 7) / 8); // bytes
    println("bits_map: %#018lx size:%u length:%u", mem_info.bits_map, mem_info.bits_size, mem_info.bits_length);
    // init bits map memory
    memset(mem_info.bits_map, 0xff, mem_info.bits_length);

    mem_info.pages = (struct Page *) align_upper_4k(mem_info.bits_map + mem_info.bits_length);
    mem_info.pages_size = total_memory >> PAGE_SHIFT_2M;
    mem_info.pages_length = align_upper_byte(mem_info.pages_size * sizeof(struct Page)); // bytes
    println("pages: %#018lx size:%u length:%u", mem_info.pages, mem_info.pages_size, mem_info.pages_length);
    // init pages memory
    memset(mem_info.pages, 0x00, mem_info.pages_length);

    mem_info.zones = (struct Zone *) align_upper_4k(mem_info.pages + mem_info.pages_length);
    mem_info.zones_size = mem_map.length;
    mem_info.zones_length = align_upper_byte(mem_info.zones_size * sizeof(struct Zone)); // bytes
    println("zones: %#018lx size:%u length:%u", mem_info.zones, mem_info.zones_size, mem_info.zones_length);
    // init zones memory
    memset(mem_info.zones, 0x00, mem_info.zones_length);

    __asm__ __volatile__ ("hlt":: :);
}