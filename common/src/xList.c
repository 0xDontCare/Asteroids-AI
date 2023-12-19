#include "xList.h"

#include <stdlib.h>

xList *xList_new() {
    xList *list = malloc(sizeof(xList));
    if (list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void xList_free(xList *list) {
    xListNode *node = list->head;
    while (node != NULL) {
        xListNode *next = node->next;
        free(node);
        node = next;
    }
    free(list);
}

void xList_pushFront(xList *list, void *data) {
    if (list == NULL || data == NULL) {
        return;
    }

    xListNode *node = malloc(sizeof(xListNode));
    if (node == NULL) {
        return;
    }

    node->data = data;
    node->next = list->head;
    node->prev = NULL;

    if (list->head != NULL) {
        list->head->prev = node;
    } else {
        list->tail = node;
    }

    list->head = node;
    list->size++;
}

void xList_pushBack(xList *list, void *data) {
    if (list == NULL || data == NULL) {
        return;
    }

    xListNode *node = malloc(sizeof(xListNode));
    if (node == NULL) {
        return;
    }

    node->data = data;
    node->next = NULL;
    node->prev = list->tail;

    if (list->tail != NULL) {
        list->tail->next = node;
    } else {
        list->head = node;
    }

    list->tail = node;
    list->size++;
}

void *xList_popFront(xList *list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }

    xListNode *node = list->head;
    void *data = node->data;

    list->head = node->next;
    if (list->head != NULL) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }

    free(node);
    list->size--;
    return data;
}

void *xList_popBack(xList *list) {
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }

    xListNode *node = list->tail;
    void *data = node->data;

    list->tail = node->prev;
    if (list->tail != NULL) {
        list->tail->next = NULL;
    } else {
        list->head = NULL;
    }

    free(node);
    list->size--;
    return data;
}

void *xList_front(xList *list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    return list->head->data;
}

void *xList_back(xList *list) {
    if (list == NULL || list->tail == NULL) {
        return NULL;
    }
    return list->tail->data;
}

void xList_insert(xList *list, int index, void *data) {
    if (list == NULL || data == NULL || index < 0 || index > list->size) {
        return;
    }

    if (index == 0) {
        xList_pushFront(list, data);
        return;
    } else if (index == list->size) {
        xList_pushBack(list, data);
        return;
    }

    if (index < list->size / 2) {
        xListNode *node = list->head;
        for (int i = 0; i < index; i++) {
            node = node->next;
        }

        xListNode *newNode = malloc(sizeof(xListNode));
        if (newNode == NULL) {
            return;
        }

        newNode->data = data;
        newNode->next = node;
        newNode->prev = node->prev;

        node->prev->next = newNode;
        node->prev = newNode;
    } else {
        xListNode *node = list->tail;
        for (int i = list->size - 1; i > index; i--) {
            node = node->prev;
        }

        xListNode *newNode = malloc(sizeof(xListNode));
        if (newNode == NULL) {
            return;
        }

        newNode->data = data;
        newNode->next = node;
        newNode->prev = node->prev;

        node->prev->next = newNode;
        node->prev = newNode;
    }

    list->size++;
}

void *xList_get(xList *list, int index) {
    if (list == NULL || index < 0 || index >= list->size) {
        return NULL;
    }

    if (index < list->size / 2) {
        xListNode *node = list->head;
        for (int i = 0; i < index; i++) {
            node = node->next;
        }
        return node->data;
    } else {
        xListNode *node = list->tail;
        for (int i = list->size - 1; i > index; i--) {
            node = node->prev;
        }
        return node->data;
    }
}

void *xList_remove(xList *list, int index) {
    if (list == NULL || index < 0 || index >= list->size) {
        return NULL;
    }

    if (index == 0) {
        return xList_popFront(list);
    } else if (index == list->size - 1) {
        return xList_popBack(list);
    }

    if (index < list->size / 2) {
        xListNode *node = list->head;
        for (int i = 0; i < index; i++) {
            node = node->next;
        }

        void *data = node->data;

        node->prev->next = node->next;
        node->next->prev = node->prev;

        free(node);
        list->size--;
        return data;
    } else {
        xListNode *node = list->tail;
        for (int i = list->size - 1; i > index; i--) {
            node = node->prev;
        }

        void *data = node->data;

        node->prev->next = node->next;
        node->next->prev = node->prev;

        free(node);
        list->size--;
        return data;
    }
}

void xList_clear(xList *list) {
    if (list == NULL) {
        return;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        xListNode *next = node->next;
        free(node);
        node = next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

static void xList_qsort(xListNode *head, xListNode *tail, int (*comparator)(const void *, const void *)) {
    if (head == NULL || tail == NULL || comparator == NULL) {
        return;
    }

    if (head == tail) {
        return;
    }

    xListNode *pivot = head;
    xListNode *node = head->next;
    while (node != tail) {
        if (comparator(node->data, pivot->data) < 0) {
            pivot = pivot->next;
            void *temp = pivot->data;
            pivot->data = node->data;
            node->data = temp;
        }
        node = node->next;
    }

    void *temp = pivot->data;
    pivot->data = head->data;
    head->data = temp;

    xList_qsort(head, pivot, comparator);
    xList_qsort(pivot->next, tail, comparator);
}

void xList_sort(xList *list, int (*comparator)(const void *, const void *)) {
    if (list == NULL || comparator == NULL) {
        return;
    }

    if (list->size < 2) {
        return;
    }

    xList_qsort(list->head, list->tail->next, comparator);

    xListNode *node = list->head;
    while (node->next != NULL) {
        node = node->next;
    }
    list->tail = node;

    node = list->tail;
    while (node->prev != NULL) {
        node = node->prev;
    }
    list->head = node;
}

void xList_reverse(xList *list) {
    if (list == NULL) {
        return;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        xListNode *next = node->next;
        node->next = node->prev;
        node->prev = next;
        node = next;
    }

    xListNode *temp = list->head;
    list->head = list->tail;
    list->tail = temp;
}

xList *xList_map(xList *list, void *(*func)(void *)) {
    if (list == NULL || func == NULL) {
        return NULL;
    }

    xList *newList = xList_new();
    if (newList == NULL) {
        return NULL;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        void *data = func(node->data);
        // not handling NULL return value since it might be expected for some functions
        xList_pushBack(newList, data);
        node = node->next;
    }

    return newList;
}

xList *xList_filter(xList *list, int (*func)(void *)) {
    if (list == NULL || func == NULL) {
        return NULL;
    }

    xList *newList = xList_new();
    if (newList == NULL) {
        return NULL;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        if (func(node->data)) {
            xList_pushBack(newList, node->data);
        }
        node = node->next;
    }

    return newList;
}

void *xList_reduce(xList *list, void *(*func)(void *, void *)) {
    if (list == NULL || func == NULL) {
        return NULL;
    }

    if (list->size == 0) {
        return NULL;
    }

    void *result = list->head->data;
    xListNode *node = list->head->next;
    while (node != NULL) {
        result = func(result, node->data);
        node = node->next;
    }

    return result;
}

void xList_forEach(xList *list, void (*func)(void *)) {
    if (list == NULL || func == NULL) {
        return;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        func(node->data);
        node = node->next;
    }
}

void xList_forEachArg(xList *list, void (*func)(void *, void **), void **arg) {
    if (list == NULL || func == NULL || arg == NULL) {
        return;
    }

    if (*arg == NULL) {
        return;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        func(node->data, arg);
        node = node->next;
    }
}

xList *xList_concat(xList *list1, xList *list2) {
    if (list1 == NULL || list2 == NULL) {
        return NULL;
    }

    xList *newList = xList_new();
    if (newList == NULL) {
        return NULL;
    }

    xListNode *node = list1->head;
    while (node != NULL) {
        xList_pushBack(newList, node->data);
        node = node->next;
    }

    node = list2->head;
    while (node != NULL) {
        xList_pushBack(newList, node->data);
        node = node->next;
    }

    return newList;
}

xList *xList_slice(xList *list, int start, int end) {
    if (list == NULL || start < 0 || end > list->size || start > end) {
        return NULL;
    }

    xList *newList = xList_new();
    if (newList == NULL) {
        return NULL;
    }

    if (start > list->size - end) {
        xListNode *node = list->tail;
        for (int i = list->size - 1; i >= end; i--) {
            node = node->prev;
        }
        for (int i = end - 1; i >= start; i--) {
            xList_pushFront(newList, node->data);
            node = node->prev;
        }
    } else {
        xListNode *node = list->head;
        for (int i = 0; i < start; i++) {
            node = node->next;
        }
        for (int i = start; i < end; i++) {
            xList_pushBack(newList, node->data);
            node = node->next;
        }
    }

    return newList;
}

xList *xList_copy(xList *list) {
    if (list == NULL) {
        return NULL;
    }

    xList *newList = xList_new();
    if (newList == NULL) {
        return NULL;
    }

    xListNode *node = list->head;
    while (node != NULL) {
        xList_pushBack(newList, node->data);
        node = node->next;
    }

    return newList;
}

void xList_swap(xList *list, int index1, int index2) {
    if (list == NULL || index1 < 0 || index1 >= list->size || index2 < 0 || index2 >= list->size || index1 == index2) {
        return;
    }

    xListNode *node1 = NULL;
    xListNode *node2 = NULL;

    if (index1 < list->size / 2) {
        node1 = list->head;
        for (int i = 0; i < index1; i++) {
            node1 = node1->next;
        }
    } else {
        node1 = list->tail;
        for (int i = list->size - 1; i > index1; i--) {
            node1 = node1->prev;
        }
    }

    if (index2 < list->size / 2) {
        node2 = list->head;
        for (int i = 0; i < index2; i++) {
            node2 = node2->next;
        }
    } else {
        node2 = list->tail;
        for (int i = list->size - 1; i > index2; i--) {
            node2 = node2->prev;
        }
    }

    void *temp = node1->data;
    node1->data = node2->data;
    node2->data = temp;
}
