//
// Created by 赵明明 on 2020/1/19.
//

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "../lib/defs.h"
#include "../lib/x86.h"

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

static inline void wait_KB_write() { while (io_in8(PORT_KBC_STATUS) & STATUS_IN_BUF_FULL) { pause(); }}

static inline void wait_KB_read() { while (io_in8(PORT_KBC_STATUS) & STATUS_OUT_BUF_FULL) { pause(); }}

#define KB_BUF_SIZE 100

typedef struct keyboard_buffer {
    uint32_t wi;
    uint32_t ri;
    uint32_t count;
    unsigned char buf[KB_BUF_SIZE];
} kb_buf_t;

extern kb_buf_t *kb_buf;

void keyboard_init();

void keyboard_exit();

uint8_t pause_keycode[6] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
uint8_t keycode_map[0x100] = {
        0, 0,
        0, 0,       // 0x01 ESC
        '1', '!',   // 0x02
        '2', '@',   // 0x03
        '3', '#',   // 0x04
        '4', '$',   // 0x05
        '5', '%',   // 0x06
        '6', '^',   // 0x07
        '7', '&',   // 0x08
        '8', '*',   // 0x09
        '9', '(',   // 0x0A
        '0', ')',   // 0x0B
        '-', '_',   // 0x0C
        '=', '+',   // 0x0D
        0, 0,       // 0x0E BACKSPACE
        0, 0,       // 0x0F TAB
        'q', 'Q',   // 0x10
        'w', 'W',   // 0x11
        'e', 'E',   // 0x12
        'r', 'R',   // 0x13
        't', 'T',   // 0x14
        'y', 'Y',   // 0x15
        'u', 'U',   // 0x16
        'i', 'I',   // 0x17
        'o', 'O',   // 0x18
        'p', 'P',   // 0x19
        '[', '{',   // 0x1A
        ']', '}',   // 0x1B
        0, 0,       // 0x1C ENTER
        0, 0,       // 0x1D LEFT CTRL
        'a', 'A',   // 0x1E
        's', 'S',   // 0x1F
        'd', 'D',   // 0x20
        'f', 'F',   // 0x21
        'g', 'G',   // 0x22
        'h', 'H',   // 0x23
        'j', 'J',   // 0x24
        'k', 'K',   // 0x25
        'l', 'L',   // 0x26
        ';', ':',   // 0x27
        '\'', '\"', // 0x28
        '`', '~',   // 0x29
        0, 0,       // 0x2A LEFT SHIFT
        '\\', '|',  // 0x2B
        'z', 'Z',   // 0x2C
        'x', 'X',   // 0x2D
        'c', 'C',   // 0x2E
        'v', 'V',   // 0x2F
        'b', 'B',   // 0x30
        'n', 'N',   // 0x31
        'm', 'M',   // 0x32
        ',', '<',   // 0x33
        '.', '>',   // 0x34
        '/', '?',   // 0x35
        0, 0,       // 0x36 RIGHT SHIFT
        '*', '*',   // 0x37
        0, 0,       // 0x38 LEFT ALT
        ' ', ' ',   // 0x39 SPACE
        0, 0,       // 0x3A CAPS
        [0x3B * 2 ... 0x44 * 2 + 1] = 0, // 0x3B~0x44 F1~F10
        0, 0,       // 0x45 NUM
        0, 0,       // 0x46 SCROLL
        '7', 0,     // 0x47
        '8', 0,     // 0x48
        '9', 0,     // 0x49
        '-', 0,     // 0x4A
        '4', 0,     // 0x4B
        '5', 0,     // 0x4C
        '6', 0,     // 0x4D
        '+', 0,     // 0x4E
        '1', 0,     // 0x4F
        '2', 0,     // 0x50
        '3', 0,     // 0x51
        '0', 0,     // 0x52
        '.', 0,     // 0x53
        [0x54 * 2 ... 0xFF] = 0
};

void analysis_keycode();

#endif //_KEYBOARD_H
