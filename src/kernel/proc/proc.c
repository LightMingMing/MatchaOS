//
// Created by 赵明明 on 2019/12/17.
//

#include "proc.h"
#include "../lib/x86.h"

void display_mm_struct(struct mm_struct mm) {
    print_color(GREEN, BLACK, "Display mm_struct info\n");
    println("code:   [%#018lx, %#018lx)", mm.start_code, mm.end_code);
    println("data:   [%#018lx, %#018lx)", mm.start_data, mm.end_data);
    println("rodata: [%#018lx, %#018lx)", mm.start_rodata, mm.end_rodata);
    println("brk:    [%#018lx, %#018lx)", mm.start_brk, mm.end_brk);
    println("start_stack: %#018lx", mm.start_stack);
    println("pml4t:       %#018lx", mm.pml4t);
}

void display_proc_ctx(struct proc_ctx ctx) {
    print_color(GREEN, BLACK, "Display proc_ctx info\n");
    println("ctx.rsp0: %#018lx", ctx.rsp0);
    println("ctx.rsp:  %#018lx", ctx.rsp);
    println("ctx.fs:   %#018lx", ctx.fs);
    println("ctx.gs:   %#018lx", ctx.gs);
}

void display_tss_struct(struct tss_struct tss) {
    print_color(GREEN, BLACK, "Display tss_struct info\n");
    println("tss.rsp0: %#018lx", tss.rsp0);
    println("tss.rsp1: %#018lx", tss.rsp1);
    println("tss.rsp2: %#018lx", tss.rsp2);
    println("tss.ist1: %#018lx", tss.ist1);
    println("tss.ist2: %#018lx", tss.ist2);
}

unsigned long init(unsigned long args) {
    print_color(GREEN, BLACK, "init proc is running, args:%#018lx", args);
    return 1;
}

void proc_init() {
    struct proc_struct *next = NULL;

    init_mm.pml4t = (pml4t_t *) get_CR3();
    init_mm.start_code = mem_info.start_code;
    init_mm.end_code = mem_info.end_code;
    init_mm.start_data = (unsigned long) &_data;
    init_mm.end_data = mem_info.end_data;
    init_mm.start_rodata = (unsigned long) &_rodata;
    init_mm.end_rodata = (unsigned long) &_erodata;
    init_mm.start_brk = 0;
    init_mm.end_brk = mem_info.end_brk;
    init_mm.start_stack = _stack_start;

    display_mm_struct(init_mm);
    display_proc_ctx(init_ctx);
    display_tss_struct(init_tss[0]);
    println("get_current():   %#018lx", get_current());
    println("init proc union: %#018lx", &init_proc_union);
    println("union.proc:      %#018lx", &init_proc_union.proc);
    println("&union.proc.ctx: %#018lx", &(init_proc_union.proc.ctx));
    println("&union.proc.mm:  %#018lx", &(init_proc_union.proc.mm));
    println("union.proc.ctx:  %#018lx", (init_proc_union.proc.ctx));
    println("union.proc.mm:   %#018lx", (init_proc_union.proc.mm));
    println("union.stack:     %#018lx", init_proc_union.stack);
    println("init_ctx:        %#018lx", &init_ctx);
    println("init_mm:         %#018lx", &init_mm);


    setup_TSS(init_ctx.rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2,
              init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);
    init_tss[0].rsp0 = init_ctx.rsp0;

    list_init(&init_proc_union.proc.list);

    // switch_to(current, next);
}