//
// Created by 赵明明 on 2020/1/24.
//
#include "disk.h"
#include "../trap/apic.h"
#include "../lib/x86.h"
#include "../lib/stdio.h"

irq_ctl_t disk_ctl = {
        .enable = io_apic_enable,
        .disable = io_apic_disable,
        .install = io_apic_install,
        .uninstall = io_apic_uninstall,
        .ack = io_apic_edge_ack
};

void disk_handler(irq_nr_t irq_nr, regs_t *regs) {
}

void disk_init() {
    struct IO_APIC_RTE entry;
    entry.intr_vector = SLAVE_DISK_IRQ_NR;
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

    register_irq(SLAVE_DISK_IRQ_NR, "slave disk", &disk_ctl, &entry, &disk_handler);

    // enable
    io_out8(PORT_DISK1_STATUS_AND_CTL, 0);

    io_out8(PORT_DISK1_ERROR, 0);
    io_out8(PORT_DISK1_SEC_CNT, 0);
    io_out8(PORT_DISK1_SEC_NUM, 0);
    io_out8(PORT_DISK1_CYL_LOW, 0);
    io_out8(PORT_DISK1_CYL_HIGH, 0);
    io_out8(PORT_DISK1_DRV_AND_HEAD, 0xE0);
    if (io_in8(PORT_DISK1_STATUS_AND_CMD) == 0) {
        print_color(RED, BLACK, "Drive does not exist\n");
        return;
    }
}