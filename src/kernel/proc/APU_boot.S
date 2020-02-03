#include "../linkage.h"

.balign 0x1000

.text
.code16

ENTRY(_APU_boot_start)

_APU_base = .
	cli
	wbinvd
	
	mov	%cs, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	# 0x400
	movl	$(_APU_tmp_Stack_end - _APU_base), %esp

	# base address
	mov	%cs, %ax	# 0x2000
	movzx	%ax, %esi
	shll	$4, %esi	# 0x20000
	
	# protect mode code address
	leal	(_APU_code32 - _APU_base)(%esi), %eax
	movl	%eax, (_APU_code32_vector - _APU_base)

	# long mode code address
	leal	(_APU_code64 - _APU_base)(%esi), %eax
	movl	%eax, (_APU_code64_vector - _APU_base)

	#
	leal	(_APU_tmp_GDT - _APU_base)(%esi), %eax
	movl	%eax, (_APU_tmp_GDT + 2 - _APU_base)

	lidtl	_APU_tmp_IDT - _APU_base
	lgdtl	_APU_tmp_GDT - _APU_base

	
	# enable protect mode
	movl	%cr0, %eax
	or	$1, %eax
	movl	%eax, %cr0

	;smsw	%ax
	;bts	$0, %ax
	;lmsw	%ax
		
	ljmpl	*(_APU_code32_vector - _APU_base)

.code32
.balign 4
_APU_code32:
	mov	$0x10, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss	

	# base 0x0000, offset = 0x20400
	leal	(_APU_tmp_Stack_end - _APU_base)(%esi), %eax
	movl	%eax, %esp
	
	# open PAE
	movl	%cr4, %eax
	bts	$5, %eax
	movl	%eax, %cr4

	# set page table
	movl	$0x90000, %eax
	movl	%eax, %cr3 
	
	# enable long-mode
	movl	$0xC0000080, %ecx
	rdmsr
	bts	$8, %eax
	wrmsr

	# open PE and paging
	movl	%cr0, %eax
	bts	$0, %eax
	bts	$31, %eax
	movl	%eax, %cr0

	ljmp	*(_APU_code64_vector - _APU_base)(%esi)

.code64
.balign 4
_APU_code64:
	mov	$0x20, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss
	
	hlt

.balign 4
_APU_tmp_IDT:
	.word	0
	.word	0, 0

.balign 4
_APU_tmp_GDT:
	.short	_APU_tmp_GDT_end - _APU_tmp_GDT - 1
	.long	_APU_tmp_GDT - _APU_base
	.short	0
	.quad	0x00CF9A000000FFFF	# 代码段, 选择子0x08
	.quad	0x00CF92000000FFFF	# 数据段, 选择子0x10
	.quad	0x0020980000000000	# 代码段, 选择子0x18
	.quad	0x0000920000000000	# 数据段, 选择子0x20
_APU_tmp_GDT_end:

.balign 4
_APU_code32_vector:
	.long	_APU_code32 - _APU_base
	.word	0x08, 0

.balign 4
_APU_code64_vector:
	.long	_APU_code64 - _APU_base
	.word	0x18, 0

.balign 4
_APU_tmp_Stack_start:
	.org 0x400
_APU_tmp_Stack_end:

ENTRY(_APU_boot_end)
