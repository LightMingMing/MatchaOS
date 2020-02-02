//
// Created by 赵明明 on 2020/1/8.
// https://c9x.me/x86/html/file_module_x86_id_45.html
// https://www.felixcloutier.com/x86/cpuid
//

#ifndef _CPU_H
#define _CPU_H

#include "../lib/defs.h"

static inline uint32_t get_cpuid_eax(uint32_t leaf, uint32_t sub_leaf) {
    uint32_t tmp = 0;
    __asm__ __volatile__ ("CPUID":"=a"(tmp):"a"(leaf), "c"(sub_leaf));
    return tmp;
}

static inline uint32_t get_cpuid_ebx(uint32_t leaf, uint32_t sub_leaf) {
    uint32_t tmp = 0;
    __asm__ __volatile__ ("CPUID":"=b"(tmp):"a"(leaf), "c"(sub_leaf));
    return tmp;
}

static inline uint32_t get_cpuid_ecx(uint32_t leaf, uint32_t sub_leaf) {
    uint32_t tmp = 0;
    __asm__ __volatile__ ("CPUID":"=c"(tmp):"a"(leaf), "c"(sub_leaf));
    return tmp;
}

static inline uint32_t get_cpuid_edx(uint32_t leaf, uint32_t sub_leaf) {
    uint32_t tmp = 0;
    __asm__ __volatile__ ("CPUID":"=d"(tmp):"a"(leaf), "c"(sub_leaf));
    return tmp;
}

// cpuid instruction
static inline void
get_cpuid(unsigned int leaf, unsigned int sub_leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx,
          unsigned int *edx) {
    __asm__ __volatile__ ("CPUID":"=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx):"0"(leaf), "2"(sub_leaf));
}


// Get Processor Vendor(公司) String
char *get_vendor(char *vendor);

// Get Processor Brand(品牌) String
char *get_brand(char *brand);

// Whether support Hardware Multi-Threading or not
// CPUID.1:EDX[28]
unsigned int HWMT_supported();

// Maximum input value for basic CPUID information
// CPUID.0:EAX
unsigned int max_input_for_basic_CPUID();

// CPUID.1:EDX[9]
unsigned int xAPIC_supported();

// CPUID.1:ECX[21]
unsigned int x2APIC_supported();

#endif //_CPU_H
