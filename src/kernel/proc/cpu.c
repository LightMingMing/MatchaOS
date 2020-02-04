//
// Created by 赵明明 on 2020/2/2.
//

#include "cpu.h"
#include "../lib/string.h"
#include "../lib/x86.h"

char *get_vendor(char *vendor) {
    uint32_t eax, ebx, ecx, edx;

    get_cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    *(uint32_t *) &vendor[0] = ebx; // eg. Genu
    *(uint32_t *) &vendor[4] = edx; // eg. ineI
    *(uint32_t *) &vendor[8] = ecx; // eg. ntel
    vendor[12] = '\0';

    return vendor;
}

char *get_brand(char *brand) {
    uint32_t eax, ebx, ecx, edx;

    for (int i = 0; i < 3; i++) {
        get_cpuid(0x80000002 + i, 0, &eax, &ebx, &ecx, &edx);
        *(uint32_t *) &brand[i * 16 + 0] = eax;
        *(uint32_t *) &brand[i * 16 + 4] = ebx;
        *(uint32_t *) &brand[i * 16 + 8] = ecx;
        *(uint32_t *) &brand[i * 16 + 12] = edx;
    }
    brand[48] = '\0';

    return brand;
}

unsigned int HWMT_supported() {
    char vendor[13];
    // Only Genuine Intel Processor support HWMT
    return (!strcmp(get_vendor(vendor), "GenuineIntel")) && (get_cpuid_edx(1, 0) >> 28U & 0x1U);
}

unsigned int max_input_for_basic_CPUID() {
    return get_cpuid_eax(0, 0);
}

unsigned int xAPIC_supported() {
    return get_cpuid_edx(1, 0) >> 9U & 0x1U;
}

unsigned int x2APIC_supported() {
    return get_cpuid_ecx(1, 0) >> 21U & 0x1U;
}

inline unsigned long get_IA32_APIC_BASE() {
    return rdmsr(IA32_APIC_BASE_MSR);
}