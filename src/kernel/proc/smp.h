//
// Created by 赵明明 on 2020/2/2.
//

#ifndef _SMP_H
#define _SMP_H

#include "../lib/defs.h"
#include "../trap/apic.h"

extern unsigned char _APU_boot_start;
extern unsigned char _APU_boot_end;

void send_IPI(struct ICR_Entry *entry);

void smp_init();

void Start_SMP();

#endif //_SMP_H
