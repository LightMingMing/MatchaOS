//
// Created by 赵明明 on 2020/1/23.
//

#include "mouse.h"
#include "../lib/x86.h"
#include "../lib/stdio.h"
#include "../mm/slab.h"
#include "../trap/apic.h"

kb_buf_t *mouse_buf = NULL;

irq_ctl_t mouse_ctl = {
        .enable = io_apic_enable,
        .disable = io_apic_disable,
        .install = io_apic_install,
        .uninstall = io_apic_uninstall,
        .ack = io_apic_edge_ack
};

void mouse_handler(irq_nr_t nr, regs_t *regs) {
    uint8_t code = io_in8(PORT_KB_DATA);
    mouse_buf->buf[mouse_buf->wi++] = code;
    mouse_buf->count++;
    if (mouse_buf->wi == KB_BUF_SIZE) mouse_buf->wi = 0;
}

void mouse_init() {
    struct IO_APIC_RTE entry;

    mouse_buf = kmalloc(sizeof(kb_buf_t));
    memset(mouse_buf->buf, 0, KB_BUF_SIZE);
    mouse_buf->wi = 0;
    mouse_buf->ri = 0;
    mouse_buf->count = 0;

    entry.intr_vector = MOUSE_IRQ_NR;
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

    wait_KB_write();
    io_out8(PORT_KBC_CMD, KBC_CMD_EN_MOUSE_INTERFACE);

    wait_KB_write();
    io_out8(PORT_KBC_CMD, KBC_CMD_SENT_TO_MOUSE);
    wait_KB_write();
    io_out8(PORT_KBC_CMD_PARAM, MOUSE_CMD_EN_DATA_REPORTING);

    uint8_t ack = io_in8(PORT_KB_DATA);
    print_color(YELLOW, BLACK, "ack: %#04x\n", ack);

    wait_KB_write();
    io_out8(PORT_KBC_CMD, KBC_CMD_WRITE);
    wait_KB_write();
    io_out8(PORT_KBC_CMD_PARAM, KBC_INIT_MODE);

    register_irq(MOUSE_IRQ_NR, "ps/2 mouse", &mouse_ctl, &entry, &mouse_handler);
}

uint8_t get_mousecode() {
    while (!mouse_buf->count) {
        nop();
    }
    uint8_t code = mouse_buf->buf[mouse_buf->ri++];
    mouse_buf->count--;
    if (mouse_buf->ri == KB_BUF_SIZE) mouse_buf->ri = 0;
    return code;
}

void analysis_mousecode() {
    struct mouse_packet packet;
    packet.state = get_mousecode();
    packet.xm = get_mousecode();
    packet.ym = get_mousecode();
    print_color(BLACK, WHITE, "%04x, (%d, %d)\n", packet.state, packet.xm, packet.ym);
}