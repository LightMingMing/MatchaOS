//
// Created by 赵明明 on 2020/1/8.
//
#include "../lib/cpu.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "gate.h"
#include "intr.h"

void local_apic_init() {
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

    if (value >> 11UL & 1UL)
        print_color(WHITE, BLACK, "Enable xAPIC\t");
    else
        print_color(RED, BLACK, "Disable xAPIC\t");

    if (value >> 10UL & 1UL)
        print_color(WHITE, BLACK, "Enable x2APIC\n");
    else
        print_color(RED, BLACK, "Disable x2APIC\n");

    // Spurious interrupt vector register
    value = rdmsr(0x80F);
    // bit 8:  APIC Software Enable/Disable 0:Disabled, 1:Enabled
    // bit 12: EOI-Broadcast Suppression 0:Disabled, 1:Enabled
    //  value = value | (1UL << 8UL) | (1UL << 12UL); TODO support EOI-Broadcast suppression
    value = value | (1UL << 8UL);
    wrmsr(0x80F, value);
    value = rdmsr(0x80F);
    print_color(RED, BLACK, "%#018lx\n", value);
    if (value >> 8UL & 1UL)
        print_color(WHITE, BLACK, "APIC Software Enabled\t");
    else
        print_color(RED, BLACK, "APIC Software Disabled\t");

    if (value >> 12UL & 1UL)
        print_color(WHITE, BLACK, "EOI-Broadcast Suppression Enabled\n");
    else
        print_color(RED, BLACK, "EOI-Broadcast Suppression Disabled\n");

    // x2APIC ID
    // MSR address: 0x802
    value = rdmsr(0x802);
    print_color(WHITE, BLACK, "x2APIC ID: %#010x\n", value);

    // x2APIC version
    // MSR address: 0x803
    value = rdmsr(0x803);
    // Version: bit 0-7
    print_color(WHITE, BLACK, "Version: %#02x\t", value & 0xFFU);
    if ((value & 0xFFU) < 0x10)
        print_color(WHITE, BLACK, "82489DX discrete APIC\n");
    else if ((value & 0xFFU) < 0x15)
        print_color(WHITE, BLACK, "Integrated APIC\n");
    else
        print_color(WHITE, BLACK, "Reserved\n");
    // Max LVT Entry: bit 16-23
    print_color(WHITE, BLACK, "Max LVT Entry: %#02x\n", (value >> 16UL) & 0xFFU);
    // Suppress EOI-broadcasts: bit 24
    // Indicates whether software can inhibit the broadcast of EOI message
    // by setting bit 12 of the Spurious Interrupt Vector Register.
    if (value >> 24UL & 1UL)
        print_color(WHITE, BLACK, "Support EOI-Broadcast suppression\n");
    else
        print_color(RED, BLACK, "Not support EOI-Broadcast suppression\n");

    // Mask all interrupts in LVT
    value = 0x10000;
    // wrmsr(0x82F, value); TODO CMCI, not support in current processor
    wrmsr(0x832, value);
    wrmsr(0x833, value);
    wrmsr(0x834, value);
    wrmsr(0x835, value);
    wrmsr(0x836, value);
    wrmsr(0x837, value);
}

void apic_init() {
    for (int i = 0; i < 24; i++) {
        set_intr_gate(0x20 + i, 2, interrupt[i]);
    }
    // Mask all interrupts of 8259A
    io_out8(0x21, 0xff);
    io_out8(0xa1, 0xff);

    // local_apic_init();

    sti();
}

void handle_IRQ(unsigned long intr_vector, unsigned long rsp) {
    print_color(RED, BLACK, "handle_IRQ:%#08x\t", intr_vector);
}