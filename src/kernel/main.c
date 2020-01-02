//
// Created by 赵明明 on 2019/11/18.
//
#include "trap/gate.h"
#include "trap/trap.h"
#include "trap/intr.h"
#include "mm/memory.h"
#include "mm/slab.h"
#include "proc/proc.h"
#include "test.h"

void Start_Kernel() {

    test_format_print();

    load_TR(10U);
    setup_TSS(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);
    trap_init();
//    test_DE();
//    test_read_PF();
//    test_write_PF();

    memory_init();
    test_mem_info();
    test_get_CR3();
    test_alloc_pages(64);

    slab_init();
    test_kmalloc();
    test_create_and_destroy_slab_cache();

    intr_init();

    proc_init();

    __asm__ __volatile__ ("hlt":: :);
}