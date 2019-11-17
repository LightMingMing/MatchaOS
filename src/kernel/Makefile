SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .S .c

bin = ../../bin

all : $(bin)/kernel.bin

$(bin)/head.o $(bin)/head.s : head.S
	gcc -E $< > $(bin)/head.s
	as --64 -o $(bin)/head.o $(bin)/head.s

$(bin)/main.o : main.c
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/system : Kernel.lds $(bin)/head.o $(bin)/main.o
	ld -b elf64-x86-64 -z muldefs -o $@ $(bin)/head.o $(bin)/main.o -T Kernel.lds

$(bin)/kernel.bin : $(bin)/system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $< $@

.PHONY : clean
clean :
	rm $(bin)/*.bin
