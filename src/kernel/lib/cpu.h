//
// Created by 赵明明 on 2020/1/8.
// https://c9x.me/x86/html/file_module_x86_id_45.html
// https://www.felixcloutier.com/x86/cpuid
//

#ifndef _CPU_H
#define _CPU_H

#include "defs.h"

// cpuid instruction
static inline void
get_cpuid(unsigned int leaf, unsigned int sub_leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx,
          unsigned int *edx) {
    __asm__ __volatile__ ("CPUID":"=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx):"0"(leaf), "2"(sub_leaf));
}

#endif //_CPU_H
