#!/bin/bash
bin=bin
matcha_img=${bin}/matcha.img
bochs_config=${bin}/bochs_config

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
