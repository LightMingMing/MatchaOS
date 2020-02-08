//
// Created by 赵明明 on 2020/2/7.
//

#ifndef _HPET_H
#define _HPET_H

#define HPET_IRQ_NR 0x22

#define HPET_BASE 0xFED00000

#define GEN_CAP_AND_ID_REG  0x00
#define GEN_CONF_REG        0x10
#define MAIN_COUNTER_REG    0xF0
#define TIMER0_CONF_REG     0x100
#define TIMER0_COMP_REG     0x108

void HPET_init();

#endif //_HPET_H
