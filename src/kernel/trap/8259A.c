//
// Created by 赵明明 on 2020/1/12.
//

#include "gate.h"
#include "intr.h"
#include "../lib/x86.h"
#include "../lib/stdio.h"

void _8259A_init() {
    for (int i = 0; i < 24; i++) {
        set_intr_gate(0x20 + i, 2, interrupt[i]);
        // print_color(INDIGO, BLACK, "intr vector: %#04lx, intr handling addr: %#018lx\n", 0x20 + i, interrupt[i]);
    }

    // init 8259A

    // 8259A-master ICW 1-4
    io_out8(0x20, 0x11);
    io_out8(0x21, 0x20);
    io_out8(0x21, 0x04);
    io_out8(0x21, 0x01);

    // 8259A-slave  ICW 1-4
    io_out8(0xa0, 0x11);
    io_out8(0xa1, 0x28);
    io_out8(0xa1, 0x02);
    io_out8(0xa1, 0x01);

    // 8259A-M/S    OCW1
    // 屏蔽除键盘以外的所有其它中断
    io_out8(0x21, 0xfd);
    io_out8(0xa1, 0xff);

    sti();
}

void handle_IRQ(unsigned long intr_vector, unsigned long rsp) {
    print_color(RED, BLACK, "handle_IRQ:%#08x\t", intr_vector);
    if (intr_vector == 0x21) { // 键盘中断向量号
        unsigned char code = io_in8(0x60);
        print_color(INDIGO, BLACK, "key code:%#08x\n", code);
    }
    io_out8(0x20, 0x20);
}