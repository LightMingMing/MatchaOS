SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .S .c

bin = ../../bin

all : $(bin)/kernel.bin

$(bin)/head.o $(bin)/head.s : head.S
	gcc -E $< > $(bin)/head.s
	as --64 -o $(bin)/head.o $(bin)/head.s

$(bin)/printk.o : lib/printk.c lib/stdio.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/string.o : lib/string.c lib/string.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/main.o : main.c $(bin)/printk.o $(bin)/string.o
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/system : Kernel.lds $(bin)/head.o $(bin)/main.o $(bin)/printk.o $(bin)/string.o
	ld -b elf64-x86-64 -z muldefs -o $@ $(bin)/head.o $(bin)/main.o $(bin)/printk.o $(bin)/string.o -T Kernel.lds

$(bin)/kernel.bin : $(bin)/system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $< $@

.PHONY : clean
clean :
	rm $(bin)/*.o $(bin)/system $(bin)/kernel.bin
