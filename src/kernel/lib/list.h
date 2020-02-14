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

static inline void __list_add(list_t *entry, list_t *prev, list_t *next) {
    prev->next = next->prev = entry;
    entry->prev = prev;
    entry->next = next;
}

static inline void __list_del(list_t *prev, list_t *next) {
    prev->next = next;
    next->prev = prev;
}

static inline void list_init(list_t *list) {
    list->prev = list->next = list;
}

static inline list_t *list_next(list_t *list) {
    return list->next;
}

static inline list_t *list_prev(list_t *list) {
    return list->prev;
}

static inline void list_add_to_before(list_t *head, list_t *entry) {
    __list_add(entry, head->prev, head);
}

static inline void list_add_to_after(list_t *head, list_t *entry) {
    __list_add(entry, head, head->next);
}

static inline void list_del(list_t *list) {
    __list_del(list->prev, list->next);
}

static inline int list_is_empty(list_t *list) {
    return list == list->next && list == list->prev;
}

#endif //_LIST_H
