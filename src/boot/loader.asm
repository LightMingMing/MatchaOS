org 10000h

%include "fat12.inc"

jmp	Label_Start

[SECTION gdt]

LABEL_GDT:		dd 0,0				; 空段描述符
LABEL_DESC_CODE32:	dd 0x0000FFFF,0x00CF9A00	; 代码段描述符
LABEL_DESC_DATA32:	dd 0x0000FFFF,0x00CF9200	; 数据段描述符

GdtLen	equ	$ - LABEL_GDT
GdtPtr	dw	GdtLen - 1
	dd	LABEL_GDT

SelectorCode32	equ LABEL_DESC_CODE32 - LABEL_GDT	; 代码段选择子, 值为0x8
SelectorData32	equ LABEL_DESC_DATA32 - LABEL_GDT	; 数据段选择子, 值为0x10


[SECTION .s16]
[BITS 16]

Label_Start:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ax, 0
	mov	ss, ax
	mov	sp, 0x7c00

	mov	ax, 1301h
	mov	bx, 000fh
	mov	cx, 12
	mov	dx, 0200h
	push	ax,
	mov	ax, ds
	mov	es, ax
	pop	ax
	mov	bp, StartLoaderMessage
	int	10h

	; open address A20
	in	al, 92h
	or	al, 0010b	; 置位0x92端口第一位, 使20位以上地址有效
	out	92h, al

	cli	; 屏蔽外部中断
	
	; 加载保护模式下临时全局描述符
	db	0x66
	lgdt	[GdtPtr]
	
	; 置位cr0第0位, 开启保护模式
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax
	
	; 加载数据段选择子至fs段寄存器
	mov	ax, SelectorData32
	mov	fs, ax

	; 退出保护模式
	mov	eax, cr0
	and	al, 0xfe
	mov	cr0, eax

	sti	; 开启外部中断

	jmp	Label_Finish

StartLoaderMessage:	db "Start Loader"
