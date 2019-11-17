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

[SECTION gdt64]

LABEL_GDT64:		dq 0
LABEL_DESC_CODE64:	dq 0x0020980000000000
LABEL_DESC_DATA64:	dq 0x0000920000000000

GdtLen64 equ	$ - LABEL_GDT64
GdtPtr64 dw	GdtLen64 - 1
	 dd	LABEL_GDT64

SelectorCode64	equ LABEL_DESC_CODE64 - LABEL_GDT64
SelectorData64	equ LABEL_DESC_DATA64 - LABEL_GDT64

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

Label_Set_SVGA_Mode:
	mov	ax, 4F02h
	mov	bx, 4180h
	int	10h
	cmp	ax, 004Fh
	jnz	Label_Set_SVGA_Mode_Fail
	jmp	Label_Switch_To_Protect_Mode

Label_Set_SVGA_Mode_Fail:
	mov	cx, 21
	mov	bp, SetSVGAModeErr
	call	Func_Display_Error_Message
	jmp	Label_Finish
	
Label_Switch_To_Protect_Mode:
	cli
	
	db	0x66
	lgdt	[GdtPtr]
	
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	; 长跳转, 触发CS寄存器值的自动修改 
	jmp	dword SelectorCode32:Label_Switch_To_Long_Mode


[SECTION .s32]
[BITS 32]

Label_Switch_To_Long_Mode:
	mov	ax, 0x10
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	ss, ax
	mov	esp, 7E00h

	call	Func_Is_Support_Long_Mode
	test	eax, eax
	jz	Label_Not_Support_Long_Mode

	; 初始化临时页目录项和页表项
	mov	dword [0x90000], 0x91007
	mov	dword [0x90800], 0x91007

	mov	dword [0x91000], 0x92007
	
	mov	dword [0x92000], 0x000083
	mov	dword [0x92008], 0x200083
	mov	dword [0x92010], 0x400083
	mov	dword [0x92018], 0x600083
	mov	dword [0x92020], 0x800083
	mov	dword [0x92028], 0xa00083
	
	; load GDTR
	;db	0x66
	lgdt	[GdtPtr64]

	mov	ax, 0x10
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	mov	esp, 7E00h

	; open PAE(物理地址扩展功能)
	mov	eax, cr4
	bts	eax, 5
	mov	cr4, eax

	; load cr3(临时页目录地址设置到cr3控制寄存器)
	mov	eax, 0x90000
	mov	cr3, eax
	
	; enable long-mode
	mov	ecx, 0C0000080h	; IA32_EFER寄存器位于MSR寄存器组中
	rdmsr			; 读IA32_EFER寄存器值至eax
	bts	eax, 8		; 置位第8位
	wrmsr

	; open PE and pagine
	mov	eax, cr0
	bts	eax, 0
	bts	eax, 31
	mov	cr0, eax

	jmp	SelectorCode64:OffsetOfKernel

Func_Is_Support_Long_Mode:
	; CPUID汇编指令的扩展功能项0x80000001的第29位, 代表是否支持IA-32e模式
	mov	eax, 0x80000000
	cpuid
	cmp	eax, 0x80000001
	setnb	al				; 无进位时(CF=0), 设置al等于1
	jb	Label_Support_Long_Mode_Done	; 比较时, 左边小于右边时(CF=1), 则跳转
	
	mov	eax, 0x80000001
	cpuid
	bt	edx, 29				; 位测试, CF=29位(位从0开始)
	setc	al				; 有进位时置1
Label_Support_Long_Mode_Done:
	movzx	eax, al				; 高位补零
	ret

Label_Not_Support_Long_Mode:
	jmp	$


CurrentOffsetOfKernel	dd OffsetOfKernel

StartLoaderMessage:	db "Start Loader"
NoKernelErr:		db "No KERNEL Found"
QueryAddressInfoErr:	db "Query Address Info [Error]"
QueryAddressInfoOK:	db "Query Address Info [OK]"
SetSVGAModeErr:		db "Set SVGA Mode [Error]"
KernelFileName:		db "KERNEL  BIN",0
