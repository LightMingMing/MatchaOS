//
// Created by 赵明明 on 2019/11/26.
//

#ifndef _GATE_H
#define _GATE_H

struct gate_struct {
    unsigned char chs[16];
};

extern unsigned long GDT_Table[109];
extern struct gate_struct IDT_Table[];

#define load_TR(idx)   \
    __asm__ __volatile__ ("ltr  %%ax"::"a" ((idx) << 3u):"memory")

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
                                "shrq   $32, %%rdx      \n\t"           \
                                "movq   %%rdx, %1       \n\t"           \
                                :"=m" (*((unsigned long*)(gate_selector_address))),      \
                                "=m" (*(1 + (unsigned long*)(gate_selector_address))),   \
                                "=&a" (_d0), "=&d"(_d1)                 \
                                :"i" ((unsigned)attr << 8u),            \
                                "3" ((unsigned long*)(segment_offset)), \
                                "2" (0x8u << 16u),                      \
                                "c" (ist)                               \
                                :"memory")                              \


static inline void set_intr_gate(unsigned int intr_vector, unsigned char ist, void *handler_address) {
    _set_gate(IDT_Table + intr_vector, 0x8E, ist, handler_address); // attr(47~40): P=1, DPL=0, TYPE=E
}

static inline void set_trap_gate(unsigned int intr_vector, unsigned char ist, void *handler_address) {
    _set_gate(IDT_Table + intr_vector, 0x8F, ist, handler_address); // attr(47~40): P=1, DPL=0, TYPE=F
}

static inline void set_sys_trap_gate(unsigned int intr_vector, unsigned char ist, void *handler_address) {
    _set_gate(IDT_Table + intr_vector, 0xEF, ist, handler_address); // attr(47~40): P=1, DPL=3, TYPE=F
}

void setup_TSS(unsigned int *tss_addr, unsigned long rsp0, unsigned long rsp1, unsigned long rsp2, unsigned long ist1,
               unsigned long ist2,
               unsigned long ist3, unsigned long ist4, unsigned long ist5, unsigned long ist6, unsigned long ist7) {
    *(unsigned long *) (tss_addr + 1) = rsp0;
    *(unsigned long *) (tss_addr + 3) = rsp1;
    *(unsigned long *) (tss_addr + 5) = rsp2;
    *(unsigned long *) (tss_addr + 7) = ist1;
    *(unsigned long *) (tss_addr + 9) = ist2;
    *(unsigned long *) (tss_addr + 11) = ist3;
    *(unsigned long *) (tss_addr + 13) = ist4;
    *(unsigned long *) (tss_addr + 15) = ist5;
    *(unsigned long *) (tss_addr + 17) = ist6;
    *(unsigned long *) (tss_addr + 18) = ist7;
}

// n = 10 + 2*i (i >= 0, < 50)
void set_tss_desc(unsigned int n, void *tss_addr) {
    unsigned long limit = 103;
    GDT_Table[n] = (limit & 0xFFFFU) | ((unsigned long) tss_addr & 0xFFFFFFU) << 16U | 0x89UL << 40U |
                   ((limit >> 16U) & 0xFU) << 48U | ((unsigned long) tss_addr >> 24U & 0xFFU) << 56U;
    GDT_Table[n + 1] = (unsigned long) tss_addr >> 32U;
}

#endif //_GATE_H
