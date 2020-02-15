//
// Created by 赵明明 on 2019/11/18.
//
#include "driver/disk.h"
#include "driver/HPET.h"
#include "driver/keyboard.h"
#include "driver/mouse.h"
#include "mm/memory.h"
#include "mm/slab.h"
#include "proc/smp.h"
#include "sched/sched.h"
#include "time/time.h"
#include "trap/gate.h"
#include "trap/trap.h"
#include "trap/intr.h"
#include "test.h"

void Start_Kernel() {
    spin_init(&pos.lock);

    test_format_print();

    load_TR(10U);
    setup_TSS(TSS_Table, _stack_start, _stack_start, _stack_start,
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

    test_cpu_info();
    sched_init();
    smp_init();
    test_IPI();

    HPET_init();
    proc_init();

    struct Time time;

    while (1) {
        if (kb_buf->count) {
            analysis_keycode();
        }
        if (mouse_buf->count) {
            get_time(&time);
            print_color(GREEN, BLACK, "%4d-%02d-%02d %02d:%02d:%02d ", time.year, time.month, time.day, time.hour,
                        time.minute, time.second);
            analysis_mousecode();
        }
        pause();
    }
}