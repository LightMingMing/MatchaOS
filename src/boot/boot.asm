; BIOS加电自检后, 跳转至0x7c00执行boot引导程序
org	0x7c00

BaseOfStack	equ 0x7c00
BaseOfLoader	equ 0x1000
OffsetOfLoader	equ 0x00

%include "fat12.inc"

Label_Start:
	; 各个段寄存器值对齐
	mov	ax, cs			; cs=0x0000, ip=0x7c00
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack		; 栈顶指针指向0x7c00处

	; 清屏
	mov	ax, 0600h
	mov	bx, 0700h
	mov	cx, 0
	mov	dx, 0184fh
	int	10h

	; 设置光标位置
	;mov	ax, 0200h
	;mov	bx, 0
	;mov	dx, 0
	;int	10h
	
	; 显示"Start Boot"
	mov	cx, 10
	mov	bp, StartBootMessage
	call	Func_Display_Message

	; 搜索 "Loader.bin"
	mov	ax, LoaderFileName
	call	Func_Search_File_In_Root_Dir
	cmp	ax, 0
	je	Label_No_LoaderBin

	; 将Loader.bin加载至内存0x10000处	
	mov	bx, BaseOfLoader
	mov	es, bx
	mov	bx, OffsetOfLoader
	
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
	call	Func_Next_Cluster_In_FAT_Entry
	cmp	ax, 0fffh
	jz	Label_File_Loaded
	add	bx, [BPB_BytsPerSec]
	jmp	Label_Load_File

Label_File_Loaded:
	; 跳转至loader
	jmp	BaseOfLoader:OffsetOfLoader

Label_No_LoaderBin:
;	mov     cx, 15
;	mov	bp, NoLoaderErr
;	call	Func_Display_Error_Message
	jmp	Label_Finish

StartBootMessage:	db "Start Boot"
NoLoaderErr:		db "No LOADER Found"
LoaderFileName:		db "LOADER  BIN",0

times	510 - ($ - $$) db 0
dw	0xaa55	; 倒数2个字节分别为0xaa, 0x55. (BIOS会进行检查)
