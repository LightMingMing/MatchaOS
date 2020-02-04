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

/**
 * IA32_APIC_BASE MSR Address
 * bit 8:       BSP flag
 * bit 10:      x2APIC Enable flag if x2APIC is supported
 * bit 11:      APIC Global Enable flag
 * bit 12-35:   APIC Base
 */
#define IA32_APIC_BASE_MSR  0x1B

#define BSP_FLAG_BIT        (8U)
#define x2APIC_EN_BIT       (10U)
#define APIC_GLOBAL_EN_BIT  (11U)

#define x2APIC_ID_MSR       0x802
/**
 * x2APIC Version MSR Address
 * bit 0-7:     Version
 * bit 16-23:   Max LVT Entry
 * bit 24:      Whether or not support suppress EOI-broadcasts
 */
#define x2APIC_VERSION_MSR  0x803
/**
 * Spurious Interrupt Vector MSR
 * bit 8:  APIC Software Enable/Disable 0:Disabled, 1:Enabled
 * bit 12: EOI-Broadcast Suppression 0:Disabled, 1:Enabled
 */
#define EOI_MSR             0x80B
#define LDR_MSR             0x80D
#define SVR_MSR             0x80F
#define LVT_CMCI_MSR        0x82F
#define LVT_TIMER_MSR       0x832
#define LVT_TS_MSR          0x833
#define LVT_PM_MSR          0x834
#define LVT_LINT0_MSR       0x835
#define LVT_LINT1_MSR       0x836
#define LVT_ERROR_MSR       0x837

// Memory Map I/O xAPIC mode
#define APIC_BASE_ADDR      0xFEE00000
#define APIC_ID_REG         0x020
#define APIC_VERSION_REG    0x030
#define EOI_REG             0x0B0
#define LDR                 0x0D0
#define DFR                 0x0E0  /// Destination Format Register
#define SVR                 0x0F0  /// Spurious Interrupt Vector Register
#define LVT_CMCI_REG        0x2F0
#define ICR_LOW             0x300  /// Interrupt Command Register
#define ICR_HIGH            0x310
#define LVT_TIMER_ERG       0x320
#define LVT_TS_REG          0x330
#define LVT_PM_REG          0x340
#define LVT_LINT0_REG       0x350
#define LVT_LINT1_REG       0x360
#define LVT_ERROR_REG       0x370

#define FLAT_MODEL      15
#define CLUSTER_MODEL   0

unsigned int rdmmio(unsigned int reg_offset);

void wrmmio(unsigned int reg_offset, unsigned int value);

unsigned long get_IA32_APIC_BASE();

#endif //_CPU_H
