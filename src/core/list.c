#include <stdlib.h>

#include "types.h"

list_t * list_create(void *data) {
    list_t * l = calloc(1, sizeof(list_t));
    l->data=data;
    return l;
}

list_t * list_dup(const list_t *list) {
    list_t *l=nullptr;
    for (;list;list=list->next) {
        list_emplace(&l, list->data);
    }
    return l;
}

void list_emplace(list_t **list, void *data) {
    if (*list) {
        list_insert(*list, data);
        return;
    }
    *list = list_create(data);
}

void list_insert(list_t *list, void *data) {
    list_t * last=list;
    while (last->next)
        last=last->next;
    last->next=list_create(data);
}
list_t * list_find(list_t *list, const void *data) {
    for (; list; list=list->next) {
        if (list->data==data)
            return list;
    }
    return nullptr;
}
void * list_remove(list_t **list, list_t *item) {
    const bool clear=*list==item?item->next==nullptr:false;
    void *r = list_erase(*list, item);
    if (clear)
        *list=nullptr;
    return r;
}
void * list_erase(list_t *list, list_t *item) {
    list_t * prev=list;
    void * data=item->data;
    if (list!=item) {
        while (prev->next && prev->next!=item) {
            prev=prev->next;
        }
        prev->next=item->next;
        free(item);
    } else {
        if (item->next) {
            list->data=item->next->data;
            list->next=item->next->next;
        } else {
            free(item);
        }
    }

    return data;
}
void list_destroy(list_t *list) {
    while (list) {
        list_t * next=list->next;
        free(list);
        list = next;
    }
}

uint64_t list_size(const list_t *list) {
    uint64_t size=0;
    for (; list; list=list->next) {
        size++;
    }
    return size;
}

void * list_front(const list_t *list) {
    return list->data;
}

void * list_back(const list_t *list) {
    for (; list; list=list->next) {
        if (!list->next)
            return list->data;
    }
    return nullptr;
}
