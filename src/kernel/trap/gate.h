//
// Created by 赵明明 on 2019/11/26.
//

#ifndef _GATE_H
#define _GATE_H

struct gate_struct {
    unsigned char chs[16];
};

extern struct gate_struct IDT_Table[];
extern unsigned int TSS_Table[26];

#define load_TR(idx)   \
    __asm__ __volatile__ ("ltr  %%ax"::"a" (idx << 3u):"memory");

#define _set_gate(gate_selector_address, attr, ist, segment_offset)     \
    unsigned long _d0, _d1;                                             \
    __asm__ __volatile__    (   "movw   %%dx, %%ax      \n\t"           \
                                "andq   $0x7, %%rcx     \n\t"           \
                                "addq   %4, %%rcx       \n\t"           \
                                "shlq   $32, %%rcx      \n\t"           \
                                "addq   %%rcx, %%rax    \n\t"           \
                                "xorq   %%rcx, %%rcx    \n\t"           \
                                "movl   %%edx, %%ecx    \n\t"           \
                                "shrl   $16, %%ecx      \n\t"           \
                                "shlq   $48, %%rcx      \n\t"           \
                                "addq   %%rcx, %%rax    \n\t"           \
                                "movq   %%rax, %0       \n\t"           \
                                "shlq   $32, %%rdx      \n\t"           \
                                "movq   %%rdx, %1       \n\t"           \
                                :"=m" (*((unsigned long*)(gate_selector_address))),      \
                                "=m" (*(1 + (unsigned long*)(gate_selector_address))),   \
                                "=&a" (_d0), "=&d"(_d1)                 \
                                :"i" ((unsigned)attr << 8u),            \
                                "3" ((unsigned long*)(segment_offset)), \
                                "2" (0x8u << 16u),                      \
                                "c" (ist)                               \
                                :"memory");                             \


inline void set_int_gate(unsigned int int_vector, unsigned char ist, void *handler_address) {
    _set_gate(IDT_Table + int_vector, 0x8E, ist, handler_address); // attr(47~40): P=1, DPL=0, TYPE=E
}

inline void set_trap_gate(unsigned int int_vector, unsigned char ist, void *handler_address) {
    _set_gate(IDT_Table + int_vector, 0x8F, ist, handler_address); // attr(47~40): P=1, DPL=0, TYPE=F
}

inline void set_sys_trap_gate(unsigned int int_vector, unsigned char ist, void *handler_address) {
    _set_gate(IDT_Table + int_vector, 0xEF, ist, handler_address); // attr(47~40): P=1, DPL=3, TYPE=F
}

void setup_TSS(unsigned long rsp0, unsigned long rsp1, unsigned long rsp2, unsigned long ist1, unsigned long ist2,
               unsigned long ist3, unsigned long ist4, unsigned long ist5, unsigned long ist6, unsigned long ist7) {
    *(unsigned long *) (TSS_Table + 1) = rsp0;
    *(unsigned long *) (TSS_Table + 3) = rsp1;
    *(unsigned long *) (TSS_Table + 5) = rsp2;
    *(unsigned long *) (TSS_Table + 7) = ist1;
    *(unsigned long *) (TSS_Table + 9) = ist2;
    *(unsigned long *) (TSS_Table + 11) = ist3;
    *(unsigned long *) (TSS_Table + 13) = ist4;
    *(unsigned long *) (TSS_Table + 15) = ist5;
    *(unsigned long *) (TSS_Table + 17) = ist6;
    *(unsigned long *) (TSS_Table + 18) = ist7;
}

#endif //_GATE_H
