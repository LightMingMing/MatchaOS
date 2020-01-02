//
// Created by 赵明明 on 2020/1/2.
//

#ifndef _BIT_H
#define _BIT_H

static inline void set(unsigned long *addr, unsigned long idx) {
    *(addr + (idx >> 6UL)) |= 1UL << (idx & 63UL);
}

static inline void reset(unsigned long *addr, unsigned long idx) {
    *(addr + (idx >> 6UL)) ^= 1UL << (idx & 63UL);
}

static inline unsigned long get(unsigned long *addr, unsigned long idx) {
    return (*(addr + (idx >> 6UL)) & (1UL << (idx & 63UL)));
}

#endif //_BIT_H
