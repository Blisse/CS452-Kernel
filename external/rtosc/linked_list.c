#include "linked_list.h"

#include <rtosc/assert.h>

VOID
RtLinkedListInit
    (
        IN RT_LINKED_LIST* list,
        IN UINT capacity
    )
{
    list->capacity = capacity;
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
}

static
inline
VOID
RtLinkedListpPushEmpty
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    )
{
    ASSERT(RtLinkedListIsEmpty(list), "Linked list is not empty");

    node->next = NULL;
    node->previous = NULL;

    list->head = node;
    list->tail = node;

    list->size = 1;
}

RT_STATUS
RtLinkedListPushFront
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (!RtLinkedListIsFull(list))
    {
        if (RtLinkedListIsEmpty(list))
        {
            RtLinkedListpPushEmpty(list, node);
        }
        else
        {
            RT_LINKED_LIST_NODE* previousHead = list->head;

            node->next = previousHead;
            node->previous = NULL;

            previousHead->previous = node;

            list->head = node;
            list->size = list->size + 1;
        }
    }
    else
    {
        status = STATUS_BUFFER_FULL;
    }
    return status;
}

RT_STATUS
RtLinkedListPushBack
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (!RtLinkedListIsFull(list))
    {
        if (RtLinkedListIsEmpty(list))
        {
            RtLinkedListpPushEmpty(list, node);
        }
        else
        {
            RT_LINKED_LIST_NODE* previousTail = list->tail;

            node->next = NULL;
            node->previous = previousTail;

            previousTail->next = node;

            list->tail = node;
            list->size = list->size + 1;
        }
    }
    else
    {
        status = STATUS_BUFFER_FULL;
    }
    return status;
}

static
inline
VOID
RtLinkedListpDetachNode
    (
        IN RT_LINKED_LIST_NODE* node
    )
{
    node->next = NULL;
    node->previous = NULL;
}

static
inline
VOID
RtLinkedListpPopEmpty
    (
        IN RT_LINKED_LIST* list
    )
{
    ASSERT(list->size == 1, "Linked list is not going to be empty");

    list->head->previous = NULL;
    list->head->next = NULL;

    list->head = NULL;
    list->tail = NULL;

    list->size = 0;
}

RT_STATUS
RtLinkedListPopFront
    (
        IN RT_LINKED_LIST* list
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (!RtLinkedListIsEmpty(list))
    {
        if (list->size == 1)
        {
            RtLinkedListpPopEmpty(list);
        }
        else
        {
            RT_LINKED_LIST_NODE* previousHead = list->head;

            list->head = previousHead->next;
            list->head->previous = NULL;
            list->size = list->size - 1;

            RtLinkedListpDetachNode(previousHead);
        }
    }
    else
    {
        status = STATUS_BUFFER_EMPTY;
    }
    return status;
}

RT_STATUS
RtLinkedListPopBack
    (
        IN RT_LINKED_LIST* list
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (!RtLinkedListIsEmpty(list))
    {
        if (list->size == 1)
        {
            RtLinkedListpPopEmpty(list);
        }
        else
        {
            RT_LINKED_LIST_NODE* previousTail = list->tail;

            list->tail = previousTail->previous;
            list->tail->next = NULL;
            list->size = list->size - 1;

            RtLinkedListpDetachNode(previousTail);
        }
    }
    else
    {
        status = STATUS_BUFFER_EMPTY;
    }
    return status;
}

RT_STATUS
RtLinkedListPeekAndPopFront
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    )
{
    RT_STATUS status = RtLinkedListPeekFront(list, node);

    if (RT_SUCCESS(status))
    {
        status = RtLinkedListPopFront(list);
    }

    return status;
}

RT_STATUS
RtLinkedListPeekAndPopBack
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    )
{
    RT_STATUS status = RtLinkedListPeekBack(list, node);

    if (RT_SUCCESS(status))
    {
        status = RtLinkedListPopBack(list);
    }

    return status;
}

RT_STATUS
RtLinkedListPeekFront
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (list->head != NULL)
    {
        *node = list->head;
    }
    else
    {
        status = STATUS_BUFFER_EMPTY;
    }
    return status;
}

RT_STATUS
RtLinkedListPeekBack
    (
        IN RT_LINKED_LIST* list,
        OUT RT_LINKED_LIST_NODE** node
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (list->tail != NULL)
    {
        *node = list->tail;
    }
    else
    {
        status = STATUS_BUFFER_EMPTY;
    }
    return status;
}

RT_STATUS
RtLinkedListInsertBetween
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* before,
        IN RT_LINKED_LIST_NODE* after,
        IN RT_LINKED_LIST_NODE* node
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (!RtLinkedListIsFull(list))
    {
        if (RtLinkedListIsEmpty(list))
        {
            RtLinkedListpPushEmpty(list, node);
        }
        else if (after == list->head)
        {
            status = RtLinkedListPushFront(list, node);
        }
        else if (before == list->tail)
        {
            status = RtLinkedListPushBack(list, node);
        }
        else if (before != NULL && after != NULL)
        {
            ASSERT(before->next == after && after->previous == before, "Must insert between two valid nodes");

            before->next = node;
            node->next = after;

            node->previous = before;
            after->previous = node;

            list->size = list->size + 1;
        }
        else
        {
            status = STATUS_NOT_FOUND;
        }
    }
    else
    {
        status = STATUS_BUFFER_FULL;
    }
    return status;
}

BOOLEAN
RtLinkedListpFind
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    )
{
    RT_LINKED_LIST_NODE* current = list->head;

    while (current != NULL)
    {
        if (current == node)
        {
            return TRUE;
        }

        current = current->next;
    }

    return FALSE;
}

RT_STATUS
RtLinkedListRemove
    (
        IN RT_LINKED_LIST* list,
        IN RT_LINKED_LIST_NODE* node
    )
{
    RT_STATUS status = STATUS_SUCCESS;

    if (!RtLinkedListIsEmpty(list))
    {
        if (node == list->head)
        {
            status = RtLinkedListPopFront(list);
        }
        else if (node == list->tail)
        {
            status = RtLinkedListPopBack(list);
        }
        else if (RtLinkedListpFind(list, node))
        {
            RT_LINKED_LIST_NODE* previous = node->previous;
            RT_LINKED_LIST_NODE* next = node->next;

            previous->next = next;
            next->previous = previous;

            node->next = NULL;
            node->previous = NULL;

            list->size = list->size - 1;
        }
        else
        {
            status = STATUS_NOT_FOUND;
        }
    }
    else
    {
        status = STATUS_NOT_FOUND;
    }
    return status;
}


inline
BOOLEAN
RtLinkedListIsEmpty
    (
        IN RT_LINKED_LIST* list
    )
{
    return (list->size == 0);
}

inline
BOOLEAN
RtLinkedListIsFull
    (
        IN RT_LINKED_LIST* list
    )
{
    return (list->size == list->capacity);
}
