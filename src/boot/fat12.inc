; ============================================
;		1.44M软盘结构
; ============================================
; |---------------------|
; |2879                 |
; |                     |
; |        Data         |
; |                     |
; |33                   |
; |---------------------|
; |32                   |
; |                     |
; |      Root Dir       |
; |        (14)         |
; |19                   |
; |---------------------|
; |18                   |
; |        FAT2         |
; |10                   |
; |---------------------|
; |9                    |
; |        FAT1         |
; |1                    |
; |---------------------|
; |0    Boot Sector     |
; |---------------------|

; Root Dir structure
; ---------------------------------------------------------------------------------------
; |0       10|    11    |12  21|22         23|24         25|26         27|28          31|
; | Dir_Name | DIR_Attr | RSVD | DIR_WrtTime | DIR_WrtDate | DIR_FstClus | DIR_FileSize |
; ---------------------------------------------------------------------------------------
;
; FAT structure
; -------------
; |0        11|
; | FAT Entry |	Value 0x002~0xFEF, 已用簇, 下一簇号标识
; -------------

RootSecCnt		equ 14	; 根目录扇区数224*32/512
FirstFAT1SecNum		equ 1	; FAT1开始扇区号
FirstRootDirSecNum	equ 19	; 根目录开始扇区号
FirstDataSecNum		equ 33	; Data开始扇区号
FirstDataClus		equ 2	; Data开始扇区所在簇号为2
EntPerSec		equ 16	; 每扇区目录数512/32=16

; BPB(BIOS Parameter Block) structure common to FAT12, FAT16 and FAT32
jmp Label_Boot
;nop
BS_OEMName	db 'MatchaOS'	; 厂商名, 8-byte
BPB_BytsPerSec	dw 512		; 每扇区512-byte
BPB_SecPerClus	db 1		; 每簇扇区数(2的次方数)
BPB_RsvdSecCnt	dw 1		; 保留扇区数(非0数)
BPB_NumFATs	db 2		; 文件分配表(FATs)个数
BPB_RootEntCnt	dw 224		; 根目录目录项(32-byte)数
BPB_TotSec16	dw 2880		; 总扇区数(FAT12, FAT16), 在FAT32该值为0
BPB_Media	db 0xF0		; 0xF0可移动介质(removable media)
BPB_FATSz16	dw 9		; 一个FAT扇区数
BPB_SecPerTrk	dw 18		; Int 0x13 每磁道扇区数
BPB_NumHeads	dw 2		; Int 0x13 磁头数
BPB_HiddSec	dd 0		; 隐藏扇区数
BPB_TotSec32	dd 0		; 总扇区数(FAT12, FAT16扇区数大于0xffff时,该属性生效)

; Extended BPB structure for FAT12 and FAT16
BS_DrvNum	db 0		; Int 0x13 驱动号(值为0x00或0x80)
BS_Reserved1	db 0		; 保留
BS_BootSig	db 0x29		; 扩展引导标记
BS_VollID	dd 0		; 卷序列号
BS_VolLab	db 'Boot Loader'; 卷标11-byte
BS_FilSysType	db 'FAT12   '	; FAT类型


; temp variable
DisMsgLineNum	db 0			; 消息行号标记
Index		dw 0			; 索引
Odd		db 0			; 奇偶标志

; message
ReadSecErr:	db "Read Sector Error!"	; 18-byte

[BITS 16]

; 借助于int 13h中断服务程序读取扇区
Func_Read_Sector:
	; ax起始扇区号(从0开始)
	; cl读入扇区数
	; es:bx 目标缓冲区
	push	bp
	mov	bp, sp
	sub	esp, 2
	mov	byte [bp-2], cl
	push	bx

	mov	bl, [BPB_SecPerTrk]
	div	bl		; ax/bl = al ... ah
	inc	ah
	mov	cl, ah		; cl 磁道内起始扇区号(从1开始)
	mov	ch, al
	shr	ch, 1		; ch 磁道号 (al>>1)
	mov	dh, al

	and	dh, 1		; dh 磁头号 (ah&1)	
	mov	dl, [BS_DrvNum] ; dl 驱动器号

	mov	ah, 2		; ah=02h读取磁盘功能
	mov	al, byte [bp-2]	; al 读入扇区数

	pop	bx		; es:bx目标缓冲区

	int	13h
	jc	Label_Read_Sector_Error ; cf=1磁盘读取错误
	add	esp, 2
	pop	bp
	ret

Label_Read_Sector_Error:
	mov	cx, 18
	mov	bp, ReadSecErr
	call	Func_Display_Error_Message
	jmp	Label_Finish


Func_Search_File_In_Root_Dir:
	; ax 输入文件名地址/返回起始簇号
	push	bp
	mov	bp, sp
	sub	esp, 2
	mov	[bp-2], ax
	mov	word [Index], 0
Label_Search_In_Next_Sector:
	cmp	word [Index], RootSecCnt
	je	Label_Not_Found
	mov	ax, 0
	mov	es, ax
	mov	bx, 8000h
	add	ax, FirstRootDirSecNum
	add	ax, [Index]
	mov	cl, 1
	call	Func_Read_Sector
	mov	si, [bp-2]
	mov	di, 8000h
	cld			; 清除方向标记,si,di向前移动
	mov	dx, 0

Label_Search_In_Next_Entry:
	cmp	dx, EntPerSec
	je	Label_Ready_Goto_Next_Sector
	inc	dx
	mov	cx, 0

Label_Cmp_FileName:
	cmp	cx, 11		; 文件名+扩展名 11-byte
	je	Label_Found
	inc	cx
	lodsb			; si指向的单元读入累加器al, 然后si++
	cmp	al, byte [es:di]
	je	Label_Cmp_Next_Char
	and	di, 0ffe0h	; 当前目录项, 一个目录项 32-byte
	add	di, 32
	mov	si, [bp-2]
	jmp	Label_Search_In_Next_Entry

Label_Cmp_Next_Char:
	inc	di
	jmp	Label_Cmp_FileName


Label_Ready_Goto_Next_Sector:
	inc	word [Index]
	jmp	Label_Search_In_Next_Sector

	
Label_Found:
	and	di, 0ffe0h	; 当前目录项
	add	di, 26		; 起始簇号
	mov	ax, word [es:di]
	jmp	Label_Search_File_Finish

Label_Not_Found:	
	mov	ax, 0

Label_Search_File_Finish:
	add	esp, 2
	pop	bp
	ret

Func_Next_Cluster_In_FAT_Entry:
	; ax 输入当前簇号/输出下一簇号
	push	es
	push	bx
	push	ax
	mov	ax, 0
	mov	es, ax
	pop	ax
	mov	byte [Odd], 0
	mov	bx, 3
	mul	bx
	mov	bx, 2
	div	bx
	cmp	dx, 0
	jz	Label_Read_Sectors
	mov	byte [Odd], 1
Label_Read_Sectors:
	xor	dx, dx
	mov	bx, [BPB_BytsPerSec]
	div	bx
	push	dx
	add	ax, FirstFAT1SecNum
	mov	cl, 2
	mov	bx, 8000h
	call	Func_Read_Sector
	
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	
	cmp	byte [Odd], 0
	jz	Label_Even
	shr	ax, 4
Label_Even:
	and	ax, 0fffh
	pop	bx
	pop	es
	ret


Func_Display_Error_Message:
	mov	bx, 000ch
	jmp	Display_Message
Func_Display_Message:
	mov	bx, 000fh
Display_Message:
	mov	ax, ds
	mov	es, ax
	mov	ax, 1301h
	mov	dx, 0
	mov	dh, byte [DisMsgLineNum]
	int	10h
	add	byte [DisMsgLineNum], 2
	ret

Label_Finish:
	hlt

Label_Boot:
