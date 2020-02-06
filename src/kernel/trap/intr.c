//
// Created by 赵明明 on 2019/12/12.
//
#include "intr.h"

#if APIC
#include "apic.h"
#else
#include "8259A.h"
#endif

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

Build_IRQ(0xC8)

Build_IRQ(0xC9)

Build_IRQ(0xCA)

Build_IRQ(0xCB)

Build_IRQ(0xCC)

Build_IRQ(0xCD)

Build_IRQ(0xCE)

Build_IRQ(0xCF)

Build_IRQ(0xD0)

Build_IRQ(0xD1)

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

void (*IPI[10])(void) = {
        IRQ0xC8_interrupt,
        IRQ0xC9_interrupt,
        IRQ0xCA_interrupt,
        IRQ0xCB_interrupt,
        IRQ0xCC_interrupt,
        IRQ0xCD_interrupt,
        IRQ0xCE_interrupt,
        IRQ0xCF_interrupt,
        IRQ0xD0_interrupt,
        IRQ0xD1_interrupt
};

void intr_init() {
#if APIC
    apic_init();
#else
    init_8259A();
#endif
}