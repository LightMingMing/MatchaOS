//
// Created by 赵明明 on 2020/1/24.
// https://wiki.osdev.org/ATA_PIO_Mode#Detecting_Controller_IO_Ports
//

#ifndef _DISK_H
#define _DISK_H

#include "../lib/defs.h"
#include "../lib/list.h"
#include "../lib/x86.h"
#include "../trap/intr.h"

#define MASTER_DISK_IRQ_NR  0x2D
#define SLAVE_DISK_IRQ_NR   0x2F

/**
I/O PORT: 0x1F6 0x176
*/

/**
 * Primary/Master DISK: 0x1F0 ~ 0x1F7
 */
#define PORT_DISK0_BASE         0x1F0U
#define PORT_DISK0_CTRL         0x3F6U
/**
 * Secondary/Slave DISK: 0x170 ~ 0x177
 */
#define PORT_DISK1_BASE         0x170U
#define PORT_DISK1_CTRL         0x376U

/**
 * Register offset
 */
#define REG_DATA        0
#define REG_ERROR       1
#define REG_SEC_CNT     2
#define REG_LBA_LOW     3
#define REG_LBA_MED     4
#define REG_LBA_HIGH    5
/*
bit 7: always 1
bit 6: mode. 0 CHS (Cylinder,Head, Sector), 1 LBA (Logical Block Address)
bit 5: always 1
bit 4: drive. 0 master, 1 slave
bit3-0: head number or LBA27_24
*/
#define REG_DEV_SEL     6
#define REG_STATUS      7
#define REG_COMMAND     7

#define REG_ALT_STATUS  0
#define REG_DRV_ADDR    1

/**
 * Command value of Command Register
 */
#define ATA_CMD_IDENTIFY    0xEC
#define ATA_CMD_READ        0x20 // 28 Bit LBA
#define ATA_CMD_READ_EXT    0x24 // 48 Bit LBA
#define ATA_CMD_WRITE       0x30 // 28 Bit LBA
#define ATA_CMD_WRITE_EXT   0x34 // 48 Bit LBA
#define ATA_CMD_FLUSH       0xE7
#define ATA_CMD_FLUSH_EXT   0xEA

/**
 * Status value of Status Register
 */
#define STATUS_ERR     (1U<<0U)
#define STATUS_IDX     (1U<<1U)
#define STATUS_CORR    (1U<<2U)
#define STATUS_DRQ     (1U<<3U)
#define STATUS_SRV     (1U<<4U)
#define STATUS_DF      (1U<<5U)
#define STATUS_RDY     (1U<<6U)
#define STATUS_BSY     (1U<<7U)

#define DISK_READ       0
#define DISK_WRITE      1
#define DISK_IDENTIFY   2

struct ide_device {
    uint16_t base;
    uint16_t ctrl; // alt status and control
    uint8_t drive; // 0 master drive, 1 slave drive
};

struct block_buffer_node {
    list_t list;
    void *buffer;
    uint8_t command;
    uint16_t sec_cnt; // Sector Count
    uint64_t LBA; // Logical Block Address
    struct ide_device *dev;

    void (*handler)(irq_nr_t nr);
};

static inline struct block_buffer_node *block_next(struct block_buffer_node *block) {
    return container_of(list_next(&block->list),
                        struct block_buffer_node, list);
}

void disk_init();

struct block_buffer_node *
make_request(struct ide_device *dev, uint8_t command, uint64_t lba, uint64_t count, void *buffer);

void submit(struct block_buffer_node *node);

#endif //_DISK_H
