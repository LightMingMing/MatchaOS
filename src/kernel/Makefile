SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .S .c

PIC := APIC

bin = ../../bin

objects = $(bin)/head.o $(bin)/entry.o $(bin)/main.o $(bin)/printk.o $(bin)/string.o \
          $(bin)/trap.o $(bin)/intr.o $(bin)/pic.o $(bin)/keyboard.o $(bin)/mouse.o \
          $(bin)/disk.o $(bin)/memory.o $(bin)/slab.o $(bin)/proc.o

gcc = gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@

all : $(bin)/kernel.bin

$(bin)/head.o $(bin)/head.s : head.S
	gcc -E $< > $(bin)/head.s
	as --64 -o $(bin)/head.o $(bin)/head.s

$(bin)/entry.o $(bin)/entry.s : entry.S
	gcc -E $< > $(bin)/entry.s
	as --64 -o $(bin)/entry.o $(bin)/entry.s

$(bin)/printk.o : lib/printk.c lib/stdio.h
	$(gcc)

$(bin)/string.o : lib/string.c lib/string.h
	$(gcc)

$(bin)/trap.o : trap/trap.c trap/trap.h
	$(gcc)

$(bin)/intr.o : trap/intr.c trap/intr.h
	$(gcc) -D$(PIC)

ifeq ($(PIC),APIC)
$(bin)/pic.o: trap/apic.c trap/apic.h
	$(gcc)
else
$(bin)/pic.o: trap/8259A.c trap/8259A.h
	$(gcc)
endif

$(bin)/keyboard.o : driver/keyboard.c driver/keyboard.h
	$(gcc)

$(bin)/mouse.o : driver/mouse.c driver/mouse.h
	$(gcc)

$(bin)/disk.o : driver/disk.c driver/disk.h
	$(gcc)

$(bin)/memory.o : mm/memory.c mm/memory.h lib/stdio.h
	$(gcc)

$(bin)/slab.o : mm/slab.c mm/slab.h
	$(gcc)

$(bin)/proc.o : proc/proc.c proc/proc.h
	$(gcc)

$(bin)/main.o : main.c $(bin)/printk.o $(bin)/string.o test.h
	$(gcc)

$(bin)/system : Kernel.lds $(objects)
	ld -b elf64-x86-64 -z muldefs -o $@ $(objects) -T Kernel.lds

$(bin)/kernel.bin : $(bin)/system
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $< $@

.PHONY : clean
clean :
	rm -f $(bin)/*.o $(bin)/system $(bin)/kernel.bin
