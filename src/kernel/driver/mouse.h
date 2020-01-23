//
// Created by 赵明明 on 2020/1/23.
//

#ifndef _MOUSE_H
#define _MOUSE_H

#include "keyboard.h"
#include "../lib/defs.h"

#define MOUSE_IRQ_NR    0x2C

#define KBC_CMD_EN_MOUSE_INTERFACE  (0xA8)
#define KBC_CMD_SENT_TO_MOUSE       (0xD4)

#define MOUSE_CMD_GET_DEVICE_ID     (0xF2)
#define MOUSE_CMD_SAMPLE_RATE       (0xF3)
#define MOUSE_CMD_EN_DATA_REPORTING (0xF4)
#define MOUSE_CMD_RESENT            (0xFE)
#define MOUSE_CMD_RESET             (0xFF)

extern kb_buf_t *mouse_buf;

void mouse_init();

// https://wiki.osdev.org/PS/2_Mouse
struct mouse_packet {
    uint8_t state; // first byte
    int8_t xm; // second byte
    int8_t ym; // third byte
};

void analysis_mousecode();

#endif //_MOUSE_H
