#include "linkage.h"

R15 =	0x00
R14 =	0x08
R13 =	0x10
R12 =	0x18
R11 =	0x20
R10 =	0x28
R9  =	0x30
R8  =	0x38
RBX =	0x40
RCX =	0x48
RDX =	0x50
RSI =	0x58
RDI =	0x60
RBP =	0x68
DS  =	0x70
ES  =	0x78
RAX =	0x80
FUNC	=	0x88
ERRCODE	=	0x90
RIP =	0x98
CS  =	0xa0
RFLAGS	=	0xa8
OLDRSP	=	0xb0
OLDSS	=	0xb8


ENTRY(divide_error)
	pushq	$0
	pushq	%rax
	leaq	handle_divide_error(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(debug_exception)
	pushq	$0
	pushq	%rax
	leaq	handle_debug_exception(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(nmi)
	pushq	$0
	pushq	%rax
	leaq	handle_nmi(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(breakpoint)
	pushq	$0
	pushq	%rax
	leaq	handle_breakpoint(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(overflow)
	pushq	$0
	pushq	%rax
	leaq	handle_overflow(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(bound_range_exceeded)
	pushq	$0
	pushq	%rax
	leaq	handle_bound_range_exceeded(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(invalid_Opcode)
	pushq	$0
	pushq	%rax
	leaq	handle_invalid_Opcode(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(device_not_available)
	pushq	$0
	pushq	%rax
	leaq	handle_device_not_available(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(double_fault)
	pushq	%rax
	leaq	handle_double_fault(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(invalid_TSS)
	pushq	%rax
	leaq	handle_invalid_TSS(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(segment_not_present)
	pushq	%rax
	leaq	handle_segment_not_present(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(stack_segment_fault)
	pushq	%rax
	leaq	handle_stack_segment_fault(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(general_protection)
	pushq	%rax
	leaq	handle_general_protection(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(page_fault)
	pushq	%rax
	leaq	handle_page_fault(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(x87_FPU_floating_point_error)
	pushq	$0
	pushq	%rax
	leaq	handle_x87_FPU_floating_point_error(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(alignment_check)
	pushq	%rax
	leaq	handle_alignment_check(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(machine_check)
	pushq	$0
	pushq	%rax
	leaq	handle_machine_check(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(SIMD_floating_point_exception)
	pushq	$0
	pushq	%rax
	leaq	handle_SIMD_floating_point_exception(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


ENTRY(virtualization_exception)
	pushq	$0
	pushq	%rax
	leaq	handle_virtualization_exception(%rip), %rax
	xchgq	%rax, (%rsp)
	jmp	exception_handling


exception_handling:
	pushq	%rax
	movq	%es, %rax
	pushq	%rax
	movq	%ds, %rax
	pushq	%rax
	xorq	%rax, %rax
	
	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	
	cld
	movq	$0x10, %rdi
	movq	%rdi, %ds
	movq	%rdi, %es
	
	movq	ERRCODE(%rsp), %rsi
	movq	%rsp, %rdi	
	movq	FUNC(%rsp), %rdx
	callq	*%rdx


ENTRY(ret_from_intr)
restore_all:
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%rax
	movq	%rax, %ds
	popq	%rax
	movq	%rax, %es
	popq	%rax
	addq	$0x10, %rsp
	iretq

ENTRY(system_call)
	subq    $0x38, %rsp
	cld

	pushq	%rax
	movq	%es, %rax
	pushq	%rax
	movq	%ds, %rax
	pushq	%rax
	xorq	%rax, %rax

	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rdx
	pushq	%rcx
	pushq	%rbx
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	movq    $0x10, %rdx
	movq    %rdx, %ds
	movq    %rdx, %es
	movq    %rsp, %rdi

	callq   system_call_func

ENTRY(ret_system_call)
	movq    %rax, 0x80(%rsp)
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rbx
	popq	%rcx
	popq	%rdx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%rax
	movq	%rax, %ds
	popq	%rax
	movq	%rax, %es
	popq	%rax
	addq	$0x38, %rsp
	.byte   0x48
	sysexit
