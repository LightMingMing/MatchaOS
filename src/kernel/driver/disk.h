//
// Created by 赵明明 on 2020/1/24.
// https://wiki.osdev.org/ATA_PIO_Mode#Detecting_Controller_IO_Ports
//

#ifndef _DISK_H
#define _DISK_H

#define MASTER_DISK_IRQ_NR  0x2D
#define SLAVE_DISK_IRQ_NR   0x2F

/**
CHS mode: Cylinder,Head, Sector
LBA mode: Logical Block Address

I/O PORT: 0x1F6 0x176
bit7: always 1
bit6: mode. 0 CHS, 1 LBA
bit5: always 1
bit4: drive. 0 master, 1 slave
bit3-0: head number or LBA27_24
*/

/**
 * Primary/Master DISK: 0x1F0 ~ 0x1F7
 */
#define PORT_DISK0_DATA             0x1F0U
#define PORT_DISK0_ERROR            0x1F1U
#define PORT_DISK0_SEC_CNT          0x1F2U
#define PORT_DISK0_SEC_NUM          0x1F3U
#define PORT_DISK0_CYL_LOW          0x1F4U
#define PORT_DISK0_CYL_HIGH         0x1F5U
#define PORT_DISK0_DRV_AND_HEAD     0x1F6U
#define PORT_DISK0_STATUS_AND_CMD   0x1F7U

#define PORT_DISK0_STATUS_AND_CTL   0x3F6

/**
 * Secondary/Slave DISK: 0x170 ~ 0x177
 */
#define PORT_DISK1_DATA             0x170U
#define PORT_DISK1_ERROR            0x171U
#define PORT_DISK1_SEC_CNT          0x172U
#define PORT_DISK1_SEC_NUM          0x173U
#define PORT_DISK1_CYL_LOW          0x174U
#define PORT_DISK1_CYL_HIGH         0x175U
#define PORT_DISK1_DRV_AND_HEAD     0x176U
#define PORT_DISK1_STATUS_AND_CMD   0x177U

#define PORT_DISK1_STATUS_AND_CTL   0x376

/**
 * COMMAND
 */
#define CMD_DISK_IDENTIFY   0xEC
#define CMD_READ_SECTORS    0x20
#define CMD_WRITE_SECTORS   0x30

/**
 * Status
 */
#define DISK_STATUS_ERR     (1U<<0U)
#define DISK_STATUS_IDX     (1U<<1U)
#define DISK_STATUS_CORR    (1U<<2U)
#define DISK_STATUS_DRQ     (1U<<3U)
#define DISK_STATUS_SRV     (1U<<4U)
#define DISK_STATUS_DF      (1U<<5U)
#define DISK_STATUS_RDY     (1U<<6U)
#define DISK_STATUS_BSY     (1U<<7U)

void disk_init();

#endif //_DISK_H
