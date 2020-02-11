//
// Created by 赵明明 on 2020/2/7.
//

#include "HPET.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../trap/apic.h"
#include "../trap/softirq.h"
#include "../mm/memory.h"

// 1s = 10E15fs (femptoseconds)
#define FS_PER_S 1000000000000000

irq_ctl_t HPET_ctl;

void HPET_handler(irq_nr_t nr, regs_t *regs);

void do_timer(void *data);

uint64_t read_reg(uint32_t offset) {
    uint64_t value = *(unsigned long *) phy_to_vir(HPET_BASE + offset);
    io_mfence();
    return value;
}

void write_reg(uint32_t offset, uint64_t value) {
    *(unsigned long *) phy_to_vir(HPET_BASE + offset) = value;
    io_mfence();
}

void HPET_init() {
    uint64_t tick_period;

    tick_period = read_reg(GEN_CAP_AND_ID_REG) >> 32UL;

    // GEN_CONF
    write_reg(GEN_CONF_REG, 3);

    // TIMER0
    write_reg(TIMER0_CONF_REG, 0x4c);

    // TIMER0 COMP
    write_reg(TIMER0_COMP_REG, FS_PER_S / tick_period);

    // MAIN COUNTER
    write_reg(MAIN_COUNTER_REG, 0);

    struct IO_APIC_RTE entry;
    entry.intr_vector = HPET_IRQ_NR;
    entry.delivery_mode = DELIVERY_MODE_FIXED;
    entry.delivery_status = DELIVERY_STATUS_IDLE;
    entry.trigger_mode = TRIGGER_MODE_EDGE;
    entry.intr_mask = INTR_MASKED;
    entry.pin_polarity = PIN_POLARITY_HIGH;
    entry.irr = IRR_RESET;
    entry.reserved = 0;
    entry.dest_mode = DEST_MODE_PHYSICAL;
    entry.dest_field.physical.reserved1 = 0;
    entry.dest_field.physical.phy_dest = 0;
    entry.dest_field.physical.reserved2 = 0;

    register_soft_irq(0, &do_timer, NULL);
    register_irq(HPET_IRQ_NR, "HPET", &HPET_ctl, &entry, &HPET_handler);
}

irq_ctl_t HPET_ctl = {
        .enable = io_apic_enable,
        .disable = io_apic_disable,
        .install = io_apic_install,
        .uninstall = io_apic_uninstall,
        .ack = io_apic_edge_ack
};

unsigned long jiffies = 0;

void HPET_handler(irq_nr_t nr, regs_t *regs) {
    jiffies++;
    set_soft_irq_status(TIMER_IRQ);
}

void do_timer(void *data) {
    print_color(YELLOW, BLACK, "(%d) ", jiffies);
}