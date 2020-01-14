//
// Created by 赵明明 on 2020/1/8.
// APIC: Advanced Programmable Interrupt Controller
//

#ifndef _APIC_H
#define _APIC_H

// Memory Mapped Registers For Accessing IO APIC Registers
struct memory_map_regs {
    unsigned int base_phy_addr;
    unsigned char *index_addr; // IO Register Select (index)
    unsigned int *data_addr; // IO Window (data)
} io_apic_map;

void apic_init();

#endif //_APIC_H
