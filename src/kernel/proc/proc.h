//
// Created by 赵明明 on 2019/12/16.
//

#ifndef _PROC_H
#define _PROC_H

#include "../lib/defs.h"
#include "../mm/memory.h"
#include "../lib/list.h"
#include "../trap/gate.h"
#include "../lib/stdio.h"
#include "../lib/reg.h"

#define NR_CPU 4

#define PROC_STACK_SIZE  32768 // 32768 Byte (32KB)

#define PROC_RUNNABLE (1u<<0u)
#define PROC_INTERRUPTIBLE (1u<<1u)
#define PROC_UNINTERRUPTIBLE (1u<<2u)

#define PROC_KERNEL_THREAD (1u<<0u)

#define KERNEL_CS (0x08u)
#define KERNEL_DS (0x10u)

extern unsigned long _stack_start;

struct proc_ctx {
    uint64_t rsp0; // stack base addr
    uint64_t rsp;
    uint64_t rip;

    uint64_t fs;
    uint64_t gs;

    uint64_t cr2;
    uint64_t trap_nr;
    uint64_t error_code;
};

struct mm_struct {
    pml4t_t *pml4t;
    uint64_t start_code, end_code;
    uint64_t start_data, end_data;
    uint64_t start_rodata, end_rodata; // readonly
    uint64_t start_brk, end_brk;
    uint64_t start_stack;
};

struct proc_struct {
    int pid;
    int priority;
    uint32_t state;
    uint32_t flags;
    struct proc_ctx *ctx;
    struct mm_struct *mm;
    list_t list;
};

union proc_union {
    struct proc_struct proc;
    uint64_t stack[PROC_STACK_SIZE / sizeof(uint64_t)];
}__attribute__((aligned (8)));

struct mm_struct init_mm;

struct proc_ctx init_ctx;

#define INIT_PROC(proc) {   \
    .pid = 0,           \
    .priority = 0,      \
    .state = PROC_UNINTERRUPTIBLE,  \
    .flags = PROC_KERNEL_THREAD,    \
    .mm = &init_mm,     \
    .ctx = &init_ctx    \
}

union proc_union init_proc_union __attribute__((__section__ (".data.init_task"))) = {INIT_PROC(init_proc_union.proc)};

struct mm_struct init_mm = {0};

struct proc_ctx init_ctx = {
        .rsp0 = (unsigned long) (init_proc_union.stack + PROC_STACK_SIZE / sizeof(uint64_t)),
        .rsp = (unsigned long) (init_proc_union.stack + PROC_STACK_SIZE / sizeof(uint64_t)),
        .fs = KERNEL_DS,
        .gs = KERNEL_DS,
        .cr2 = 0,
        .trap_nr = 0,
        .error_code = 0
};

// 64-Bit TSS Format
struct tss_struct {
    uint32_t rsd0; // reserved
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t rsd1; // reserved

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint64_t rsd2; // reserved
    uint16_t rsd3; // reserved
    uint16_t io_map_base_addr;
}__attribute__((packed));

#define INIT_TSS {  \
    .rsd0 = 0,  \
    .rsp0 = (unsigned long) (init_proc_union.stack + PROC_STACK_SIZE / sizeof(uint64_t)),   \
    .rsp1 = (unsigned long) (init_proc_union.stack + PROC_STACK_SIZE / sizeof(uint64_t)),   \
    .rsp2 = (unsigned long) (init_proc_union.stack + PROC_STACK_SIZE / sizeof(uint64_t)),   \
    .rsd1 = 0,  \
    .ist1 = 0xffff800000007c00, \
    .ist2 = 0xffff800000007c00, \
    .ist3 = 0xffff800000007c00, \
    .ist4 = 0xffff800000007c00, \
    .ist5 = 0xffff800000007c00, \
    .ist6 = 0xffff800000007c00, \
    .ist7 = 0xffff800000007c00, \
    .rsd2 = 0,  \
    .rsd3 = 0,  \
    .io_map_base_addr = 0       \
}

struct tss_struct init_tss[NR_CPU] = {[0 ... NR_CPU - 1]=INIT_TSS};

static inline struct proc_struct *get_current() {
    struct proc_struct *current = NULL;
    __asm__ __volatile__ ("andq %%rsp, %0":"=r"(current):"0"(~32767UL));
    return current;
}

#define switch_to(prev, next)       \
    __asm__ __volatile__ (          \
            "pushq  %%rbp               \n\t"   \
            "pushq  %%rax               \n\t"   \
            "movq   %%rsp, %0           \n\t"   \
            "movq   %2, %%rsp           \n\t"   \
            "leaq   1f(%%rip), %%rax    \n\t"   \
            "movq   %%rax, %1           \n\t"   \
            "movq   %2, %%rsp           \n\t"   \
            "pushq  %3                  \n\t"   \
            "jmp    __switch_to         \n\t"   \
            "1:                         \n\t"   \
            "popq   %%rax               \n\t"   \
            "popq   %%rbp               \n\t"   \
            :"=m"(prev->ctx->rsp), "=m"(prev->ctx->rip) \
            :"m"(next->ctx->rsp), "m"(next->ctx->rip),  \
             "D"(prev), "S"(next)               \
            :"memory")

void __switch_to(struct proc_struct *prev, struct proc_struct *next) {
    init_tss[0].rsp0 = next->ctx->rsp0;
    setup_TSS(init_tss[0].rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2,
              init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);

    __asm__ __volatile__("movq  %%fs, %0":"=a"(prev->ctx->fs));
    __asm__ __volatile__("movq  %%gs, %0":"=a"(prev->ctx->gs));

    __asm__ __volatile__("movq  %0, %%fs"::"a"(next->ctx->fs));
    __asm__ __volatile__("movq  %0, %%gs"::"a"(next->ctx->gs));

    print_color(GREEN, BLACK, "prev->ctx->rsp0:%#018lx\n", prev->ctx->rsp0);
    print_color(GREEN, BLACK, "next->ctx->rsp0:%#018lx\n", next->ctx->rsp0);
}

typedef unsigned long (*system_call_t)(regs_t *regs);

unsigned long no_system_call(regs_t *regs) {
    print_color(RED, BLACK, "no_system_call is calling, NR:%04x\n", regs->rax);
    return -1;
}

unsigned long sys_printf(regs_t *regs) {
    print_color(BLACK, WHITE, (char *) regs->rdi);
    return 1;
}

system_call_t system_call_table[128] = {
        [0] = no_system_call,
        [1] = sys_printf,
        [2 ... 127] = no_system_call
};

void proc_init();

int do_fork(regs_t *regs, unsigned long flags, unsigned long stack_start, unsigned long stack_end);

#endif //_PROC_H
