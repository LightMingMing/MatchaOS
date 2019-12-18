//
// Created by 赵明明 on 2019/12/16.
//

#ifndef _LIST_H
#define _LIST_H

struct list {
    struct list *prev;
    struct list *next;
};

typedef struct list list_t;

static inline void list_init(list_t *list) {
    list->prev = list;
    list->next = list;
}

#endif //_LIST_H
