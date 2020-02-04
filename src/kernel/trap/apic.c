//
// Created by 赵明明 on 2020/1/8.
//
#include "apic.h"
#include "intr.h"
#include "gate.h"
#include "../proc/cpu.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../mm/memory.h"
#include "../mm/slab.h"

void local_apic_page_table_map() {
    unsigned long *pml4t = NULL, *pdpt = NULL, *pdt = NULL; // base
    unsigned long *pml4e = NULL, *pdpe = NULL, *pde = NULL; // base + offset
    unsigned long *addr = NULL;

    pml4t = get_CR3();
    pml4e = pml4t + pml4t_off(APIC_BASE_ADDR);
    if (*pml4e == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pml4t(pml4e, mk_pml4t(vir_to_phy(addr), PAGE_KERNEL_PML4T));
    }

    pdpt = phy_to_vir(*pml4e & (~0xFFUL));
    pdpe = pdpt + pdpt_off(APIC_BASE_ADDR);
    if (*pdpe == 0) {
        addr = kmalloc(PAGE_SIZE_4K);
        set_pdpt(pdpe, mk_pdpt(vir_to_phy(addr), PAGE_KERNEL_PDPT));
    }

    pdt = phy_to_vir(*pdpe & (~0xFFUL));
    pde = pdt + pdt_off(APIC_BASE_ADDR);
    set_pdt(pde, mk_pdt(APIC_BASE_ADDR, PAGE_KERNEL_PDT | PAGE_PWT | PAGE_PCD));

    flush_TLB();
}

void local_apic_init() {
    uint8_t xAPIC, x2APIC;
    uint8_t EOI_suppress; // whether or not support EOI-broadcast suppression
    uint8_t entry_cnt; // Max LTV Entry + 1
    uint8_t version;

    xAPIC = xAPIC_supported();
    x2APIC = x2APIC_supported();

    if (xAPIC)
        print_color(WHITE, BLACK, "Processor support APIC&xAPIC\t");
    else
        print_color(RED, BLACK, "Processor not support APIC&xAPIC\t");

    if (x2APIC)
        print_color(WHITE, BLACK, "Processor support x2APIC\n");
    else
        print_color(RED, BLACK, "Processor not support x2APIC\n");

    // IA32_APIC_BASE MSR (MSR address 1BH)
    unsigned long value = get_IA32_APIC_BASE();
    // APIC Global Enable flag: bit 11
    value = value | (1UL << APIC_GLOBAL_EN_BIT) | (x2APIC ? (1UL << x2APIC_EN_BIT) : 0);
    wrmsr(IA32_APIC_BASE_MSR, value);
    value = get_IA32_APIC_BASE();
    print_color(WHITE, BLACK, "IA32_APIC_BASE MSR: %#018lx, APIC BASE: %#010lx\n", value, value & 0xFFFFF000);

    if (value >> APIC_GLOBAL_EN_BIT & 1UL)
        print_color(WHITE, BLACK, "Enable xAPIC\t");
    else
        print_color(RED, BLACK, "Disable xAPIC\t");

    if (value >> x2APIC_EN_BIT & 1UL)
        print_color(WHITE, BLACK, "Enable x2APIC\n");
    else
        print_color(RED, BLACK, "Disable x2APIC\n");

    local_apic_page_table_map();
    uint8_t xAPIC_ID = (uint8_t) (rdmmio(APIC_ID_REG) >> 24U);
    print_color(WHITE, BLACK, "xAPIC ID: %#04x\n", xAPIC_ID);

    if (x2APIC) {
        // x2APIC ID
        value = rdmsr(x2APIC_ID_MSR);
        print_color(WHITE, BLACK, "x2APIC ID: %#010x\n", value);
    }

    if (!x2APIC) {
        // APIC version
        value = rdmmio(APIC_VERSION_REG);
    } else {
        // x2APIC version
        value = rdmsr(x2APIC_VERSION_MSR);
    }
    // Version: bit 0-7
    version = value & 0xFFU;
    // Max LVT Entry: bit 16-23
    entry_cnt = (value >> 16U & 0xFFU) + 1;
    // Suppress EOI-broadcasts: bit 24
    // Indicates whether software can inhibit the broadcast of EOI message
    // by setting bit 12 of the Spurious Interrupt Vector Register.
    EOI_suppress = value >> 24UL & 0x1U;

    print_color(WHITE, BLACK, "Version: %#010lx\n", value);
    print_color(WHITE, BLACK, x2APIC ? "x2APIC Version: %#04x\t" : "xAPIC Version: %#04x\t", version);
    if (version < 0x10)
        print_color(WHITE, BLACK, "82489DX discrete APIC\n");
    else if (version < 0x15)
        print_color(WHITE, BLACK, "Integrated APIC\n");
    else
        print_color(WHITE, BLACK, "Reserved\n");

    print_color(WHITE, BLACK, "LVT Entry Count: %#02x\n", entry_cnt);
    if (EOI_suppress)
        print_color(WHITE, BLACK, "Support EOI-Broadcast suppression\n");
    else
        print_color(RED, BLACK, "Not support EOI-Broadcast suppression\n");

    if (x2APIC) {
        // Mask all interrupts in LVT
        value = 0x10000;
//         wrmsr(LVT_CMCI_MSR, value); //TODO CMCI, not support in current processor
        wrmsr(LVT_TIMER_MSR, value);
        wrmsr(LVT_TS_MSR, value);
        wrmsr(LVT_PM_MSR, value);
        wrmsr(LVT_LINT0_MSR, value);
        wrmsr(LVT_LINT1_MSR, value);
        wrmsr(LVT_ERROR_MSR, value);
    } else {
        // Mask all interrupts in LVT
        value = 0x10000;
        wrmmio(LVT_CMCI_REG, value);
        wrmmio(LVT_TIMER_ERG, value);
        wrmmio(LVT_TS_REG, value);
        wrmmio(LVT_PM_REG, value);
        wrmmio(LVT_LINT0_REG, value);
        wrmmio(LVT_LINT1_REG, value);
        wrmmio(LVT_ERROR_REG, value);
    }

    if (x2APIC) {
        // Spurious interrupt vector register
        value = rdmsr(SVR_MSR);
        // bit 8:  APIC Software Enable/Disable 0:Disabled, 1:Enabled
        // bit 12: EOI-Broadcast Suppression 0:Disabled, 1:Enabled
        value = value | (1UL << 8UL) | (EOI_suppress ? (1UL << 12UL) : 0);
        wrmsr(SVR_MSR, value);
        value = rdmsr(SVR_MSR);
    } else {
        value = rdmmio(SVR);
        value = value | (1UL << 8UL) | (EOI_suppress ? (1UL << 12UL) : 0);
        wrmmio(SVR, value);
        value = rdmmio(SVR);
    }
    if (value >> 8UL & 1UL)
        print_color(WHITE, BLACK, "APIC Software Enabled\t");
    else
        print_color(RED, BLACK, "APIC Software Disabled\t");

    if (value >> 12UL & 1UL)
        print_color(WHITE, BLACK, "EOI-Broadcast Suppression Enabled\n");
    else
        print_color(RED, BLACK, "EOI-Broadcast Suppression Disabled\n");
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

unsigned long io_apic_rte_read(unsigned char index) {
    unsigned long value = 0;

    *io_apic_map.index_addr = index;
    io_mfence();
    value = *io_apic_map.data_addr;
    io_mfence();

    *io_apic_map.index_addr = index + 1;
    io_mfence();
    value |= (unsigned long) *io_apic_map.data_addr << 32U;
    io_mfence();

    return value;
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

static inline unsigned char rte_idx(irq_nr_t nr) {
    return (nr - 0x20) * 2 + 0x10;
}

void io_apic_enable(irq_nr_t nr) {
    io_apic_rte_write(rte_idx(nr), io_apic_rte_read(rte_idx(nr)) & (~0x10000UL));
}

void io_apic_disable(irq_nr_t nr) {
    io_apic_rte_write(rte_idx(nr), io_apic_rte_read(rte_idx(nr)) | 0x10000UL);
}

void io_apic_install(irq_nr_t nr, void *rte) {
    struct IO_APIC_RTE *io_apic_rte = (struct IO_APIC_RTE *) rte;
    io_apic_rte_write(rte_idx(nr), *(unsigned long *) io_apic_rte);
}

void io_apic_uninstall(irq_nr_t nr) {
    io_apic_rte_write(rte_idx(nr), 0x10000);
}

void io_apic_edge_ack(irq_nr_t nr) {
    // EOI register
    if (x2APIC_supported())
        wrmsr(EOI_MSR, 0);
    else
        wrmmio(EOI_REG, 0);
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
    irq_desc_t *irq = &IRQ_Table[nr - 0x20];
    if (irq->handler != NULL) {
        irq->handler(nr, regs);
    }
    if (irq->ctl != NULL && irq->ctl->ack != NULL) {
        irq->ctl->ack(nr);
    }
}