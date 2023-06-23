#pragma once

#include <sys/types.h>

typedef struct linkedListNode {
    void *next;
} linkedListNode;

typedef struct linkedList {
    linkedListNode *head;
    linkedListNode *tail;
} linkedList;

void *
linkedListNodeNew(size_t size);

void
linkedListInit(linkedList *list);

void
linkedListAppend(linkedList *list, linkedListNode *node);

void
linkedListFree(linkedList *list, void (*cleanup)(void *));

#define LINKED_LIST_ITERATE(list, item) \
    for (item = (void *)(list)->head; item; item = ((linkedListNode *)item)->next)
