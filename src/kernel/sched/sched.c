//
// Created by 赵明明 on 2020/2/11.
//

#include "sched.h"

struct proc_struct *get_next_proc() {
    if (list_is_empty(&sched_queue.head_proc.list)) {
        return &init_proc_union.proc;
    }
    struct proc_struct *next = container_of(list_next(&sched_queue.head_proc.list), struct proc_struct, list);
    list_del(&next->list);
    sched_queue.count--;
    return next;
}

void insert(struct proc_struct *proc) {
    if (proc == &init_proc_union.proc) {
        return;
    }
    list_add_to_before(&sched_queue.head_proc.list, &proc->list);
    sched_queue.count++;
}

void sched_init() {
    memset(&sched_queue, 0, sizeof(sched_queue_t));
    list_init(&sched_queue.head_proc.list);
    sched_queue.count = 1;
    sched_queue.exec_jiffies = 4;
}

void do_sched() {
    struct proc_struct *current = get_current();
    current->flags &= ~PROC_NEED_SCHEDULE;

    struct proc_struct *next = get_next_proc();

    if (!sched_queue.exec_jiffies) {
        sched_queue.exec_jiffies = 4;
    }

    if (current->run_time >= next->run_time && current->pid != next->pid) {
        if (current->state == PROC_RUNNABLE) {
            insert(current);
        }
        switch_to(current, next);
    } else {
        insert(current);
    }
}