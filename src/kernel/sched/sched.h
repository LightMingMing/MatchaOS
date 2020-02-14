//
// Created by 赵明明 on 2020/2/11.
//

#ifndef _SCHED_H
#define _SCHED_H

#include "../lib/defs.h"
#include "../proc/proc.h"

typedef struct {
    uint32_t count;
    long exec_jiffies;
    struct proc_struct head_proc;
} sched_queue_t;

sched_queue_t sched_queue;

struct proc_struct *get_next_proc();

void insert(struct proc_struct *proc);

void sched_init();

void do_sched();

#endif //_SCHED_H
