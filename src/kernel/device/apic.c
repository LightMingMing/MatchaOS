//
// Created by 赵明明 on 2020/1/8.
//
#include "../lib/cpu.h"
#include "../lib/stdio.h"

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
}