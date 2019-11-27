//
// Created by 赵明明 on 2019/11/26.
//
#include "trap.h"
#include "../lib/defs.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"

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

static void general_exception_handling(char *name, unsigned long rsp, unsigned long error_code) {
    unsigned long *rip = NULL;
    rip = (unsigned long *) (rsp + 0x98);
    print_color(RED, BLACK, "%s, ERROR CODE:%#018lX, RSP:%#018lX, RIP:%#018lX\n", name, error_code, rsp, *rip);
    hlt();
}

void handle_divide_error(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Divide Error[DE]", rsp, error_code);
}

void handle_debug_exception(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Debug Exception[DB]", rsp, error_code);
}

void handle_nmi(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("NMI", rsp, error_code);
}

void handle_breakpoint(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Breakpoint[BP]", rsp, error_code);
}

void handle_overflow(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Overflow[OF]", rsp, error_code);
}

void handle_bound_range_exceeded(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Bound Range Exceeded[BR]", rsp, error_code);
}

void handle_invalid_Opcode(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Invalid Opcode[UD]", rsp, error_code);
}

void handle_device_not_available(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Device Not Available[NM]", rsp, error_code);
}

void handle_double_fault(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Double Fault[DF]", rsp, error_code);
}

void handle_invalid_TSS(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Invalid TSS[TS]", rsp, error_code);
}

void handle_segment_not_present(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Segment Not Present[NP]", rsp, error_code);
}

void handle_stack_segment_fault(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Stack Segment Fault[SS]", rsp, error_code);
}

void handle_general_protection(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("General Protection[GP]", rsp, error_code);
}

void handle_page_fault(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Page Fault[PF]", rsp, error_code);
}

void handle_x87_FPU_floating_point_error(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("x87 FPU Floating Point Error[MF]", rsp, error_code);
}

void handle_alignment_check(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Alignment Check[AC]", rsp, error_code);
}

void handle_machine_check(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Machine Check[MC]", rsp, error_code);
}

void handle_SIMD_floating_point_exception(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("SIMD Floating Point Exception[XM]", rsp, error_code);
}

void handle_virtualization_exception(unsigned long rsp, unsigned long error_code) {
    general_exception_handling("Virtualization Exception[VE]", rsp, error_code);
}