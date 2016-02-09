#include <rtosc/linked_list.h>

#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include <rtosc/buffer.h>
#include <rt.h>

#define TEST_BUFFER_SIZE 8

void test_linked_list_init() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list);

    T_ASSERT(list.size == 0);
    T_ASSERT(list.head == NULL);
    T_ASSERT(list.tail == NULL);
}

void test_linked_list_push_front() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list);

    INT i = 1;
    RT_LINKED_LIST_NODE na;
    na.data = &i;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)));

    T_ASSERT(list.size == 1);
    T_ASSERT(list.head == &na);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->previous == NULL);
    T_ASSERT(list.head->next == NULL);
    T_ASSERT(list.head->data == &i);
    T_ASSERT(list.tail->data == &i);
    T_ASSERT(list.tail->previous == NULL);
    T_ASSERT(list.tail->next == NULL);

    INT j = 1;
    RT_LINKED_LIST_NODE nb;
    nb.data = &j;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)));

    T_ASSERT(list.size == 2);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->previous == NULL);
    T_ASSERT(list.head->next == &na);
    T_ASSERT(list.head->data == &j);
    T_ASSERT(list.tail->previous == &nb);
    T_ASSERT(list.tail->next == NULL);
    T_ASSERT(list.tail->data == &i);

    INT k = 1;
    RT_LINKED_LIST_NODE nc;
    nc.data = &k;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nc)));

    T_ASSERT(list.size == 3);
    T_ASSERT(list.head == &nc);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->previous == NULL);
    T_ASSERT(list.head->next == &nb);
    T_ASSERT(list.head->data == &k);
    T_ASSERT(list.tail->previous == &nb);
    T_ASSERT(list.tail->next == NULL);
    T_ASSERT(list.tail->data == &i);
}

void test_linked_list_push_back() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list);

    INT i = 1;
    RT_LINKED_LIST_NODE na;
    na.data = &i;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &na)));

    T_ASSERT(list.size == 1);
    T_ASSERT(list.head == &na);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->previous == NULL);
    T_ASSERT(list.head->next == NULL);
    T_ASSERT(list.head->data == &i);
    T_ASSERT(list.tail->previous == NULL);
    T_ASSERT(list.tail->next == NULL);
    T_ASSERT(list.tail->data == &i);

    INT j = 1;
    RT_LINKED_LIST_NODE nb;
    nb.data = &j;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nb)));

    T_ASSERT(list.size == 2);
    T_ASSERT(list.head == &na);
    T_ASSERT(list.tail == &nb);
    T_ASSERT(list.head->previous == NULL);
    T_ASSERT(list.head->next == &nb);
    T_ASSERT(list.head->data == &i);
    T_ASSERT(list.tail->previous == &na);
    T_ASSERT(list.tail->next == NULL);
    T_ASSERT(list.tail->data == &j);

    INT k = 1;
    RT_LINKED_LIST_NODE nc;
    nc.data = &k;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nc)));

    T_ASSERT(list.size == 3);
    T_ASSERT(list.head == &na);
    T_ASSERT(list.tail == &nc);
    T_ASSERT(list.head->previous == NULL);
    T_ASSERT(list.head->next == &nb);
    T_ASSERT(list.head->data == &i);
    T_ASSERT(list.tail->data == &k);
    T_ASSERT(list.tail->previous == &nb);
    T_ASSERT(list.tail->next == NULL);
}

void test_linked_list_push_more() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list);

    T_ASSERT(RtLinkedListIsEmpty(&list));

    RT_LINKED_LIST_NODE na;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &na)));
    RT_LINKED_LIST_NODE nb;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nb)));
    RT_LINKED_LIST_NODE nc;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nc)));
    RT_LINKED_LIST_NODE nd;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nd)));

    T_ASSERT(list.size == 4);

    T_ASSERT(!RtLinkedListIsEmpty(&list));

    RT_LINKED_LIST_NODE ne;
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &ne)));
}

void test_linked_list_pop() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list);

    RT_LINKED_LIST_NODE na;
    RT_LINKED_LIST_NODE nb;
    RT_LINKED_LIST_NODE nc;
    RT_LINKED_LIST_NODE nd;

    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &na)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nb)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nc)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushBack(&list, &nd)));

    T_ASSERT(list.size == 4);

    RT_LINKED_LIST_NODE* ne;
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &nd);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &nc);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &nb);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &na);
    T_ASSERT(RT_FAILURE(RtLinkedListPeekAndPopBack(&list, &ne)));

    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nc)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nd)));

    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &na);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &nb);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &nc);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopBack(&list, &ne)));
    T_ASSERT(ne == &nd);
    T_ASSERT(RT_FAILURE(RtLinkedListPeekAndPopBack(&list, &ne)));


    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)));

    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)));
    T_ASSERT(ne == &nb);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)));
    T_ASSERT(ne == &na);

    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &na)));
    T_ASSERT(RT_SUCCESS(RtLinkedListPushFront(&list, &nb)));

    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)));
    T_ASSERT(ne == &nb);
    T_ASSERT(RT_SUCCESS(RtLinkedListPeekAndPopFront(&list, &ne)));
    T_ASSERT(ne == &na);

    T_ASSERT(RT_FAILURE(RtLinkedListPeekAndPopFront(&list, &ne)));
}

void test_linked_list_insert_and_remove() {
    RT_LINKED_LIST list;
    RtLinkedListInit(&list);

    RT_LINKED_LIST_NODE na;
    RT_LINKED_LIST_NODE nb;
    RT_LINKED_LIST_NODE nc;
    RT_LINKED_LIST_NODE nd;
    RT_LINKED_LIST_NODE ne;
    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, NULL, NULL, &na)));
    T_ASSERT(list.size == 1);
    T_ASSERT(list.head == &na);
    T_ASSERT(list.tail == &na);

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, NULL, &na, &nb)));
    T_ASSERT(list.size == 2);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &na);

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, &nb, &na, &nc)));
    T_ASSERT(list.size == 3);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &nc);

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, &nc, &na, &nd)));
    T_ASSERT(list.size == 4);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &nc);
    T_ASSERT(list.head->next->next == &nd);

    T_ASSERT(RT_SUCCESS(RtLinkedListInsertBetween(&list, &na, NULL, &ne)));
    T_ASSERT(list.size == 5);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &ne);
    T_ASSERT(list.head->next == &nc);
    T_ASSERT(list.head->next->next == &nd);
    T_ASSERT(list.head->next->next->next == &na);
    T_ASSERT(list.head->next->next->next->next == &ne);


    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &ne)));
    T_ASSERT(list.size == 4);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &nc);
    T_ASSERT(list.head->next->next == &nd);
    T_ASSERT(list.head->next->next->next == &na);

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &nc)));
    T_ASSERT(list.size == 3);
    T_ASSERT(list.head == &nb);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &nd);
    T_ASSERT(list.head->next->next == &na);

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &nb)));
    T_ASSERT(list.size == 2);
    T_ASSERT(list.head == &nd);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &na);

    T_ASSERT(RT_FAILURE(RtLinkedListRemove(&list, &nb)));
    T_ASSERT(list.size == 2);
    T_ASSERT(list.head == &nd);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == &na);

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &nd)));
    T_ASSERT(list.size == 1);
    T_ASSERT(list.head == &na);
    T_ASSERT(list.tail == &na);
    T_ASSERT(list.head->next == NULL);

    T_ASSERT(RT_SUCCESS(RtLinkedListRemove(&list, &na)));
    T_ASSERT(list.size == 0);
    T_ASSERT(list.head == NULL);
    T_ASSERT(list.tail == NULL);

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
