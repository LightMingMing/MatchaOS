org 10000h

%include "fat12.inc"

Label_Start:
	mov	ax, 1301h
	mov	bx, 000fh
	mov	cx, 12
	mov	dx, 0100h
	push	ax,
	mov	ax, ds
	mov	es, ax
	pop	ax
	mov	bp, StartLoaderMessage
	
	jmp	Label_Finish

StartLoaderMessage:	db "Loader Start"
