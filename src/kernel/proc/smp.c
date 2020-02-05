//
// Created by 赵明明 on 2020/2/2.
//

#include "smp.h"
#include "cpu.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../mm/memory.h"
#include "proc.h"
#include "../mm/slab.h"
#include "../test.h"

void smp_init() {
    print_color(YELLOW, BLACK, "AP Boot start addr: %#018lx\n", &_APU_boot_start);
    print_color(YELLOW, BLACK, "AP Boot end   addr: %#018lx\n", &_APU_boot_end);
    memcpy(&_APU_boot_start, (void *) phy_to_vir(0x20000), &_APU_boot_end - &_APU_boot_start);

    wrmmio(ICR_HIGH, 0);
    wrmmio(ICR_LOW, 0xc4500); // INIT IPI

    unsigned long stack_start = (unsigned long) (kmalloc(PROC_STACK_SIZE) + PROC_STACK_SIZE);
    void *tss = kmalloc(128);
    set_tss_desc(12, tss);
    setup_TSS(tss, stack_start, stack_start, stack_start, stack_start, stack_start, stack_start, stack_start,
              stack_start, stack_start, stack_start);

    wrmmio(ICR_HIGH, 1U << 24U); // start AP which apic id is 1
    wrmmio(ICR_LOW, 0x04620); // Start-up IPI

//    wrmmio(ICR_HIGH, 0);
//    wrmmio(ICR_LOW, 0xc4620); // Start-up IPI
//
//    wrmmio(ICR_HIGH, 0);
//    wrmmio(ICR_LOW, 0xc4620); // Start-up IPI
}

void Start_SMP() {
    load_TR(12U);
    uint8_t id = rdmmio(APIC_ID_REG) >> 24UL;
    print_color(BLACK, WHITE, "AP started... [%d]\n", id);
    test_DE(); // test
    hlt();
}