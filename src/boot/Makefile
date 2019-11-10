SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .asm

bin = ../../bin

all : $(bin)/boot.bin $(bin)/loader.bin $(bin)/kernel.bin

$(bin)/boot.bin : boot.asm fat12.inc
	nasm $< -o $@

$(bin)/loader.bin : loader.asm fat12.inc
	nasm $< -o $@

$(bin)/kernel.bin : kernel.asm
	nasm $< -o $@

.PHONY : clean
clean :
	rm $(bin)/*.bin
