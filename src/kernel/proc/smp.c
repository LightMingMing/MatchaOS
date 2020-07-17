//
// Created by 赵明明 on 2020/2/2.
//

#include "cpu.h"
#include "proc.h"
#include "smp.h"
#include "spinlock.h"
#include "../mm/slab.h"
#include "../sched/sched.h"

uint8_t global_i;
spinlock_t smp_lock;

void IPI_0xC8(uint8_t nr, regs_t *regs) {
    jiffies_down();
}

void send_IPI(struct ICR_Entry *entry) {
    unsigned long value = *(unsigned long *) entry;
    if (x2APIC_supported()) {
        wrmsr(ICR_MSR, value);
    } else {
        wrmmio(ICR_HIGH, value >> 32UL);
        wrmmio(ICR_LOW, value & ((1UL << 32U) - 1U));
    }
}

void smp_init() {
    uint32_t start_up_code = 0x20000; // < 1M
    memcpy(&_APU_boot_start, (void *) phy_to_vir(start_up_code), &_APU_boot_end - &_APU_boot_start);

    struct ICR_Entry entry;
    entry.IPI_vector = 0;
    entry.delivery_mode = DELIVERY_MODE_INIT;
    entry.dest_mode = DEST_MODE_PHYSICAL;
    entry.delivery_status = DELIVERY_STATUS_IDLE;
    entry.reserved1 = 0;
    entry.level = LEVEL_ASSERT;
    entry.trigger_mode = TRIGGER_MODE_EDGE;
    entry.reserved2 = 0;
    entry.dest_shorthand = SHORTHAND_ALL_EX_SELF;
    entry.reserved3 = 0;

    send_IPI(&entry);

    spin_init(&smp_lock);

    unsigned long ist = (unsigned long) (kmalloc(PROC_STACK_SIZE) + PROC_STACK_SIZE);
    setup_TSS((unsigned int *) &init_tss[0], _stack_start, _stack_start, _stack_start,
              ist, ist, ist, ist, ist, ist, ist);

    while (global_i < NR_CPUs - 1) {
        spin_lock(&smp_lock);
        global_i++;

        _stack_start = (unsigned long) (kmalloc(PROC_STACK_SIZE) + PROC_STACK_SIZE); //  head.S
        ist = (unsigned long) (kmalloc(PROC_STACK_SIZE) + PROC_STACK_SIZE);

        set_tss_desc(10 + global_i * 2, &init_tss[global_i]);
        setup_TSS((unsigned int *) &init_tss[global_i], _stack_start, _stack_start, _stack_start,
                  ist, ist, ist, ist, ist, ist, ist);

        entry.IPI_vector = start_up_code >> 12U;
        entry.delivery_mode = DELIVERY_MODE_START_UP;
        entry.dest_shorthand = SHORTHAND_NO;
        entry.dest_field = global_i;

        // Send SIPI twice
        send_IPI(&entry); // Start-up IPI (SIPI)
        send_IPI(&entry); // Start-up IPI (SIPI)
    }

    for (int i = 0; i < 10; i++) {
        set_intr_gate(0xC8 + i, 0, IPI[i]);
    }
    register_IPI(0xC8, "Sched IPI", &IPI_0xC8);
}

void Start_SMP() {
    print_color(BLACK, WHITE, "AP started... [%d]\n", rdmmio(APIC_ID_REG) >> 24UL);

    uint8_t x2APIC = x2APIC_supported();
    // IA32_APIC_BASE MSR (MSR address 1BH)
    unsigned long value = get_IA32_APIC_BASE();
    // APIC Global Enable flag: bit 11
    wrmsr(IA32_APIC_BASE_MSR, value | (1UL << APIC_GLOBAL_EN_BIT) | (x2APIC ? (1UL << x2APIC_EN_BIT) : 0));

    if (!x2APIC) {
        // APIC version
        value = rdmmio(APIC_VERSION_REG);
    } else {
        // x2APIC version
        value = rdmsr(x2APIC_VERSION_MSR);
    }
    // Suppress EOI-broadcasts: bit 24
    // Indicates whether software can inhibit the broadcast of EOI message
    // by setting bit 12 of the Spurious Interrupt Vector Register.
    uint8_t EOI_suppress = value >> 24UL & 0x1U; // whether or not support EOI-broadcast suppression

    if (x2APIC) {
        // Mask all interrupts in LVT
        value = 0x10000;
        // wrmsr(LVT_CMCI_MSR, value); //TODO CMCI, not support in current processor
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
        // bit 8:  APIC Software Enable/Disable 0:Disabled, 1:Enabled
        // bit 12: EOI-Broadcast Suppression 0:Disabled, 1:Enabled
        value = rdmsr(SVR_MSR) | (1UL << 8UL) | (EOI_suppress ? (1UL << 12UL) : 0);
        wrmsr(SVR_MSR, value);
    } else {
        wrmmio(SVR, rdmmio(SVR) | (1UL << 8UL) | (EOI_suppress ? (1UL << 12UL) : 0));
    }

    get_current()->cpu_id = global_i;
    proc_init();

    spin_unlock(&smp_lock);

    load_TR(10U + global_i * 2);
    sti();

    while (1) {
        hlt();
    }
}