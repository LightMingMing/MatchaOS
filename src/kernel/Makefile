SHELL = /bin/bash
.SUFFIXES:
.SUFFIXES: .S .c

PIC := APIC

bin = ../../bin

objects = $(bin)/head.o $(bin)/entry.o $(bin)/main.o $(bin)/printk.o $(bin)/string.o \
          $(bin)/trap.o $(bin)/intr.o $(bin)/pic.o $(bin)/softirq.o	\
          $(bin)/keyboard.o $(bin)/mouse.o $(bin)/HPET.o $(bin)/disk.o \
          $(bin)/mem.o $(bin)/slab.o \
          $(bin)/cpu.o $(bin)/proc.o $(bin)/smp.o $(bin)/apu_boot.o $(bin)/sched.o

gcc = gcc -mcmodel=large -fno-builtin -m64 -c $< -o $@ -g

all : $(bin)/kernel.bin

$(bin)/head.o $(bin)/head.s : head.S
	gcc -E $< > $(bin)/head.s
	as --64 -o $(bin)/head.o $(bin)/head.s

$(bin)/entry.o $(bin)/entry.s : entry.S
	gcc -E $< > $(bin)/entry.s
	as --64 -o $(bin)/entry.o $(bin)/entry.s

$(bin)/apu_boot.o $(bin)/apu_boot.s : proc/APU_boot.S
	gcc -E $< > $(bin)/apu_boot.s
	as --64 -o $(bin)/apu_boot.o $(bin)/apu_boot.s

$(bin)/printk.o : lib/printk.c lib/stdio.h
	$(gcc)

$(bin)/string.o : lib/string.c lib/string.h
	$(gcc)

$(bin)/trap.o : trap/trap.c trap/trap.h
	$(gcc)

$(bin)/intr.o : trap/intr.c trap/intr.h
	$(gcc) -D$(PIC)

$(bin)/softirq.o : trap/softirq.c trap/softirq.h
	$(gcc)

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

$(bin)/HPET.o : driver/HPET.c driver/HPET.h
	$(gcc)

$(bin)/mem.o : mm/mem.c mm/mem.h lib/stdio.h
	$(gcc)

$(bin)/slab.o : mm/slab.c mm/slab.h
	$(gcc)

$(bin)/cpu.o : proc/cpu.c proc/cpu.h
	$(gcc)

$(bin)/proc.o : proc/proc.c proc/proc.h
	$(gcc)

$(bin)/smp.o : proc/smp.c proc/smp.h
	$(gcc)

$(bin)/sched.o : sched/sched.c sched/sched.h
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
