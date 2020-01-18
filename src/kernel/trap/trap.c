//
// Created by 赵明明 on 2019/11/26.
//
#include "trap.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"

void trap_init() {
    set_trap_gate(0, 1, divide_error);
    set_trap_gate(1, 1, debug_exception);
    set_intr_gate(2, 1, nmi);
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

static void general_exception_handling(char *name, regs_t *regs, unsigned long error_code) {
    unsigned int idx;
    print_color(RED, BLACK, "%s, ERROR CODE:%#018lX, RSP:%#018lX, RIP:%#018lX\n", name, error_code, regs->rsp,
                regs->rip);
    if (error_code & 1u)
        print_color(RED, BLACK, "Exception occurred during delivery of an even external to the program\n");
    idx = (error_code & 0xfff8u) >> 3u;
    if (error_code & 2u)
        print_color(RED, BLACK, "Refers to a gate descriptor [%#04X] in the IDT", idx);
    else {
        if (error_code & 4u)
            print_color(RED, BLACK, "Refers to a descriptor [%#04X] in the LDT");
        else
            print_color(RED, BLACK, "Refers to a descriptor [%#04X] in the GDT");
    }
    println("");
    hlt();
}

void handle_divide_error(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Divide Error[DE]", regs, error_code);
}

void handle_debug_exception(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Debug Exception[DB]", regs, error_code);
}

void handle_nmi(regs_t *regs, unsigned long error_code) {
    general_exception_handling("NMI", regs, error_code);
}

void handle_breakpoint(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Breakpoint[BP]", regs, error_code);
}

void handle_overflow(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Overflow[OF]", regs, error_code);
}

void handle_bound_range_exceeded(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Bound Range Exceeded[BR]", regs, error_code);
}

void handle_invalid_Opcode(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Invalid Opcode[UD]", regs, error_code);
}

void handle_device_not_available(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Device Not Available[NM]", regs, error_code);
}

void handle_double_fault(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Double Fault[DF]", regs, error_code);
}

void handle_invalid_TSS(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Invalid TSS[TS]", regs, error_code);
}

void handle_segment_not_present(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Segment Not Present[NP]", regs, error_code);
}

void handle_stack_segment_fault(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Stack Segment Fault[SS]", regs, error_code);
}

void handle_general_protection(regs_t *regs, unsigned long error_code) {
    general_exception_handling("General Protection[GP]", regs, error_code);
}

void handle_page_fault(regs_t *regs, unsigned long error_code) {
    unsigned long cr2 = 0;
    __asm__ __volatile__ ("movq %%cr2, %0":"=r"(cr2)::"memory");
    print_color(RED, BLACK, "Page Fault[PF], ERROR CODE:%#018lX, RSP:%#018lX, RIP:%#018lX\n", error_code, regs->rsp,
                regs->rip);
    if (error_code & 1u)
        print_color(RED, BLACK, "Page Not-Presented, ");
    if (error_code & 2u)
        print_color(RED, BLACK, "Write caused the fault, ");
    else
        print_color(RED, BLACK, "Read caused the fault, ");
    if (error_code & 4u)
        print_color(RED, BLACK, "User-mode access caused the fault, ");
    else
        print_color(RED, BLACK, "Supervisor-mode access caused the fault, ");
    if (error_code & 8u)
        print_color(RED, BLACK, "Set reserved bit caused the fault, ");
    if (error_code & 16u)
        print_color(RED, BLACK, "Instruction fetch caused the fault, ");
    print_color(RED, BLACK, "CR2:%#018X\n", cr2);
    hlt();
}

void handle_x87_FPU_floating_point_error(regs_t *regs, unsigned long error_code) {
    general_exception_handling("x87 FPU Floating Point Error[MF]", regs, error_code);
}

void handle_alignment_check(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Alignment Check[AC]", regs, error_code);
}

void handle_machine_check(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Machine Check[MC]", regs, error_code);
}

void handle_SIMD_floating_point_exception(regs_t *regs, unsigned long error_code) {
    general_exception_handling("SIMD Floating Point Exception[XM]", regs, error_code);
}

void handle_virtualization_exception(regs_t *regs, unsigned long error_code) {
    general_exception_handling("Virtualization Exception[VE]", regs, error_code);
}