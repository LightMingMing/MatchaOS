//
// Created by 赵明明 on 2019/11/20.
//

#ifndef _X86_H
#define _X86_H

#define do_div(num, base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (num),"=d" (__res):"0" (num),"1" (0),"c" (base)); \
__res; })

#endif //_X86_H
