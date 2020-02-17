//
// Created by 赵明明 on 2020/2/11.
//

#include "sched.h"

struct proc_struct *get_next_proc() {
    int cpu_id = get_cpu_id();
    if (list_is_empty(&sched_queue[cpu_id].head_proc.list)) {
        return init_proc[cpu_id];
    }
    struct proc_struct *next = container_of(list_next(&sched_queue[cpu_id].head_proc.list), struct proc_struct, list);
    list_del(&next->list);
    sched_queue[cpu_id].count--;
    return next;
}

void insert(struct proc_struct *proc) {
    int cpu_id = get_cpu_id();
    if (proc == init_proc[cpu_id]) {
        return;
    }
    list_add_to_before(&sched_queue[cpu_id].head_proc.list, &proc->list);
    sched_queue[cpu_id].count++;
}

void sched_init() {
    memset(&sched_queue, 0, sizeof(sched_queue_t) * NR_CPUs);
    for (int i = 0; i < NR_CPUs; i++) {
        list_init(&sched_queue[i].head_proc.list);
        sched_queue[i].count = 1;
        sched_queue[i].exec_jiffies = 4;
    }
}

void do_sched() {
    struct proc_struct *current = get_current();
    current->flags &= ~PROC_NEED_SCHEDULE;

    struct proc_struct *next = get_next_proc();

    int cpu_id = get_cpu_id();
    if (!sched_queue[cpu_id].exec_jiffies) {
        sched_queue[cpu_id].exec_jiffies = 4;
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