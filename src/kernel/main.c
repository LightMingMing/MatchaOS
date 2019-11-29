//
// Created by 赵明明 on 2019/11/18.
//
#include "lib/stdio.h"
#include "trap/gate.h"
#include "trap/trap.h"

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
    i = *(int *) 0xffff80000aa00000;
    // *(int *) 0xffff80000aa00000 = 1;

    __asm__ __volatile__ ("hlt":: :);
}