//
// Created by 赵明明 on 2020/2/4.
//

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

typedef struct {
    __volatile__ unsigned long lock;
} spinlock_t;

static inline void spin_init(spinlock_t *lock) {
    lock->lock = 1;
}

static inline void spin_lock(spinlock_t *lock) {
    __asm__ __volatile__ ("1:           \n\t"
                          "lock decq %0 \n\t"
                          "jns  3f      \n\t"
                          "2:           \n\t"
                          "cmpq $1, %0  \n\t"
                          "je   1b      \n\t"
                          "pause        \n\t"
                          "jmp  2b      \n\t"
                          "3:           \n\t"
    :"=m"(lock->lock)
    :
    :"memory");
}

static inline void spin_unlock(spinlock_t *lock) {
    __asm__ __volatile__ ("movq $1, %0":"=m"(lock->lock)::"memory");
}

#endif //_SPINLOCK_H
