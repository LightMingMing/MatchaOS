//
// Created by 赵明明 on 2020/2/2.
//

#ifndef _SMP_H
#define _SMP_H

// Interrupt Command Register
#define ICR_LOW     0xFEE00300
#define ICR_HIGH    0xFEE00310

// Logical Destination Register
#define LDR 0xFEE000D0
// Destination Format Register
#define DFR 0xFEE000E0

#define FLAT_MODEL      0x1111U
#define CLUSTER_MODEL   0x0000U

extern unsigned char _APU_boot_start;
extern unsigned char _APU_boot_end;

void smp_init();

#endif //_SMP_H
