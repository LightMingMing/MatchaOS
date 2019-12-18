//
// Created by 赵明明 on 2019/12/12.
//

#ifndef _INTR_H
#define _INTR_H

#include "../linkage.h"

#define IRQ_NAME(intr_vector)  IRQ##intr_vector##_interrupt(void)

#define SAVE_ALL            \
    "pushq	%rax            \t\n"   \
    "pushq	%rax            \t\n"   \
    "movq	%es, %rax       \t\n"   \
    "pushq	%rax            \t\n"   \
    "movq	%ds, %rax       \t\n"   \
    "pushq	%rax            \t\n"   \
    "xorq	%rax, %rax      \t\n"   \
    "pushq	%rbp    \t\n"   \
    "pushq	%rdi    \t\n"   \
    "pushq	%rsi    \t\n"   \
    "pushq	%rdx    \t\n"   \
    "pushq	%rcx    \t\n"   \
    "pushq	%rbx    \t\n"   \
    "pushq	%r8     \t\n"   \
    "pushq	%r9     \t\n"   \
    "pushq	%r10    \t\n"   \
    "pushq	%r11    \t\n"   \
    "pushq	%r12    \t\n"   \
    "pushq	%r13    \t\n"   \
    "pushq	%r14    \t\n"   \
    "pushq	%r15    \t\n"   \

#define Build_IRQ(intr_vector)          \
    void IRQ_NAME(intr_vector);         \
    __asm__ (                           \
            "IRQ"#intr_vector"_interrupt:       \t\n"   \
            "pushq  $0x00                       \t\n"   \
            SAVE_ALL                                    \
            "movq   $0x10, %rax                 \t\n"   \
            "movq   %rax, %ds                   \t\n"   \
            "movq   %rax, %es                   \t\n"   \
            "leaq   ret_from_intr(%rip), %rax   \t\n"   \
            "pushq  %rax                        \t\n"   \
            "movq   $"#intr_vector", %rdi       \t\n"   \
            "movq   %rsp, %rsi                  \t\n"   \
            "jmp    handle_IRQ                  \t\n"   \
);

void intr_init();

void handle_IRQ(unsigned long intr_vector, unsigned long rsp);

#endif //_INTR_H
