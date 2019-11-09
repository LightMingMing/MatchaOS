org 10000h

%include "fat12.inc"

jmp	Label_Start

; 多个扇区加载测试, loader文件大小大于10个扇区
times	10240 db 0

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

	jmp	Label_Finish

StartLoaderMessage:	db "Start Loader"
