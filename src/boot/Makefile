SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .asm

bin = ../../bin

all : $(bin)/boot.bin

$(bin)/boot.bin : boot.asm fat12.inc
	nasm $< -o $@

.PHONY : clean
clean :
	rm $(bin)/*.bin
