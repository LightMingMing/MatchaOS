//
// Created by 赵明明 on 2019/11/26.
//
#include "trap.h"

void trap_init() {
    set_trap_gate(0, 1, divide_error);
    set_trap_gate(1, 1, debug_exception);
    set_int_gate(2, 1, nmi);
    set_sys_trap_gate(3, 1, breakpoint);
    set_sys_trap_gate(4, 1, overflow);
    set_sys_trap_gate(5, 1, bound_range_exceeded);
    set_trap_gate(6, 1, invalid_Opcode);
    set_trap_gate(7, 1, device_not_available);
    set_trap_gate(8, 1, double_fault);
    // 9 reserved
    set_trap_gate(10, 1, invalid_TSS);
    set_trap_gate(11, 1, segment_not_present);
    set_trap_gate(12, 1, stack_segment_fault);
    set_trap_gate(13, 1, general_protection);
    set_trap_gate(14, 1, page_fault);
    // 15 Intel reserved. Do not use
    set_trap_gate(16, 1, x87_FPU_floating_point_error);
    set_trap_gate(17, 1, alignment_check);
    set_trap_gate(18, 1, machine_check);
    set_trap_gate(19, 1, SIMD_floating_point_exception);
    set_trap_gate(20, 1, virtualization_exception);
    // 21-31 Intel reserved. Do not use

    // 32-255 User Defined Interrupts
}