#include <rtosc/linked_list.h>

#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rt.h>

#define TEST_BUFFER_SIZE 8

void test_linked_list_init() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list, TEST_BUFFER_SIZE);

    T_ASSERT(list.capacity == TEST_BUFFER_SIZE, "List capacity is incorrect");
    T_ASSERT(list.size == 0, "List size is incorrect");
    T_ASSERT(list.head == NULL, "List head is incorrect");
    T_ASSERT(list.tail == NULL, "List tail is incorrect");
}

void test_linked_list_push_front() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list, TEST_BUFFER_SIZE);

    INT i = 1;
    RT_LINKED_LIST_NODE na;
    na.data = &i;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)), "Failed to push to list front");

    T_ASSERT(list.size == 1, "List size is incorrect");
    T_ASSERT(list.head == &na, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->previous == NULL, "List head data is incorrect");
    T_ASSERT(list.head->next == NULL, "List head data is incorrect");
    T_ASSERT(list.head->data == &i, "List head data is incorrect");
    T_ASSERT(list.tail->data == &i, "List tail data is incorrect");
    T_ASSERT(list.tail->previous == NULL, "List tail data is incorrect");
    T_ASSERT(list.tail->next == NULL, "List tail data is incorrect");

    INT j = 1;
    RT_LINKED_LIST_NODE nb;
    nb.data = &j;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)), "Failed to push to list front");

    T_ASSERT(list.size == 2, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->previous == NULL, "List head data is incorrect");
    T_ASSERT(list.head->next == &na, "List head data is incorrect");
    T_ASSERT(list.head->data == &j, "List head data is incorrect");
    T_ASSERT(list.tail->previous == &nb, "List tail data is incorrect");
    T_ASSERT(list.tail->next == NULL, "List tail data is incorrect");
    T_ASSERT(list.tail->data == &i, "List tail data is incorrect");

    INT k = 1;
    RT_LINKED_LIST_NODE nc;
    nc.data = &k;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nc)), "Failed to push to list front");

    T_ASSERT(list.size == 3, "List size is incorrect");
    T_ASSERT(list.head == &nc, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->previous == NULL, "List head data is incorrect");
    T_ASSERT(list.head->next == &nb, "List head data is incorrect");
    T_ASSERT(list.head->data == &k, "List head data is incorrect");
    T_ASSERT(list.tail->previous == &nb, "List tail data is incorrect");
    T_ASSERT(list.tail->next == NULL, "List tail data is incorrect");
    T_ASSERT(list.tail->data == &i, "List tail data is incorrect");
}

void test_linked_list_push_back() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list, TEST_BUFFER_SIZE);

    INT i = 1;
    RT_LINKED_LIST_NODE na;
    na.data = &i;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &na)), "Failed to push to list back");

    T_ASSERT(list.size == 1, "List size is incorrect");
    T_ASSERT(list.head == &na, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->previous == NULL, "List head data is incorrect");
    T_ASSERT(list.head->next == NULL, "List head data is incorrect");
    T_ASSERT(list.head->data == &i, "List head data is incorrect");
    T_ASSERT(list.tail->previous == NULL, "List tail data is incorrect");
    T_ASSERT(list.tail->next == NULL, "List tail data is incorrect");
    T_ASSERT(list.tail->data == &i, "List tail data is incorrect");

    INT j = 1;
    RT_LINKED_LIST_NODE nb;
    nb.data = &j;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nb)), "Failed to push to list back");

    T_ASSERT(list.size == 2, "List size is incorrect");
    T_ASSERT(list.head == &na, "List head is incorrect");
    T_ASSERT(list.tail == &nb, "List tail is incorrect");
    T_ASSERT(list.head->previous == NULL, "List head data is incorrect");
    T_ASSERT(list.head->next == &nb, "List head data is incorrect");
    T_ASSERT(list.head->data == &i, "List head data is incorrect");
    T_ASSERT(list.tail->previous == &na, "List tail data is incorrect");
    T_ASSERT(list.tail->next == NULL, "List tail data is incorrect");
    T_ASSERT(list.tail->data == &j, "List tail data is incorrect");

    INT k = 1;
    RT_LINKED_LIST_NODE nc;
    nc.data = &k;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nc)), "Failed to push to list back");

    T_ASSERT(list.size == 3, "List size is incorrect");
    T_ASSERT(list.head == &na, "List head is incorrect");
    T_ASSERT(list.tail == &nc, "List tail is incorrect");
    T_ASSERT(list.head->previous == NULL, "List head data is incorrect");
    T_ASSERT(list.head->next == &nb, "List head data is incorrect");
    T_ASSERT(list.head->data == &i, "List head data is incorrect");
    T_ASSERT(list.tail->data == &k, "List tail data is incorrect");
    T_ASSERT(list.tail->previous == &nb, "List tail data is incorrect");
    T_ASSERT(list.tail->next == NULL, "List tail data is incorrect");
}

void test_linked_list_push_more() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list, 4);

    T_ASSERT(RtLinkedListIsEmpty(&list), "List is not empty");
    T_ASSERT(!RtLinkedListIsFull(&list), "List is full");

    RT_LINKED_LIST_NODE na;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &na)), "Failed to push to list back");
    RT_LINKED_LIST_NODE nb;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nb)), "Failed to push to list back");
    RT_LINKED_LIST_NODE nc;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nc)), "Failed to push to list back");
    RT_LINKED_LIST_NODE nd;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nd)), "Failed to push to list back");

    T_ASSERT(list.size == 4, "List size is incorrect");

    T_ASSERT(!RtLinkedListIsEmpty(&list), "List is empty");
    T_ASSERT(RtLinkedListIsFull(&list), "List is not full");

    RT_LINKED_LIST_NODE ne;
    T_ASSERT(RT_FAILURE(RtLinkedListPushBack(&list, &ne)), "Shouldn't be able to push to list back");
}

void test_linked_list_pop() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list, TEST_BUFFER_SIZE);

    RT_LINKED_LIST_NODE na;
    RT_LINKED_LIST_NODE nb;
    RT_LINKED_LIST_NODE nc;
    RT_LINKED_LIST_NODE nd;

    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &na)), "Failed to push to list back");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nb)), "Failed to push to list back");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nc)), "Failed to push to list back");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nd)), "Failed to push to list back");

    T_ASSERT(list.size == 4, "List size is incorrect");

    RT_LINKED_LIST_NODE* ne;
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nd, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nc, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nb, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &na, "Popped wrong node");
    T_ASSERT(RT_FAILURE(RtLinkedListPeekAndPopBack(&list, &ne)), "Succeeded to pop list back");

    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)), "Failed to push to list front");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)), "Failed to push to list front");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nc)), "Failed to push to list front");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nd)), "Failed to push to list front");

    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &na, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nb, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nc, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nd, "Popped wrong node");
    T_ASSERT(RT_FAILURE(RtLinkedListPeekAndPopBack(&list, &ne)), "Succeeded to pop list back");


    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)), "Failed to push to list front");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)), "Failed to push to list front");

    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nb, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &na, "Popped wrong node");

    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)), "Failed to push to list front");
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)), "Failed to push to list front");

    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &nb, "Popped wrong node");
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)), "Failed to pop list back");
    T_ASSERT(ne == &na, "Popped wrong node");

    T_ASSERT(RT_FAILURE(RtLinkedListPeekAndPopFront(&list, &ne)), "Succeeded to pop list back");
}

void test_linked_list_insert_and_remove() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list, TEST_BUFFER_SIZE);

    RT_LINKED_LIST_NODE na;
    RT_LINKED_LIST_NODE nb;
    RT_LINKED_LIST_NODE nc;
    RT_LINKED_LIST_NODE nd;
    RT_LINKED_LIST_NODE ne;
    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, NULL, NULL, &na)), "Failed to insert");
    T_ASSERT(list.size == 1, "List size is incorrect");
    T_ASSERT(list.head == &na, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, NULL, &na, &nb)), "Failed to insert");
    T_ASSERT(list.size == 2, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &na, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, &nb, &na, &nc)), "Failed to insert");
    T_ASSERT(list.size == 3, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &nc, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, &nc, &na, &nd)), "Failed to insert");
    T_ASSERT(list.size == 4, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &nc, "List head is incorrect");
    T_ASSERT(list.head->next->next == &nd, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, &na, NULL, &ne)), "Failed to insert");
    T_ASSERT(list.size == 5, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &ne, "List tail is incorrect");
    T_ASSERT(list.head->next == &nc, "List head is incorrect");
    T_ASSERT(list.head->next->next == &nd, "List head is incorrect");
    T_ASSERT(list.head->next->next->next == &na, "List head is incorrect");
    T_ASSERT(list.head->next->next->next->next == &ne, "List head is incorrect");


    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &ne)), "Failed to remove");
    T_ASSERT(list.size == 4, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &nc, "List head is incorrect");
    T_ASSERT(list.head->next->next == &nd, "List head is incorrect");
    T_ASSERT(list.head->next->next->next == &na, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &nc)), "Failed to remove");
    T_ASSERT(list.size == 3, "List size is incorrect");
    T_ASSERT(list.head == &nb, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &nd, "List head is incorrect");
    T_ASSERT(list.head->next->next == &na, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &nb)), "Failed to remove");
    T_ASSERT(list.size == 2, "List size is incorrect");
    T_ASSERT(list.head == &nd, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &na, "List head is incorrect");

    T_ASSERT(RT_FAILURE(RtLinkedListRemove(&list, &nb)), "Shouldn't have removed");
    T_ASSERT(list.size == 2, "List size is incorrect");
    T_ASSERT(list.head == &nd, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == &na, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &nd)), "Failed to remove");
    T_ASSERT(list.size == 1, "List size is incorrect");
    T_ASSERT(list.head == &na, "List head is incorrect");
    T_ASSERT(list.tail == &na, "List tail is incorrect");
    T_ASSERT(list.head->next == NULL, "List head is incorrect");

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &na)), "Failed to remove");
    T_ASSERT(list.size == 0, "List size is incorrect");
    T_ASSERT(list.head == NULL, "List head is incorrect");
    T_ASSERT(list.tail == NULL, "List tail is incorrect");

}

int main(int argc, char* argv[]) {

    test_linked_list_init();
    test_linked_list_push_front();
    test_linked_list_push_back();
    test_linked_list_push_more();
    test_linked_list_pop();
    test_linked_list_insert_and_remove();

    return 0;
}
