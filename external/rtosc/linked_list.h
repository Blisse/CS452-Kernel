#pragma once

#include <rt.h>

struct _RT_LINKED_LIST_NODE;

typedef struct _RT_LINKED_LIST_NODE {
    struct _RT_LINKED_LIST_NODE* next;
    struct _RT_LINKED_LIST_NODE* previous;
    PVOID data;
} RT_LINKED_LIST_NODE;

typedef struct _RT_LINKED_LIST {
    UINT size;
    RT_LINKED_LIST_NODE* head;
    RT_LINKED_LIST_NODE* tail;
} RT_LINKED_LIST;

VOID
RtLinkedListNodeInit
    (
        RT_LINKED_LIST_NODE* node
    );

VOID
RtLinkedListInit
    (
        IN RT_LINKED_LIST* list
    );

RT_STATUS
RtLinkedListPushFront
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    );

RT_STATUS
RtLinkedListPushBack
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    );

RT_STATUS
RtLinkedListPopFront
    (
        IN RT_LINKED_LIST* list
    );

RT_STATUS
RtLinkedListPopBack
    (
        IN RT_LINKED_LIST* list
    );

RT_STATUS
RtLinkedListPeekFront
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    );

RT_STATUS
RtLinkedListPeekBack
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    );

RT_STATUS
RtLinkedListPeekAndPopFront
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    );

RT_STATUS
RtLinkedListPeekAndPopBack
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    );

RT_STATUS
RtLinkedListInsertBetween
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* before,
        IN RT_LINKED_LIST_NODE* after,
        IN RT_LINKED_LIST_NODE* node
    );

RT_STATUS
RtLinkedListRemove
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    );

inline
BOOLEAN
RtLinkedListIsEmpty
    (
        IN RT_LINKED_LIST* list
    );
