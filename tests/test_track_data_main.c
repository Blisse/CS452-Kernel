#include <track/track_lib.h>
#include <rtosc/assert.h>

void test_trackb_lib_find_path() {
    TRACK_NODE nodes[TRACK_MAX];
    init_trackb(nodes);

    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathBuffer;
    RtCircularBufferInit(&pathBuffer, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    // trackb only has 140 nodes
    for (UINT i = 1; i < 140; i++)
    {
        for (UINT j = 10; j < 140; j++)
        {
            if (FindPath(nodes, sizeof(nodes) / sizeof(nodes[0]), &nodes[i], &nodes[j], &pathBuffer))
            {
                T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&pathBuffer, sizeof(RtCircularBufferSize(&pathBuffer)))));
            }
            else
            {
                T_ASSERT(RtCircularBufferSize(&pathBuffer) == 0);
            }
        }
    }
}

void test_tracka_lib_find_path() {
    TRACK_NODE nodes[TRACK_MAX];
    init_tracka(nodes);

    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathBuffer;
    RtCircularBufferInit(&pathBuffer, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    for (UINT i = 1; i < TRACK_MAX; i++)
    {
        for (UINT j = 1; j < TRACK_MAX; j++)
        {
            if (FindPath(nodes, sizeof(nodes) / sizeof(nodes[0]), &nodes[i], &nodes[j], &pathBuffer))
            {
                T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&pathBuffer, sizeof(RtCircularBufferSize(&pathBuffer)))));
            }
            else
            {
                T_ASSERT(RtCircularBufferSize(&pathBuffer) == 0);
            }
        }
    }
}

int main(int argc, char const *argv[])
{
    test_trackb_lib_find_path();
    test_tracka_lib_find_path();

    return 0;
}
