//
// Created by 赵明明 on 2020/2/9.
//

#include "softirq.h"
#include "../lib/defs.h"
#include "../lib/x86.h"

void set_soft_irq_status(unsigned long status) {
    soft_irq_status |= status;
}

void register_soft_irq(uint8_t nr, void (*action)(void *), void *data) {
    soft_irq_vector[nr].action = action;
    soft_irq_vector[nr].data = data;
}

void unregister_soft_irq(uint8_t nr) {
    soft_irq_vector[nr].action = NULL;
    soft_irq_vector[nr].data = NULL;
}

void do_soft_irq() {
    sti();
    for (uint8_t i = 0; i < 64 && soft_irq_status; i++) {
        if (soft_irq_status >> i & 0x1U) {
            struct soft_irq soft_irq = soft_irq_vector[i];
            if (soft_irq.action)
                soft_irq.action(soft_irq.data);
            soft_irq_status &= ~(1UL << i);
        }
    }
    cli();
}