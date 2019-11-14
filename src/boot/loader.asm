org 10000h

%include "fat12.inc"

jmp	Label_Start

BaseOfKernel		equ 0x00
OffsetOfKernel		equ 0x100000
TmpBaseOfKernel		equ 0x00
TmpOffsetOfKernel	equ 0x7e00
AddrRangeDescBuffer	equ 0x7e00

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
	
Label_Search_Kernel_File:
	mov	byte [DisMsgLineNum], 4
	mov	ax, KernelFileName
	call	Func_Search_File_In_Root_Dir
	cmp	ax, 0
	je	Label_No_Kernel_Found

	; 将Kernel.bin加载至临时区内存0x7e00处
	mov	bx, TmpBaseOfKernel
	mov	es, bx
	mov	bx, TmpOffsetOfKernel

Label_Load_File:
	push	ax
	push	bx
	mov	ah, 0eh
	mov	al, '.'
	mov	bl, 0fh
	int	10h
	pop	bx
	pop	ax

	push	ax
	add	ax, FirstDataSecNum
	sub	ax, FirstDataClus
	mov	cl, 1
	call	Func_Read_Sector
	pop	ax

	; 0x7e00临时区内数据拷贝至内存0x100000处
	push	cx
	push	eax
	push	fs
	push	edi
	push	ds
	push	esi
	
	mov	cx, [BPB_BytsPerSec]
	mov	ax, BaseOfKernel
	mov	fs, ax
	mov	edi, dword [CurrentOffsetOfKernel]

	mov	ax, TmpBaseOfKernel
	mov	ds, ax
	mov	esi, TmpOffsetOfKernel

Loop_Copy_Kernel:
	mov	al, byte [ds:esi]
	mov	byte [fs:edi], al
	inc	esi
	inc	edi
	loop	Loop_Copy_Kernel
	
	mov	eax, 0x1000
	mov	ds, eax
	mov	dword [CurrentOffsetOfKernel], edi
	
	pop	esi
	pop	ds
	pop	edi
	pop	fs
	pop	eax
	pop	cx

	call	Func_Next_Cluster_In_FAT_Entry
	cmp	ax, 0fffh
	jz	Label_File_Loaded
	add	bx, [BPB_BytsPerSec]
	jmp	Label_Load_File

Label_No_Kernel_Found:
	mov	cx, 15
	mov	bp, NoKernelErr
	call	Func_Display_Error_Message

Label_File_Loaded:
	mov	ax, 0B800h
	mov	gs, ax
	mov	ah, 0Fh				; 黑底白字
	mov	al, 'G'
	mov	[gs:((80 * 0 + 39) * 2)], ax	; 屏幕0行39列	

Label_Kill_Motor:
	push	dx
	mov	dx, 03F2h
	mov	al, 0
	out	dx, al		; I/O端口写入命令, 关闭驱动马达
	pop	dx

	mov	ebx, 0
	mov	ax, 0
	mov	es, ax
	mov	di, AddrRangeDescBuffer

; 获取物理地址信息
Label_Query_Physical_Address_Info:
	mov	eax, 0x0E820
	mov	ecx, 20
	mov	edx, 0x534d4150
	int	15h
	jc	Label_Query_Address_Info_Fail
	add	di, 20
	cmp	ebx, 0
	jne	Label_Query_Physical_Address_Info
	jmp	Label_Query_Address_Info_OK

Label_Query_Address_Info_Fail:
	mov	cx, 26
	mov	bp, QueryAddressInfoErr
	call	Func_Display_Error_Message
	jmp	Label_Finish

Label_Query_Address_Info_OK:
	mov	cx, 23
	mov	bp, QueryAddressInfoOK
	call	Func_Display_Message

	jmp	Label_Finish

CurrentOffsetOfKernel	dd OffsetOfKernel

StartLoaderMessage:	db "Start Loader"
NoKernelErr:		db "No KERNEL Found"
QueryAddressInfoErr:	db "Query Address Info [Error]"
QueryAddressInfoOK:	db "Query Address Info [OK]"
KernelFileName:		db "KERNEL  BIN",0
