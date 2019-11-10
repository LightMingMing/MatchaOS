; test, temp
org 100000h	; 1M

; 多个扇区加载测试, kernel文件大小大于10个扇区
times	5120 db 0
