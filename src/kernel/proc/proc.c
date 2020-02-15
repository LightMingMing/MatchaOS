//
// Created by 赵明明 on 2019/12/17.
//

#include "proc.h"
#include "../mm/slab.h"
#include "../sched/sched.h"

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

extern void ret_system_call();

extern void system_call();

unsigned long init(unsigned long args) {
    regs_t *regs;
    struct proc_struct *current = get_current();

    print_color(YELLOW, BLACK, "init proc is running, args:%#018lx\n", args);

    current->flags = 0;
    current->ctx->rip = (unsigned long) ret_system_call;
    current->ctx->rsp = (unsigned long) current + PROC_STACK_SIZE - sizeof(regs_t);
    regs = (regs_t *) current->ctx->rsp;

    __asm__ __volatile__("movq %1, %%rsp    \n\t"
                         "pushq %2          \n\t"
                         "jmp do_execve     \n\t"
    ::"D"(regs), "m"(current->ctx->rsp), "m"(current->ctx->rip):"memory");
    return 1;
}

void user_level_func() {
    long ret = 0;
    char *str = "Hello, World!\n";

    __asm__ __volatile__ ("leaq sysexit_ret_addr(%%rip), %%rdx  \n\t"
                          "movq %%rsp, %%rcx    \n\t"
                          "sysenter             \n\t"
                          "sysexit_ret_addr:    \n\t"
    :"=a"(ret):"0"(1), "D"(str):"memory");
    while (1) {
        pause();
    }
}

unsigned long system_call_func(regs_t *regs) {
    return system_call_table[regs->rax](regs);
}

unsigned long do_execve(regs_t *regs) {
    struct Page *page = NULL;
    unsigned long phy_addr = 0x800000;

    regs->rdx = 0x800000; // rip
    regs->rcx = 0xa00000; // rsp
    regs->rax = 1;
    regs->ds = 0;
    regs->es = 0;
    print_color(YELLOW, BLACK, "do_execve is running\n");

    unsigned long *pml4t = NULL, *pdpt = NULL, *pdt = NULL; // base
    unsigned long *pml4e = NULL, *pdpe = NULL, *pde = NULL; // base + offset
    unsigned long *addr = NULL;

    pml4t = phy_to_vir(get_CR3());
    pml4e = pml4t + pml4t_off(phy_addr);
    addr = kmalloc(PAGE_SIZE_4K);
    set_pml4t(pml4e, mk_pml4t(vir_to_phy(addr), PAGE_USER_PML4T));

    pdpt = phy_to_vir(*pml4e & (~0xFFUL));
    pdpe = pdpt + pdpt_off(phy_addr);
    addr = kmalloc(PAGE_SIZE_4K);
    set_pdpt(pdpe, mk_pdpt(vir_to_phy(addr), PAGE_USER_PDPT));

    pdt = phy_to_vir(*pdpe & (~0xFFUL));
    pde = pdt + pdt_off(phy_addr);
    page = alloc_pages(1, PG_PTable_Mapped);
    set_pdt(pde, mk_pdt(page->phy_addr, PAGE_USER_PDT));

    flush_TLB();
    memcpy(user_level_func, (void *) 0x800000, 1024);
    return 0;
}

void do_exit(unsigned long args) {
    print_color(YELLOW, BLACK, "exit proc is running, args:%#018lx\n", args);
    while (1);
}

extern void kernel_thread_func(void);

__asm__ (
"kernel_thread_func:"
"popq	%r15            \n\t"
"popq	%r14            \n\t"
"popq	%r13            \n\t"
"popq	%r12            \n\t"
"popq	%r11            \n\t"
"popq	%r10            \n\t"
"popq	%r9             \n\t"
"popq	%r8             \n\t"
"popq	%rbx            \n\t"
"popq	%rcx            \n\t"
"popq	%rdx            \n\t"
"popq	%rsi            \n\t"
"popq	%rdi            \n\t"
"popq	%rbp            \n\t"
"popq	%rax            \n\t"
"movq	%rax, %ds       \n\t"
"popq	%rax            \n\t"
"movq	%rax, %es       \n\t"
"popq	%rax            \n\t"
"addq   $0x38, %rsp     \n\t"
"movq   %rdx, %rdi      \n\t"
"callq  *%rbx           \n\t"
"movq   %rax, %rdi      \n\t"
"callq  do_exit         \n\t"
);

int kernel_thread(unsigned long (*entry)(unsigned long), unsigned long arg, unsigned long flags) {
    regs_t regs;
    memset(&regs, 0, sizeof(regs_t));

    regs.rbx = (unsigned long) entry;
    print_color(YELLOW, BLACK, "rbx: %#018lx\n", regs.rbx);
    regs.rdx = arg;

    regs.ds = KERNEL_DS;
    regs.es = KERNEL_DS;
    regs.cs = KERNEL_CS;
    regs.ss = KERNEL_DS;
    regs.rflags = (1UL << 9UL);
    regs.rip = (unsigned long) kernel_thread_func;
    return do_fork(&regs, flags, 0, 0);
}

void proc_init() {
    struct proc_struct *next = NULL;
    struct proc_struct *current = get_current();

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
    println("get_current():   %#018lx", current);
    println("init proc union: %#018lx", &init_proc_union);
    println("union.proc:      %#018lx", &init_proc_union.proc);
    println("&union.proc.ctx: %#018lx", &(init_proc_union.proc.ctx));
    println("&union.proc.mm:  %#018lx", &(init_proc_union.proc.mm));
    println("union.proc.ctx:  %#018lx", (init_proc_union.proc.ctx));
    println("union.proc.mm:   %#018lx", (init_proc_union.proc.mm));
    println("union.stack:     %#018lx", init_proc_union.stack);
    println("init_ctx:        %#018lx", &init_ctx);
    println("init_mm:         %#018lx", &init_mm);

    wrmsr(0x174, KERNEL_CS);
    wrmsr(0x175, current->ctx->rsp0);
    wrmsr(0x176, (unsigned long) system_call);
    setup_TSS(TSS_Table, init_ctx.rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2,
              init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);
    init_tss[0].rsp0 = init_ctx.rsp0;

    list_init(&init_proc_union.proc.list);
    kernel_thread(init, 0, 0);
    init_proc_union.proc.state = PROC_RUNNABLE;

//    next = container_of(list_next(&sched_queue.head_proc.list), struct proc_struct, list);
//    switch_to(current, next);
}

int do_fork(regs_t *regs, unsigned long flags, unsigned long stack_start, unsigned long stack_end) {
    struct proc_struct *proc = NULL;
    struct proc_ctx *ctx = NULL;
    struct Page *page = NULL;

    print_color(WHITE, BLACK, "bitmap:%#018lx\n", *mem_info.bit_map);
    page = alloc_pages(1, PG_PTable_Mapped | PG_Kernel);
    print_color(WHITE, BLACK, "bitmap:%#018lx\n", *mem_info.bit_map);

    proc = (struct proc_struct *) phy_to_vir(page->phy_addr);
    memset(proc, 0, sizeof(struct proc_struct));
    *proc = *get_current();

    list_init(&proc->list);

    proc->pid++;
    proc->priority = 2;
    proc->state = PROC_UNINTERRUPTIBLE;

    ctx = (struct proc_ctx *) (proc + 1);
    memcpy(regs, (void *) ((unsigned long) proc + PROC_STACK_SIZE - sizeof(regs_t)), sizeof(regs_t));

    ctx->rsp0 = (unsigned long) proc + PROC_STACK_SIZE;
    ctx->rip = regs->rip;
    ctx->rsp = (unsigned long) proc + PROC_STACK_SIZE - sizeof(regs_t);
    proc->ctx = ctx;

    proc->state = PROC_RUNNABLE;
    insert(proc);

    return 1;
}