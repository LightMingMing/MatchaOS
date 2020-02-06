//
// Created by 赵明明 on 2020/2/2.
//

#include "smp.h"
#include "cpu.h"
#include "proc.h"
#include "spinlock.h"
#include "../lib/stdio.h"
#include "../lib/x86.h"
#include "../mm/memory.h"
#include "../mm/slab.h"
#include "../trap/apic.h"

uint8_t global_i;
spinlock_t smp_lock;

void send_IPI(struct ICR_Entry *entry) {
    unsigned long value = *(unsigned long *) entry;
    if (x2APIC_supported()) {
        wrmsr(ICR_MSR, value);
    } else {
        wrmmio(ICR_HIGH, value >> 32UL);
        wrmmio(ICR_LOW, value & ((1UL << 32U) - 1U));
    }
}

void smp_init() {
    void *tss;

    print_color(YELLOW, BLACK, "AP Boot start addr: %#018lx\n", &_APU_boot_start);
    print_color(YELLOW, BLACK, "AP Boot end   addr: %#018lx\n", &_APU_boot_end);

    uint32_t start_up_code = 0x20000; // < 1M
    memcpy(&_APU_boot_start, (void *) phy_to_vir(start_up_code), &_APU_boot_end - &_APU_boot_start);

    struct ICR_Entry entry;
    entry.IPI_vector = 0;
    entry.delivery_mode = DELIVERY_MODE_INIT;
    entry.dest_mode = DEST_MODE_PHYSICAL;
    entry.delivery_status = DELIVERY_STATUS_IDLE;
    entry.reserved1 = 0;
    entry.level = LEVEL_ASSERT;
    entry.trigger_mode = TRIGGER_MODE_EDGE;
    entry.reserved2 = 0;
    entry.dest_shorthand = SHORTHAND_ALL_EX_SELF;
    entry.reserved3 = 0;

    send_IPI(&entry);

    spin_init(&smp_lock);

    while (global_i < 3) {
        spin_lock(&smp_lock);
        global_i++;

        _stack_start = (unsigned long) (kmalloc(PROC_STACK_SIZE) + PROC_STACK_SIZE); //  head.S
        tss = kmalloc(128);
        set_tss_desc(10 + global_i * 2, tss);
        setup_TSS(tss, _stack_start, _stack_start, _stack_start, _stack_start, _stack_start, _stack_start, _stack_start,
                  _stack_start, _stack_start, _stack_start);

        entry.IPI_vector = start_up_code >> 12U;
        entry.delivery_mode = DELIVERY_MODE_START_UP;
        entry.dest_shorthand = SHORTHAND_NO;
        entry.dest_field = global_i;

        // Send SIPI twice
        send_IPI(&entry); // Start-up IPI (SIPI)
        send_IPI(&entry); // Start-up IPI (SIPI)
    }
}

void Start_SMP() {
    load_TR(10U + global_i * 2);
    print_color(BLACK, WHITE, "AP started... [%d]\n", rdmmio(APIC_ID_REG) >> 24UL);
    spin_unlock(&smp_lock);
    hlt();
}