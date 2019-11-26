//
// Created by 赵明明 on 2019/11/26.
//

#ifndef _TRAP_H
#define _TRAP_H

#include "gate.h"

void trap_init();

void divide_error();

void debug_exception();

void nmi();

void breakpoint();

void overflow();

void bound_range_exceeded();

void invalid_Opcode();

void device_not_available();

void double_fault();

void invalid_TSS();

void segment_not_present();

void stack_segment_fault();

void general_protection();

void page_fault();

void x87_FPU_floating_point_error();

void alignment_check();

void machine_check();

void SIMD_floating_point_exception();

void virtualization_exception();

#endif //_TRAP_H
