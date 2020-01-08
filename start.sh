#!/bin/bash
bin=bin
matcha_img=${bin}/matcha.img
boot_bin=${bin}/boot.bin
loader_bin=${bin}/loader.bin
kernel_bin=${bin}/kernel.bin

bochs_config=${bin}/bochs_config

# make
cd src/boot && make && cd ../..

# create when image file does not exist
if [[ ! -e ${matcha_img} ]]; then
	bximage -mode=create -fd=1.44M ${matcha_img} -q
fi

dd if=${boot_bin} of=${matcha_img} bs=512 count=1 conv=notrunc >> /dev/null 2>&1

# mount 'loader.bin' for mac
mount_node=$(hdiutil mount ${matcha_img} | sed 's/^[^[:space:]]*[[:space:]]*//')
echo "${mount_node}"
cp ${loader_bin} "${mount_node}"
cp ${kernel_bin} "${mount_node}"
umount "${mount_node}"

if [[ ! -e ${bochs_config} ]]; then
	cat>${bochs_config}<<EOF
romimage: file=/usr/local/share/bochs/BIOS-bochs-latest
vgaromimage: file=/usr/local/share/bochs/VGABIOS-lgpl-latest
floppya: 1_44=${matcha_img}, status=inserted
boot: floppy
megs: 2048
cpuid: x86_64=1, apic=x2apic
cpuid: family=6, model=0x1a, stepping=5, vendor_string="GenuineIntel", brand_string="Intel(R) Core(TM) i7-4770 CPU (Haswell)"
EOF
fi

bochs -f ${bochs_config} -q
