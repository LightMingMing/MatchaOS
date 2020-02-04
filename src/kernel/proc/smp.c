//
// Created by 赵明明 on 2020/2/2.
//

#include "smp.h"
#include "cpu.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../mm/memory.h"

void smp_init() {
    print_color(YELLOW, BLACK, "AP Boot start addr: %#018lx\n", &_APU_boot_start);
    print_color(YELLOW, BLACK, "AP Boot end   addr: %#018lx\n", &_APU_boot_end);
    memcpy(&_APU_boot_start, (void *) phy_to_vir(0x20000), &_APU_boot_end - &_APU_boot_start);

    *(uint32_t *) phy_to_vir(ICR_HIGH) = 0;
    *(uint32_t *) phy_to_vir(ICR_LOW) = 0xc4500; // INIT IPI

    *(uint32_t *) phy_to_vir(ICR_HIGH) = 0;
    *(uint32_t *) phy_to_vir(ICR_LOW) = 0xc4620; // Start-up IPI

    *(uint32_t *) phy_to_vir(ICR_HIGH) = 0;
    *(uint32_t *) phy_to_vir(ICR_LOW) = 0xc4620; // Start-up IPI
}