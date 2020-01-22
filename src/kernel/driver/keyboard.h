//
// Created by 赵明明 on 2020/1/19.
//

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "../lib/defs.h"

#define KB_IRQ_NR   0x21

#define PORT_KB_DATA        (0x60U)
#define PORT_KB_CMD         (0x60U)
#define PORT_KB_CMD_PARAM   (0x60U)
#define PORT_KBC_STATUS     (0x64U)
#define PORT_KBC_CMD        (0x64U)
#define PORT_KBC_CMD_PARAM  (0x60U)

#define KB_CMD_RESET    (0xFFU)
#define KB_CMD_RESEND   (0xFEU)
#define KB_CMD_SETMODE  (0xEDU)

#define KBC_CMD_READ    (0x20U)
#define KBC_CMD_WRITE   (0x60U)

#define STATUS_IN_BUF_FULL  (0x02U)
#define STATUS_OUT_BUF_FULL (0x01U)

#define KBC_INIT_MODE       (0x47U)

#define wait_KB_write() while(io_in8(PORT_KBC_STATUS) & STATUS_IN_BUF_FULL)
#define wait_KB_read()  while(io_in8(PORT_KBC_STATUS) & STATUS_OUT_BUF_FULL)

#define KB_BUF_SIZE 100

typedef struct keyboard_buffer {
    uint32_t wi;
    uint32_t ri;
    uint32_t count;
    unsigned char buf[KB_BUF_SIZE];
} kb_buf_t;

void keyboard_init();

void keyboard_exit();

#endif //_KEYBOARD_H
