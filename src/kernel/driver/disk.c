//
// Created by 赵明明 on 2020/1/24.
//
#include "disk.h"
#include "../trap/apic.h"
#include "../lib/x86.h"
#include "../lib/stdio.h"
#include "../mm/slab.h"

irq_ctl_t disk_ctl = {
        .enable = io_apic_enable,
        .disable = io_apic_disable,
        .install = io_apic_install,
        .uninstall = io_apic_uninstall,
        .ack = io_apic_edge_ack
};
struct block_buffer_node block_request;
struct block_buffer_node *in_using = NULL;
uint32_t req_cnt;

void destroy_request();

void exec_next_cmd() {
    struct block_buffer_node *node = NULL;
    struct ide_device *dev = NULL;

    node = in_using = block_next(&block_request);
    dev = node->dev;
    list_del(&node->list);
    req_cnt--;

    int ext = node->LBA > 0xFFFFFFFU; // 28 bit PIO or 48 bit PIO

    io_out8(dev->base + REG_DEV_SEL, 0xE0U | (dev->drive & 0x1U) << 4U |
                                     (ext ? 0U : ((node->LBA >> 24U) & 0xFU)));

    io_in8(dev->ctrl + REG_ALT_STATUS); // 400ns delays
    io_in8(dev->ctrl + REG_ALT_STATUS);
    io_in8(dev->ctrl + REG_ALT_STATUS);
    io_in8(dev->ctrl + REG_ALT_STATUS);

    uint8_t status = io_in8(dev->base + REG_STATUS);
    if (status == 0) {
        print_color(RED, BLACK, "Device does not exist\n");
        destroy_request();
        return;
    }
    // wait busy clear and data rdy
    while (status & STATUS_BSY || !(status & STATUS_RDY)) {
        status = io_in8(dev->base + REG_STATUS);
    }

    if (ext) {
        io_out8(dev->base + REG_SEC_CNT, node->sec_cnt >> 8U); // sector count
        io_out8(dev->base + REG_LBA_HIGH, node->LBA >> 40U);
        io_out8(dev->base + REG_LBA_MED, node->LBA >> 32U);
        io_out8(dev->base + REG_LBA_LOW, node->LBA >> 24U);
    }

    io_out8(dev->base + REG_SEC_CNT, node->sec_cnt);
    io_out8(dev->base + REG_LBA_HIGH, (node->LBA >> 16U) & 0xFFU);
    io_out8(dev->base + REG_LBA_MED, (node->LBA >> 8U) & 0xFFU);
    io_out8(dev->base + REG_LBA_LOW, (node->LBA) & 0xFFU);

    // command
    if (node->command == DISK_READ) {
        io_out8(dev->base + REG_COMMAND, ext ? ATA_CMD_READ_EXT : ATA_CMD_READ);
    } else if (node->command == DISK_WRITE) {
        io_out8(dev->base + REG_COMMAND, ext ? ATA_CMD_WRITE_EXT : ATA_CMD_WRITE);
        port_outsw(node->dev->base + REG_DATA, node->buffer, 256 * node->sec_cnt);
    } else {
        io_out8(dev->base + REG_COMMAND, ATA_CMD_IDENTIFY);
    }
}

void destroy_request() {
    kfree(in_using);
    in_using = NULL;
    if (req_cnt)
        exec_next_cmd();
}

void read_handler(irq_nr_t nr) {
    struct block_buffer_node *node = in_using;
    int8_t status = io_in8(node->dev->base + REG_STATUS);
    print_color(YELLOW, BLACK, "status=%#04x\n", status);
    port_insw(node->dev->base + REG_DATA, node->buffer, 256 * node->sec_cnt);
    destroy_request();
}

void write_handler(irq_nr_t nr) {
    int8_t status = io_in8(in_using->dev->base + REG_STATUS);
    print_color(YELLOW, BLACK, "status=%#04x\n", status);
    destroy_request();
}

void identify_handler(irq_nr_t nr) {
    int8_t status = io_in8(in_using->dev->base + REG_STATUS);
    print_color(YELLOW, BLACK, "status=%#04x\n", status);
    port_insw(in_using->dev->base + REG_DATA, in_using->buffer, 256);
    destroy_request();
}

void other_handler(irq_nr_t nr) {
    destroy_request();
}

void disk_handler(irq_nr_t irq_nr, regs_t *regs) {
    in_using->handler(irq_nr);
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

    entry.intr_vector = MASTER_DISK_IRQ_NR;
    register_irq(MASTER_DISK_IRQ_NR, "master disk", &disk_ctl, &entry, &disk_handler);

    list_init(&block_request.list);

    // enable
    io_out8(PORT_DISK0_CTRL, 0);
    io_out8(PORT_DISK1_CTRL, 0);
}

struct block_buffer_node *
make_request(struct ide_device *dev, uint8_t command, uint64_t lba, uint64_t count, void *buffer) {
    struct block_buffer_node *node = kmalloc(sizeof(struct block_buffer_node));
    list_init(&node->list);
    switch (command) {
        case DISK_READ:
            node->command = DISK_READ;
            node->handler = read_handler;
            break;
        case DISK_WRITE:
            node->command = DISK_WRITE;
            node->handler = write_handler;
            break;
        case DISK_IDENTIFY:
            node->command = DISK_IDENTIFY;
            node->handler = identify_handler;
            break;
        default:
            node->command = 0;
            node->handler = other_handler;
            break;
    }
    node->dev = dev;
    node->LBA = lba;
    node->sec_cnt = count;
    node->buffer = buffer;
    return node;
}

void submit(struct block_buffer_node *node) {
    list_add_to_before(&block_request.list, &node->list);
    req_cnt++;
    if (in_using == NULL) {
        exec_next_cmd();
    }
}