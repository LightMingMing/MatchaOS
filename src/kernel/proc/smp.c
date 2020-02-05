//
// Created by 赵明明 on 2020/2/2.
//

#include "smp.h"
#include "cpu.h"
#include "proc.h"
#include "spinlock.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../mm/memory.h"
#include "../mm/slab.h"

uint8_t global_i;
spinlock_t smp_lock;

void smp_init() {
    void *tss;

    print_color(YELLOW, BLACK, "AP Boot start addr: %#018lx\n", &_APU_boot_start);
    print_color(YELLOW, BLACK, "AP Boot end   addr: %#018lx\n", &_APU_boot_end);
    memcpy(&_APU_boot_start, (void *) phy_to_vir(0x20000), &_APU_boot_end - &_APU_boot_start);

    wrmmio(ICR_HIGH, 0);
    wrmmio(ICR_LOW, 0xc4500); // INIT IPI

    spin_init(&smp_lock);

    while (global_i < 3) {
        spin_lock(&smp_lock);
        global_i++;

        _stack_start = (unsigned long) (kmalloc(PROC_STACK_SIZE) + PROC_STACK_SIZE); //  head.S
        tss = kmalloc(128);
        set_tss_desc(10 + global_i * 2, tss);
        setup_TSS(tss, _stack_start, _stack_start, _stack_start, _stack_start, _stack_start, _stack_start, _stack_start,
                  _stack_start, _stack_start, _stack_start);

        wrmmio(ICR_HIGH, global_i << 24U);
        wrmmio(ICR_LOW, 0x04620); // Start-up IPI

        wrmmio(ICR_HIGH, global_i << 24U);
        wrmmio(ICR_LOW, 0x04620); // Start-up IPI
    }
}

void Start_SMP() {
    load_TR(10U + global_i * 2);
    print_color(BLACK, WHITE, "AP started... [%d]\n", rdmmio(APIC_ID_REG) >> 24UL);
    spin_unlock(&smp_lock);
    hlt();
}