//
// Created by 赵明明 on 2019/12/12.
//
#include "intr.h"
#include "gate.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"

Build_IRQ(0x20)

Build_IRQ(0x21)

Build_IRQ(0x22)

Build_IRQ(0x23)

Build_IRQ(0x24)

Build_IRQ(0x25)

Build_IRQ(0x26)

Build_IRQ(0x27)

Build_IRQ(0x28)

Build_IRQ(0x29)

Build_IRQ(0x2a)

Build_IRQ(0x2b)

Build_IRQ(0x2c)

Build_IRQ(0x2d)

Build_IRQ(0x2e)

Build_IRQ(0x2f)

Build_IRQ(0x30)

Build_IRQ(0x31)

Build_IRQ(0x32)

Build_IRQ(0x33)

Build_IRQ(0x34)

Build_IRQ(0x35)

Build_IRQ(0x36)

Build_IRQ(0x37)

void (*interrupt[24])(void) = {
        IRQ0x20_interrupt,
        IRQ0x21_interrupt,
        IRQ0x22_interrupt,
        IRQ0x23_interrupt,
        IRQ0x24_interrupt,
        IRQ0x25_interrupt,
        IRQ0x26_interrupt,
        IRQ0x27_interrupt,
        IRQ0x28_interrupt,
        IRQ0x29_interrupt,
        IRQ0x2a_interrupt,
        IRQ0x2b_interrupt,
        IRQ0x2c_interrupt,
        IRQ0x2d_interrupt,
        IRQ0x2e_interrupt,
        IRQ0x2f_interrupt,
        IRQ0x30_interrupt,
        IRQ0x31_interrupt,
        IRQ0x32_interrupt,
        IRQ0x33_interrupt,
        IRQ0x34_interrupt,
        IRQ0x35_interrupt,
        IRQ0x36_interrupt,
        IRQ0x37_interrupt
};

void intr_init() {
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