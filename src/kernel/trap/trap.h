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

void handle_divide_error(unsigned long rsp, unsigned long error_code);

void handle_debug_exception(unsigned long rsp, unsigned long error_code);

void handle_nmi(unsigned long rsp, unsigned long error_code);

void handle_breakpoint(unsigned long rsp, unsigned long error_code);

void handle_overflow(unsigned long rsp, unsigned long error_code);

void handle_bound_range_exceeded(unsigned long rsp, unsigned long error_code);

void handle_invalid_Opcode(unsigned long rsp, unsigned long error_code);

void handle_device_not_available(unsigned long rsp, unsigned long error_code);

void handle_double_fault(unsigned long rsp, unsigned long error_code);

void handle_invalid_TSS(unsigned long rsp, unsigned long error_code);

void handle_segment_not_present(unsigned long rsp, unsigned long error_code);

void handle_stack_segment_fault(unsigned long rsp, unsigned long error_code);

void handle_general_protection(unsigned long rsp, unsigned long error_code);

void handle_page_fault(unsigned long rsp, unsigned long error_code);

void handle_x87_FPU_floating_point_error(unsigned long rsp, unsigned long error_code);

void handle_alignment_check(unsigned long rsp, unsigned long error_code);

void handle_machine_check(unsigned long rsp, unsigned long error_code);

void handle_SIMD_floating_point_exception(unsigned long rsp, unsigned long error_code);

void handle_virtualization_exception(unsigned long rsp, unsigned long error_code);

#endif //_TRAP_H
