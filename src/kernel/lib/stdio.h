//
// Created by 赵明明 on 2019/11/18.
//

#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include "defs.h"
#include "font.h"
#include "../proc/spinlock.h"

#define FLAG_PAD_LEFT   1u
#define FLAG_PAD_SPACE  2u  // ' '
#define FLAG_PAD_ZERO   4u  // '0'

#define FLAG_SIGN_PLUS  1u  // '+'
#define FLAG_SIGN_MINUS 2u  // '-'
#define FLAG_SIGN_NUM   4u  // '#'
#define FLAG_LOWER   8u
#define FLAG_UPPER  16u

#define WHITE   0x00ffffff
#define BLACK   0x00000000
#define RED     0x00ff0000
#define ORANGE  0x00ff8000
#define YELLOW  0x00ffff00
#define GREEN   0x0000ff00
#define BLUE    0x000000ff
#define INDIGO  0x0000ffff
#define PURPLE  0x008000ff

// Fixed Frame Buffer Physical Address
//unsigned long const FB_phy_address = 0xe0000000; // bochs
unsigned long const FB_phy_address = 0xfd000000; // qemu

// Initial Linear Address Mapped by Page Table
//unsigned long const FB_vir_address = 0xffff800003000000; // bochs
unsigned long const FB_vir_address = 0xffff800004000000; // qemu

// Frame Buffer Length
size_t FB_length = 1440 * 900 * 4; // in bytes

struct position {
    int x_resolution;
    int y_resolution;

    int x_position;
    int y_position;

    int x_pos_max;
    int y_pos_max;

    int x_char_size;
    int y_char_size;

    unsigned int *cur_address;
    unsigned int *FB_address; // Linear Address of Frame Buffer

    spinlock_t lock;
};

struct color {
    unsigned fd; // foreground color
    unsigned bd; // background color
};

struct position pos = {1440, 900, 0, 0, 0, 0, 8, 12, (unsigned int *) FB_vir_address, (unsigned int *) FB_vir_address};

// void print_fmt(void (*put_char)(int, void *, void *), void *put_data, const char *fmt, ...);

void print(const char *fmt, ...);

void println(const char *fmt, ...);

void print_color(unsigned fd, unsigned bd, const char *fmt, ...);


#endif //_STDIO_H
