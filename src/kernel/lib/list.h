//
// Created by 赵明明 on 2019/12/16.
//

#ifndef _LIST_H
#define _LIST_H

#include "defs.h"

struct list {
    struct list *prev;
    struct list *next;
};

typedef struct list list_t;

static inline void list_init(list_t *list) {
    list->prev = list;
    list->next = list;
}

static inline void list_add_to_before(list_t *entry, list_t *new) {
    new->next = entry;
    new->prev = entry->prev;
    entry->prev->next = new;
    entry->prev = new;
}

static inline void list_delete(list_t *entry) {
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->prev = NULL;
    entry->prev = NULL;
}

#endif //_LIST_H
