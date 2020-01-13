SHELL = /bin/bash

bin = ../bin

all : subsystem $(bin)/matcha.img

subsystem :
	make -C boot
	make -C kernel

$(bin)/matcha.img : $(bin)/boot.bin $(bin)/loader.bin $(bin)/kernel.bin
	if [[ ! -e $@ ]]; then dd if=/dev/zero of=$@ bs=512 count=2880; fi
	dd if=$(bin)/boot.bin of=$@ bs=512 count=1 conv=notrunc >> /dev/null 2>&1
	mount $@ /media/ -t vfat -o loop
	cp $(bin)/loader.bin /media/
	cp $(bin)/kernel.bin /media/
	umount /media/

.PHONY : clean
clean :
	rm -f $(bin)/*.o $(bin)/system $(bin)/*.bin
