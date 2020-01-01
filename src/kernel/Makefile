SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .S .c

bin = ../../bin

all : $(bin)/kernel.bin

$(bin)/head.o $(bin)/head.s : head.S
	gcc -E $< > $(bin)/head.s
	as --64 -o $(bin)/head.o $(bin)/head.s

$(bin)/entry.o $(bin)/entry.s : entry.S
	gcc -E $< > $(bin)/entry.s
	as --64 -o $(bin)/entry.o $(bin)/entry.s

$(bin)/printk.o : lib/printk.c lib/stdio.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/string.o : lib/string.c lib/string.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/trap.o : trap/trap.c trap/trap.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/intr.o : trap/intr.c trap/intr.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/memory.o : mm/memory.c mm/memory.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/slab.o : mm/slab.c mm/slab.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/proc.o : proc/proc.c proc/proc.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/main.o : main.c $(bin)/printk.o $(bin)/string.o test.h
	gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

$(bin)/system : Kernel.lds $(bin)/head.o $(bin)/entry.o $(bin)/main.o $(bin)/printk.o $(bin)/string.o $(bin)/trap.o $(bin)/intr.o $(bin)/memory.o $(bin)/slab.o $(bin)/proc.o
	ld -b elf64-x86-64 -z muldefs -o $@ $(bin)/head.o $(bin)/entry.o $(bin)/main.o $(bin)/printk.o $(bin)/string.o $(bin)/trap.o $(bin)/intr.o $(bin)/memory.o $(bin)/slab.o $(bin)/proc.o -T Kernel.lds

$(bin)/kernel.bin : $(bin)/system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $< $@

.PHONY : clean
clean :
	rm -f $(bin)/*.o $(bin)/system $(bin)/kernel.bin
