//
// Created by 赵明明 on 2020/1/19.
//

#include "keyboard.h"
#include "../mm/slab.h"
#include "../trap/intr.h"
#include "../trap/apic.h"
#include "../lib/stdio.h"

kb_buf_t *kb_buf = NULL;

irq_ctl_t kb_ctl = {
        .enable = io_apic_enable,
        .disable = io_apic_disable,
        .install = io_apic_install,
        .uninstall = io_apic_uninstall,
        .ack = io_apic_edge_ack
};

void kb_handler(irq_nr_t nr, regs_t *regs) {
    uint8_t code = io_in8(PORT_KB_DATA);
    print_color(YELLOW, BLACK, "IRQ:%#04x, keycode: %#04x\n", nr, code);
    kb_buf->buf[kb_buf->wi++] = code;
    kb_buf->count++;
    if (kb_buf->wi == KB_BUF_SIZE) kb_buf->wi = 0;
}

void keyboard_init() {
    struct IO_APIC_RTE entry;

    wait_KB_write();
    io_out8(PORT_KBC_CMD, KBC_CMD_WRITE);
    wait_KB_write();
    io_out8(PORT_KBC_CMD_PARAM, KBC_INIT_MODE);

    kb_buf = (kb_buf_t *) kmalloc(sizeof(kb_buf_t));

    memset(kb_buf->buf, 0, KB_BUF_SIZE);
    kb_buf->wi = 0;
    kb_buf->ri = 0;
    kb_buf->count = 0;

    entry.intr_vector = KB_IRQ_NR;
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

    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < 1000; j++) {
            nop();
        }
    }

    register_irq(KB_IRQ_NR, "ps/2 keyboard", &kb_ctl, &entry, &kb_handler);
}

void keyboard_exit() {
    unregister_irq(KB_IRQ_NR);
    kfree(kb_buf);
}