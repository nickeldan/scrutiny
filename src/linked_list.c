#include <stdlib.h>

#include "linked_list.h"

void *
linkedListNodeNew(size_t size)
{
    linkedListNode *node;

    node = malloc(size);
    if (!node) {
        exit(1);
    }
    node->next = NULL;
    return node;
}

void
linkedListInit(linkedList *list)
{
    list->head = list->tail = NULL;
}

void
linkedListAppend(linkedList *list, linkedListNode *node)
{
    if (list->tail) {
        list->tail->next = node;
        list->tail = node;
    }
    else {
        list->head = list->tail = node;
    }
}

void
linkedListFree(linkedList *list, void (*cleanup)(void *))
{
    while (list->head) {
        void *item = list->head;

        list->head = list->head->next;
        if (cleanup) {
            cleanup(item);
        }
        free(item);
    }

    list->tail = NULL;
}
