//
// Created by 赵明明 on 2019/11/20.
//

#ifndef _X86_H
#define _X86_H

#include "defs.h"

#define nop() __asm__ __volatile__ ("nop")
#define cli() __asm__ __volatile__ ("cli":::"memory")
#define sti() __asm__ __volatile__ ("sti":::"memory")
// 串行化处理器的执行指令流, 保证之前的读写操作全部完成
#define io_mfence() __asm__ __volatile__ ("mfence":::"memory")

#define do_div(num, base) ({ \
int __res; \
__asm__ __volatile__("divq %%rcx":"=a" (num),"=d" (__res):"0" (num),"1" (0),"c" (base)); \
__res; })

#define hlt() __asm__ __volatile__("hlt":: :)
#define pause() __asm__ __volatile__("pause":: :)

#define container_of(ptr, type, member)   \
({  \
   typeof(((type *)0) -> member) *p = (ptr);\
   (type*)((unsigned long)p - (unsigned long)&(((type *)0)->member));\
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

static inline void *memcpy(void *from, void *to, uint64_t size) {
    int d0, d1, d2;
    __asm__ __volatile("cld                 \n\t"
                       "rep                 \n\t"
                       "movsq               \n\t"
                       "testb $4, %b3       \n\t"
                       "je 1f               \n\t"
                       "movsl               \n\t"
                       "1: testb $2, %b3    \n\t"
                       "je 2f               \n\t"
                       "movsw               \n\t"
                       "2: testb $1, %b3    \n\t"
                       "je 3f               \n\t"
                       "movsb               \n\t"
                       "3:                  \n\t"
    :"=&c"(d0), "=&D"(d1), "=&S"(d2)
    :"q"(size), "0"(size / 8), "1"(to), "2"(from)
    :"memory");
    return to;
}

static inline void io_out8(unsigned short port, unsigned char value) {
    __asm__ __volatile__("outb  %0, %%dx; mfence"::"a"(value), "d"(port):"memory");
}

static inline unsigned char io_in8(unsigned short port) {
    unsigned char tmp;
    __asm__ __volatile__("inb  %%dx, %0; mfence":"=a"(tmp): "d"(port):"memory");
    return tmp;
}

static inline unsigned long rdmsr(unsigned long addr) {
    unsigned long ret;
    __asm__ __volatile__("rdmsr\n\t"
                         "shlq $32, %%rdx\n\t"
                         "addq %%rdx, %0 \n\t"
    :"=a"(ret):"c"(addr):"memory");
    return ret;
}

static inline void wrmsr(unsigned long addr, unsigned long value) {
    __asm__ __volatile__("wrmsr"::"d"(value >> 32U), "a"(value & 0xffffffff), "c"(addr):"memory");
}

#endif //_X86_H
