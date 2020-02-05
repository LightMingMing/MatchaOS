//
// Created by 赵明明 on 2019/11/18.
//
#include "trap/gate.h"
#include "trap/trap.h"
#include "trap/intr.h"
#include "mm/memory.h"
#include "mm/slab.h"
#include "test.h"
#include "driver/keyboard.h"
#include "driver/mouse.h"
#include "driver/disk.h"
#include "proc/smp.h"

void Start_Kernel() {

    test_format_print();

    load_TR(10U);
    setup_TSS(TSS_Table, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00);
    trap_init();
//    test_DE();
//    test_read_PF();
//    test_write_PF();

    memory_init();
    slab_init();

    test_mem_info();
    test_get_CR3();

    frame_buffer_init();
    page_table_init();

    test_alloc_pages(64);

    test_kmalloc();
    test_create_and_destroy_slab_cache();

    intr_init();
    keyboard_init();
    mouse_init();
    disk_init();
    test_disk();

//    proc_init();
    test_cpu_info();
    smp_init();

    while (1) {
        if (kb_buf->count) {
            analysis_keycode();
        }
        if (mouse_buf->count) {
            analysis_mousecode();
        }
        hlt();
    }
}