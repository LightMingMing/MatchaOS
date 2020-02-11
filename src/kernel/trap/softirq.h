//
// Created by 赵明明 on 2020/2/9.
//

#ifndef _SOFTIRQ_H
#define _SOFTIRQ_H

#include "../lib/defs.h"

unsigned long soft_irq_status = 0;

#define TIMER_IRQ  (1UL<<0UL)

struct soft_irq {
    void (*action)(void *data);

    void *data;
};

struct soft_irq soft_irq_vector[64];

void set_soft_irq_status(unsigned long status);

void register_soft_irq(uint8_t nr, void (*action)(void *data), void *data);

void unregister_soft_irq(uint8_t nr);

#endif //_SOFTIRQ_H
