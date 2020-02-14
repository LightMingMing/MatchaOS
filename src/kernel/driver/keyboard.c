//
// Created by 赵明明 on 2020/1/19.
//

#include "keyboard.h"
#include "../lib/stdio.h"
#include "../mm/slab.h"
#include "../trap/intr.h"
#include "../trap/apic.h"

kb_buf_t *kb_buf = NULL;
uint8_t L_Shift = 0;
uint8_t R_Shift = 0;
uint8_t L_ALT = 0;
uint8_t R_ALT = 0;
uint8_t L_CTRL = 0;
uint8_t R_CTRL = 0;

irq_ctl_t kb_ctl = {
        .enable = io_apic_enable,
        .disable = io_apic_disable,
        .install = io_apic_install,
        .uninstall = io_apic_uninstall,
        .ack = io_apic_edge_ack
};

void kb_handler(irq_nr_t nr, regs_t *regs) {
    uint8_t code = io_in8(PORT_KB_DATA);
//    print_color(YELLOW, BLACK, "IRQ:%#04x, keycode: %#04x\n", nr, code);
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

uint8_t get_scan_code() {
    while (!kb_buf->count) {
        nop();
    }
    uint8_t code = kb_buf->buf[kb_buf->ri++];
    kb_buf->count--;
    if (kb_buf->ri == KB_BUF_SIZE) kb_buf->ri = 0;
    return code;
}

void analysis_keycode() {
    uint8_t code = get_scan_code();
    if (code == 0xE1) {
        // PAUSE Make
        for (int i = 1; i < 6; i++) {
            if (get_scan_code() != pause_keycode[i]) {
                break;
            }
        }
    } else if (code == 0xE0) {
        code = get_scan_code();
        if (code == 0x1D) {
            // R CTRL Make
            R_CTRL = 1;
        } else if (code == 0x9D) {
            // R CTRL Break
            R_CTRL = 0;
        } else if (code == 0x38) {
            // R ALT Make
            R_ALT = 1;
        } else if (code == 0xB8) {
            // R ALT Break
            R_ALT = 0;
        } else if (code == 0x2A) {
            if (get_scan_code() == 0xE0) {
                if (get_scan_code() == 0x37) {
                    // PRINT SCREEN Make
                }
            }
        } else if (code == 0xB7) {
            if (get_scan_code() == 0xE0) {
                if (get_scan_code() == 0xAA) {
                    // PRINT SCREEN Break
                }
            }
        }
    } else if (code == 0x2A) {
        // LEFT SHIFT Make
        L_Shift = 1;
    } else if (code == 0xAA) {
        // LEFT SHIFT Break
        L_Shift = 0;
    } else if (code == 0x36) {
        // RIGHT SHIFT Make
        R_Shift = 1;
    } else if (code == 0xB6) {
        // RIGHT SHIFT Break
        R_Shift = 0;
    } else if (code == 0x1D) {
        // L CTRL Make
        L_CTRL = 1;
    } else if (code == 0x9D) {
        // L CTRL Break
        L_CTRL = 0;
    } else if (code == 0x38) {
        // L ALT Make
        L_ALT = 1;
    } else if (code == 0xB8) {
        // L ALT Break
        L_ALT = 0;
    } else {
        uint8_t value;
        if (code < 0x80) {
            if (L_Shift | R_Shift) {
                value = keycode_map[(code << 1U) + 1];
            } else {
                value = keycode_map[code << 1U];
            }
            print_color(BLACK, WHITE, "%c", value);
        }
    }
}