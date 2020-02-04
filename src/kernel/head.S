#include "linkage.h"

.section .text

.globl _start
_start:
	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %ss
	mov	$0x7E00, %esp

	lgdt	GDT_PTR(%rip)

	lidt	IDT_PTR(%rip)

	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %ss	
	movq	$0x7E00, %rsp
	
	movq	$0x101000, %rax
	movq	%rax, %cr3
	movq	switch_seg(%rip), %rax
	pushq	$0x08
	pushq	%rax
	lretq

switch_seg:
	.quad	entry64

entry64:
	movq	$0x10, %rax
	movq	%rax, %ds
	movq	%rax, %es
	movq	%rax, %fs
	movq	%rax, %ss
	movq	_stack_start(%rip), %rsp

	movq    $0x1B, %rcx
	rdmsr
	bt	$8, %rax
	jnc	start_smp

setup_IDT:
	leaq	ignore_int(%rip), %rdx
	movq	$(0x8E0000080000), %rax
	movw	%dx, %ax
	movl	%edx, %ecx
	shrl	$16, %ecx
	shlq	$48, %rcx
	addq	%rcx, %rax
	shrq	$32, %rdx

	leaq	IDT_Table(%rip), %rdi
	mov	$256, %rcx
setup_next_int_desc:
	movq	%rax, (%rdi)
	movq	%rdx, 8(%rdi)
	addq	$16, %rdi
	dec	%rcx
	jne	setup_next_int_desc

setup_TSS:
	leaq	TSS_Table(%rip), %rdx
	xorq	%rax, %rax
	xorq	%rcx, %rcx
	mov	$0x89, %ax
	shlq	$40, %rax
	movl	%edx, %ecx
	shrl	$24, %ecx
	shlq	$56, %rcx
	addq	%rcx, %rax
	xorq	%rcx, %rcx
	addq	%rcx, %rax
	movl	%edx, %ecx
	andl	$0xffffff, %ecx
	shlq	$16, %rcx
	addq	%rcx, %rax
	mov	$103, %ax
	
	leaq	GDT_Table(%rip), %rdi
	movq	%rax, 80(%rdi)
	shrq	$32, %rdx
	movq	%rdx, 88(%rdi)

	movq	go_to_kernel(%rip), %rax
	pushq	$0x08
	pushq	%rax
	lretq

go_to_kernel:
	.quad	Start_Kernel

start_smp:
	movq	go_to_smp(%rip), %rax
	pushq	$0x08
	pushq	%rax
	lretq

go_to_smp:
	.quad	Start_SMP

ignore_int:
	movq	$0x10, %rax
	movq	%rax, %ds
	movq	%rax, %es

	leaq	int_msg(%rip), %rdx
	movq	$0x00000000, %rsi
	movq	$0x00ff0000, %rdi
	movq	$0, %rax
	callq	print_color
	addq	$0x8, %rsp

	hlt

int_msg:
	.asciz	"Unknown interrupt or exception!\n"

ENTRY(_stack_start)
    .quad   init_proc_union + 32768

// init page
/*
2M大小的物理页时, 线性地址映射方式
-------------------------------------------
|63   48|47   39|38   30|29   21|20      0|
| SIGN  | PML4  | PDPT  |  PDT  |  OFFSET |
-------------------------------------------
这里达到这样的效果:
1. 线性地址0和0xffff800000000000开始的10M内存映射到物理地址0处
2. 线性地址0xa00000和0xffff800000a00000开始的16M内存映射到物理地址0xe000000000处
线性地址			物理地址
0000 0000 0000 0000	->	0000 0000 0000 0000
......
0000 0000 0080 0000	->	0000 0000 0080 0000

ffff 8000 0000 0000	->	0000 0000 0000 0000
......
ffff 8000 0080 0000	->	0000 0000 0080 0000

0000 0000 00a0 0000	->	0000 000e 0000 0000
......
0000 0000 01a0 0000	->	0000 000e 0100 0000

ffff 8000 00a0 0000	->	0000 000e 0000 0000
......
ffff 8000 01a0 0000	->	0000 000e 0100 0000
*/
.align 8

.org 0x1000
// Page Map Level 4, 物理地址: 0x101000
_PML4E:
	.quad 0x102007
	.fill 255, 8, 0
	.quad 0x102007
	.fill 255, 8, 0

.org 0x2000
// Page Directory Pointer Table Entry, 物理地址: 0x102000
_PDPTE:
	.quad 0x103007
	.fill 511, 8, 0

.org 0x3000
// Page Directory Entry, 物理地址: 0x103000
_PDE:
	.quad 0x000087
	.quad 0x200087
	.quad 0x400087
	.quad 0x600087
	.quad 0x800087
	.quad 0xa00087
	.quad 0xc00087
	.quad 0xe00087
	.quad 0x1000087
	.quad 0x1200087
	.quad 0x1400087
	.quad 0x1600087
	.quad 0x1800087
	.quad 0x1a00087
	.quad 0x1c00087
	.quad 0x1e00087
	.quad 0x2000087
	.quad 0x2200087
	.quad 0x2400087
	.quad 0x2600087
	.quad 0x2800087
	.quad 0x2a00087
	.quad 0x2c00087
	.quad 0x2e00087     // 0x2e00000
	.quad 0xe0000087    // 0x3000000
	.quad 0xe0200087
	.quad 0xe0400087
	.quad 0xe0600087
	.quad 0xe0800087
	.quad 0xe0a00087
	.quad 0xe0c00087
	.quad 0xe0e00087
	.quad 0xfd000087    // 0x4000000
	.quad 0xfd200087
	.quad 0xfd400087
	.quad 0xfd600087
	.quad 0xfd800087
	.quad 0xfda00087
	.quad 0xfdc00087
	.quad 0xfde00087
	.fill 472, 8, 0
// 这里使用2MB物理页, 不再需要PTE(Page Table Entry)


.section .data

// init GDT
.globl GDT_Table

GDT_Table:
	.quad 0x0000000000000000 // NULL descriptor
	.quad 0x0020980000000000 // KERNEL Code 64-bit Segment 0x08
	.quad 0x0000920000000000 // KERNEL Data 64-bit Segment 0x10
	.quad 0x0000000000000000 // User   Code 32-bit Segment 0x18
	.quad 0x0000000000000000 // User   Data 32-bit Segment 0x20
	.quad 0x0020f80000000000 // User   Code 64-bit Segment 0x28
	.quad 0x0000f20000000000 // User   Data 64-bit Segment 0x30
	.quad 0x00cf9a000000ffff // KERNEL Code 32-bit Segment 0x38
	.quad 0x00cf92000000ffff // KERNEL Data 32-bit Segment 0x40
	.fill 10, 8, 0           // 10 ~ 11 TSS
GDT_END:

GDT_PTR:
GDT_LIMIT:	.word GDT_END - GDT_Table - 1
GDT_BASE:	.quad GDT_Table

// init IDT
.globl IDT_Table

IDT_Table:
	.fill 512, 8, 0
IDT_END:

IDT_PTR:
IDT_LIMIT:	.word IDT_END - IDT_Table - 1
IDT_BASE:	.quad IDT_Table

// init TSS
.globl TSS_Table

TSS_Table:
	.fill 13, 8, 0
TSS_END:

TSS_PTR:
TSS_LIMIT:	.word TSS_END - TSS_Table - 1
TSS_BASE:	.quad TSS_Table
