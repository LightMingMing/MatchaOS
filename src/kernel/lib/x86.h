//
// Created by 赵明明 on 2019/11/20.
//

#ifndef _X86_H
#define _X86_H

#include "defs.h"

#define do_div(num, base) ({ \
int __res; \
__asm__ __volatile__("divq %%rcx":"=a" (num),"=d" (__res):"0" (num),"1" (0),"c" (base)); \
__res; })

#define hlt() ({\
__asm__ __volatile__("hlt":: :); \
})

static inline unsigned long *get_CR3() {
    unsigned long *tmp;
    __asm__ __volatile__("movq %%cr3, %0":"=r"(tmp): :"memory");
    return tmp;
}

// flush Translation Look-aside Buffer
static inline void flush_TLB() {
    unsigned long tmp;
    __asm__ __volatile__ ("movq %%cr3, %0   \n\t"
                          "movq %0, %%cr3   \n\t"
    :"=r"(tmp)
    :
    :"memory");
}

static inline void *memset(void *addr, unsigned char c, uint64_t size) {
    int d0, d1;
    unsigned long tmp = c * 0x0101010101010101;
    __asm__ __volatile__("cld               \n\t"
                         "rep               \n\t"
                         "stosq             \n\t"
                         "testb $4, %b3     \n\t"
                         "je 1f             \n\t"
                         "stosl             \n\t"
                         "1: testb $2, %b3  \n\t"
                         "je 2f             \n\t"
                         "stosw             \n\t"
                         "2: testb $1, %b3  \n\t"
                         "je 3f             \n\t"
                         "stosb             \n\t"
                         "3:                \n\t"
    :"=&c"(d0), "=&D"(d1)
    :"a"(tmp), "q"(size), "0"(size / 8), "1"(addr)
    :"memory");
    return addr;
}

#endif //_X86_H
