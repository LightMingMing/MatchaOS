//
// Created by 赵明明 on 2019/12/12.
//

#ifndef _INTR_H
#define _INTR_H

#include "../lib/defs.h"
#include "../lib/reg.h"
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

extern void (*interrupt[24])(void);

extern void (*IPI[10])(void);

void intr_init();

typedef unsigned char irq_nr_t;

extern void handle_IRQ(irq_nr_t nr, regs_t *rsp);

typedef struct {
    void (*enable)(irq_nr_t nr);

    void (*disable)(irq_nr_t nr);

    void (*install)(irq_nr_t nr, void *arg);

    void (*uninstall)(irq_nr_t nr);

    void (*ack)(irq_nr_t irq);
} irq_ctl_t;

typedef struct {
    char *name;
    irq_ctl_t *ctl;

    void (*handler)(irq_nr_t nr, regs_t *regs);
} irq_desc_t;

typedef struct {
    char *name;

    void (*handler)(irq_nr_t nr, regs_t *regs);
} IPI_desc_t;

#define NR_IRQs  24
irq_desc_t IRQ_Table[NR_IRQs];

#define NR_IPIs 10
IPI_desc_t IPI_Table[NR_IPIs];

int register_irq(irq_nr_t nr, char *name, irq_ctl_t *ctl, void *arg,
                 void(*handler)(irq_nr_t, regs_t *)) {
    irq_desc_t *irq = &IRQ_Table[nr - 0x20];
    irq->name = name;
    irq->ctl = ctl;
    irq->handler = handler;

    irq->ctl->install(nr, arg);
    irq->ctl->enable(nr);
    return 1;
}

int unregister_irq(irq_nr_t nr) {
    irq_desc_t *irq = &IRQ_Table[nr - 0x20];
    irq->ctl->disable(nr);
    irq->ctl->uninstall(nr);

    irq->name = NULL;
    irq->ctl = NULL;
    irq->handler = NULL;
    return 1;
}

int register_IPI(uint8_t nr, char *name, void(*handler)(uint8_t, regs_t *)) {
    IPI_desc_t *ipi = &IPI_Table[nr - 200];
    ipi->name = name;
    ipi->handler = handler;
    return 1;
}

int unregister_IPI(uint8_t nr) {
    IPI_desc_t *ipi = &IPI_Table[nr - 200];
    ipi->name = NULL;
    ipi->handler = NULL;
    return 1;
}

#endif //_INTR_H
