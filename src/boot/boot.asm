; BIOS加电自检后, 跳转至0x7c00执行boot引导程序
org	0x7c00

BaseOfStack equ	0x7c00

Label_Start:
	; 各个段寄存器值对齐
	mov	ax, cs			; cs=0x0000, ip=0x7c00
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack		; 栈顶指针指向0x7c00处

	; 清屏
	mov	ax, 0600h
	mov	cx, 0
	mov	dx, 0184fh
	int	10h

	; 设置光标位置
	mov	ax, 0200h
	mov	bx, 0
	mov	dx, 0
	int	10h
	

	; 显示"Start Boot"
	mov	ax, 1301h
	mov	bx, 000fh
	mov	cx, 10
	push	ax
	mov	ax, ds
	mov	es, ax
	pop	ax
	mov	bp, StartBootMessage	; es:bp指向要显示的字符串的地址
	int	10h


Label_Finish:
	hlt


StartBootMessage: db "Start Boot"

times	510 - ($ - $$) db 0
dw	0xaa55	; 倒数2个字节分别为0xaa, 0x55. (BIOS会进行检查)

