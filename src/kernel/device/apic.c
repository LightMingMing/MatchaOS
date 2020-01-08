//
// Created by 赵明明 on 2020/1/8.
//
#include "../lib/cpu.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"

void local_APIC_init() {
    unsigned int eax, ebx, ecx, edx;
    get_cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    print_color(WHITE, BLACK, "CPUID.01, eax:%#010x, ebx:%#010x, ecx:%#010x, edx:%010x\n", eax, ebx, ecx, edx);

    if ((1UL << 9UL) & edx)
        print_color(WHITE, BLACK, "Processor support APIC&xAPIC\t");
    else
        print_color(RED, BLACK, "Processor not support APIC&xAPIC\t");

    if ((1UL << 21UL) & ecx)
        print_color(WHITE, BLACK, "Processor support x2APIC\n");
    else
        print_color(RED, BLACK, "Processor not support x2APIC\n");

    // IA32_APIC_BASE MSR (MSR address 1BH)
    unsigned long value = rdmsr(0x1B);
    value = value | (1UL << 11UL) | (1UL << 10UL);
    wrmsr(0x1B, value);
    value = rdmsr(0x1B);
    print_color(WHITE, BLACK, "IA32_APIC_BASE MSR: %#018lx\n", value);

    if (value >> 11UL & 1UL) {
        print_color(WHITE, BLACK, "Enable xAPIC\t");
    } else {
        print_color(RED, BLACK, "Disable xAPIC\t");
    }
    if (value >> 10UL & 1UL) {
        print_color(WHITE, BLACK, "Enable 2xAPIC\n");
    } else {
        print_color(RED, BLACK, "Disable 2xAPIC\n");
    }
}