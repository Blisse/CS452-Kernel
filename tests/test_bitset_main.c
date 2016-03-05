#include <bwio/bwio.h>
#include <rtosc/assert.h>
#include <rtosc/bitset.h>

void test_bitset() {

    INT n = 0;
    INT n1 = 1;
    INT n2 = 2;
    INT n3 = 3;
    INT n4 = 4;
    INT n15 = 15;

    for (UINT i = 0; i < 8; i++) {
        T_ASSERT(!BIT_CHECK(n,i));
    }

    T_ASSERT(BIT_CHECK(n1,0));

    T_ASSERT(BIT_CHECK(n2,1));

    T_ASSERT(BIT_CHECK(n3,0));
    T_ASSERT(BIT_CHECK(n3,1));

    T_ASSERT(BIT_CHECK(n4,2));

    T_ASSERT(BIT_CHECK(n15,0));
    T_ASSERT(BIT_CHECK(n15,1));
    T_ASSERT(BIT_CHECK(n15,2));
    T_ASSERT(BIT_CHECK(n15,3));
    T_ASSERT(!BIT_CHECK(n15,4));
}

int main(int argc, char* argv[]) {

    test_bitset();

    return 0;
}
