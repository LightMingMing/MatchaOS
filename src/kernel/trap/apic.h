//
// Created by 赵明明 on 2020/1/8.
// APIC: Advanced Programmable Interrupt Controller
//

#ifndef _APIC_H
#define _APIC_H

#include "../lib/defs.h"
#include "intr.h"

// Memory Mapped Registers For Accessing IO APIC Registers
struct memory_map_regs {
    unsigned int base_phy_addr;
    unsigned char *index_addr; // IO Register Select (index)
    unsigned int *data_addr; // IO Window (data)
} io_apic_map;

#define DELIVERY_MODE_FIXED             0x0U
#define DELIVERY_MODE_LOWEST_PRIORITY   0x1U
#define DELIVERY_MODE_SMI               0x2U
#define DELIVERY_MODE_NMI               0x4U
#define DELIVERY_MODE_INIT              0x5U
#define DELIVERY_MODE_EXT_INT           0x5U

#define DEST_MODE_PHYSICAL  0x0U
#define DEST_MODE_LOGICAL   0x1U

#define DELIVERY_STATUS_IDLE            0x0U
#define DELIVERY_STATUS_SEND_PENDING    0x1U

#define PIN_POLARITY_HIGH   0x0U
#define PIN_POLARITY_LOW    0x1U

#define IRR_RESET   0x0U
#define IRR_SET     0x1U

#define TRIGGER_MODE_EDGE   0x0U
#define TRIGGER_MODE_LEVEL  0x1U

#define INTR_NOT_MASKED 0x0U
#define INTR_MASKED     0x1U

// I/O APIC Redirection Table Entry
struct IO_APIC_RTE {
    uint32_t intr_vector :8, // From 10H to FEH, bit 7:0
            delivery_mode :3,       // bit 10:8
            dest_mode :1,           // bit 11
            delivery_status :1,     // bit 12
            pin_polarity :1,        // bit 13
            irr :1,                 // bit 14
            trigger_mode :1,        // bit 15
            intr_mask :1,           // bit 16
            reserved:15;            // bit 31:17
    union {
        struct {
            uint32_t reserved1 :24, // bit 55:32
                    phy_dest :4,    // bit 59:56
                    reserved2 :4;   // bit 63:60
        } physical;

        struct {
            uint32_t reserved1 :24, // bit 55:32
                    logical_dest :8;// bit 64:56
        } logical;
    } dest_field;
}__attribute__((packed));

void io_apic_enable(irq_nr_t nr);

void io_apic_disable(irq_nr_t nr);

void io_apic_install(irq_nr_t nr, void *arg);

void io_apic_uninstall(irq_nr_t nr);

void io_apic_level_ack(irq_nr_t nr);

void io_apic_edge_ack(irq_nr_t nr);

void apic_init();

#endif //_APIC_H
