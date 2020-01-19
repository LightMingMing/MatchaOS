//
// Created by 赵明明 on 2020/1/8.
//
#include "apic.h"
#include "intr.h"
#include "gate.h"
#include "../lib/cpu.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../mm/memory.h"
#include "../mm/slab.h"

void local_apic_init() {
    unsigned int eax, ebx, ecx, edx;
    get_cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    print_color(WHITE, BLACK, "CPUID.01, eax:%#010x, ebx:%#010x, ecx:%#010x, edx:%010x\n", eax, ebx, ecx, edx);

    if ((1UL << 9UL) & edx)
        print_color(WHITE, BLACK, "Processor support APIC&xAPIC\t");
    else
        print_color(RED, BLACK, "Processor not support APIC&xAPIC\t");

    if ((1UL << 21UL) & ecx)
        print_color(WHITE, BLACK, "Processor support x2APIC\n");
    else
        print_color(RED, BLACK, "Processor not support x2APIC\n");

    // IA32_APIC_BASE MSR (MSR address 1BH)
    unsigned long value = rdmsr(0x1B);
    value = value | (1UL << 11UL) | (1UL << 10UL);
    wrmsr(0x1B, value);
    value = rdmsr(0x1B);
    print_color(WHITE, BLACK, "IA32_APIC_BASE MSR: %#018lx\n", value);

    if (value >> 11UL & 1UL)
        print_color(WHITE, BLACK, "Enable xAPIC\t");
    else
        print_color(RED, BLACK, "Disable xAPIC\t");

    if (value >> 10UL & 1UL)
        print_color(WHITE, BLACK, "Enable x2APIC\n");
    else
        print_color(RED, BLACK, "Disable x2APIC\n");

    // Spurious interrupt vector register
    value = rdmsr(0x80F);
    // bit 8:  APIC Software Enable/Disable 0:Disabled, 1:Enabled
    // bit 12: EOI-Broadcast Suppression 0:Disabled, 1:Enabled
    //  value = value | (1UL << 8UL) | (1UL << 12UL); TODO support EOI-Broadcast suppression
    value = value | (1UL << 8UL);
    wrmsr(0x80F, value);
    value = rdmsr(0x80F);
    print_color(RED, BLACK, "%#018lx\n", value);
    if (value >> 8UL & 1UL)
        print_color(WHITE, BLACK, "APIC Software Enabled\t");
    else
        print_color(RED, BLACK, "APIC Software Disabled\t");

    if (value >> 12UL & 1UL)
        print_color(WHITE, BLACK, "EOI-Broadcast Suppression Enabled\n");
    else
        print_color(RED, BLACK, "EOI-Broadcast Suppression Disabled\n");

    // x2APIC ID
    // MSR address: 0x802
    value = rdmsr(0x802);
    print_color(WHITE, BLACK, "x2APIC ID: %#010x\n", value);

    // x2APIC version
    // MSR address: 0x803
    value = rdmsr(0x803);
    // Version: bit 0-7
    print_color(WHITE, BLACK, "Version: %#02x\t", value & 0xFFU);
    if ((value & 0xFFU) < 0x10)
        print_color(WHITE, BLACK, "82489DX discrete APIC\n");
    else if ((value & 0xFFU) < 0x15)
        print_color(WHITE, BLACK, "Integrated APIC\n");
    else
        print_color(WHITE, BLACK, "Reserved\n");
    // Max LVT Entry: bit 16-23
    print_color(WHITE, BLACK, "Max LVT Entry: %#02x\n", (value >> 16UL) & 0xFFU);
    // Suppress EOI-broadcasts: bit 24
    // Indicates whether software can inhibit the broadcast of EOI message
    // by setting bit 12 of the Spurious Interrupt Vector Register.
    if (value >> 24UL & 1UL)
        print_color(WHITE, BLACK, "Support EOI-Broadcast suppression\n");
    else
        print_color(RED, BLACK, "Not support EOI-Broadcast suppression\n");

    // Mask all interrupts in LVT
    value = 0x10000;
    // wrmsr(0x82F, value); TODO CMCI, not support in current processor
    wrmsr(0x832, value);
    wrmsr(0x833, value);
    wrmsr(0x834, value);
    wrmsr(0x835, value);
    wrmsr(0x836, value);
    wrmsr(0x837, value);
}

void io_apic_page_table_map() {
    unsigned long *pml4t = NULL, *pdpt = NULL, *pdt = NULL; // base
    unsigned long *pml4e = NULL, *pdpe = NULL, *pde = NULL; // base + offset
    unsigned long *addr = NULL;

    unsigned int phy_addr = io_apic_map.base_phy_addr;
    pml4t = get_CR3();
    pml4e = pml4t + pml4t_off((unsigned long) phy_addr);
    if (*pml4e == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pml4t(pml4e, mk_pml4t(vir_to_phy(addr), PAGE_KERNEL_PML4T));
    }

    pdpt = phy_to_vir(*pml4e & (~0xFFUL));
    pdpe = pdpt + pdpt_off(phy_addr);
    if (*pdpe == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pdpt(pdpe, mk_pdpt(vir_to_phy(addr), PAGE_KERNEL_PDPT));
    }

    pdt = phy_to_vir(*pdpe & (~0xFFUL));
    pde = pdt + pdt_off(phy_addr);
    set_pdt(pde, mk_pdt(phy_addr, PAGE_KERNEL_PDT | PAGE_PWT | PAGE_PCD));

    flush_TLB();
}

void io_apic_rte_write(unsigned char index, unsigned long value) {
    *io_apic_map.index_addr = index;
    io_mfence();
    *io_apic_map.data_addr = value & 0xFFFFFFFF;
    io_mfence();

    value >>= 32UL;
    *io_apic_map.index_addr = index + 1;
    io_mfence();
    *io_apic_map.data_addr = value & 0xFFFFFFFF;
    io_mfence();
}

void io_apic_init() {
    io_apic_map.base_phy_addr = 0xFEC00000;
    io_apic_map.index_addr = (unsigned char *) phy_to_vir(io_apic_map.base_phy_addr);
    io_apic_map.data_addr = (unsigned int *) phy_to_vir(io_apic_map.base_phy_addr + 0x10);

    io_apic_page_table_map();

    // I/O APIC ID Register
    *io_apic_map.index_addr = 0x00;
    io_mfence();
    *io_apic_map.data_addr = 0x0f000000;
    io_mfence();
    print_color(GREEN, BLACK, "I/O APIC ID Register: index=%#04x, ID=%#03x\n", *io_apic_map.index_addr,
                *io_apic_map.data_addr >> 24UL & 0xFUL);

    // I/O APIC Version Register
    *io_apic_map.index_addr = 0x01;
    io_mfence();
    print_color(GREEN, BLACK, "I/O APIC Version Register: index=%#04x, Version=%#04x, Max Redirection Entry=%d\n",
                *io_apic_map.index_addr,
                *io_apic_map.data_addr & 0xFFUL,
                *io_apic_map.data_addr >> 16UL & 0xFFUL);

    // I/O Redirection Table Registers
    // Bit 16 Interrupt Mask
    // Bit 7:0 Interrupt Vector
    for (unsigned int i = 0x10; i < 0x40; i += 2) {
        io_apic_rte_write(i, 0x10020 + ((i - 0x10) >> 1UL));
    }

    // Don't mask keyboard interrupt
    io_apic_rte_write(0x12, 0x21);
}

void apic_init() {
    for (int i = 0; i < 24; i++) {
        set_intr_gate(0x20 + i, 2, interrupt[i]);
    }

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

    // Mask all interrupts of 8259A
    io_out8(0x21, 0xff);
    io_out8(0xa1, 0xff);

    // enable IMCR
    io_out8(0x22, 0x70);
    io_out8(0x23, 0x01);

    local_apic_init();

    io_apic_init();

    memset(IRQ_Table, 0, sizeof(irq_desc_t) * NR_IRQs);

    sti();
}

void handle_IRQ(irq_nr_t nr, regs_t *regs) {
    uint8_t code = io_in8(0x60);
    print_color(YELLOW, BLACK, "IRQ:%#04x key code:%#04x\n", nr, code);
    irq_desc_t *irq = &IRQ_Table[nr - 0x20];
    if (irq->handler != NULL) {
        irq->handler(nr, regs);
    }
    if (irq->ctl != NULL && irq->ctl->ack != NULL) {
        irq->ctl->ack(nr);
    }
    // EOI register
    wrmsr(0x80B, 0);
}