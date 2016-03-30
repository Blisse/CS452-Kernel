#include <track/track_lib.h>
#include <rtosc/assert.h>

void trackb_lib_find_path() {
    TRACK_NODE nodes[TRACK_MAX];
    init_trackb(nodes);

    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathBuffer;
    RtCircularBufferInit(&pathBuffer, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    for (UINT i = 1; i < 140; i++) // trackb only has 140 nodes
    {
        T_ASSERT(FindPath(nodes, sizeof(nodes) / sizeof(nodes[0]), &nodes[0], &nodes[i], &pathBuffer) == TRUE);
        T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&pathBuffer, RtCircularBufferSize(&pathBuffer))));
    }
}

void tracka_lib_find_path() {
    TRACK_NODE nodes[TRACK_MAX];
    init_tracka(nodes);

    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathBuffer;
    RtCircularBufferInit(&pathBuffer, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    for (UINT i = 1; i < TRACK_MAX; i++)
    {
        T_ASSERT(FindPath(nodes, sizeof(nodes) / sizeof(nodes[0]), &nodes[0], &nodes[i], &pathBuffer) == TRUE);
        T_ASSERT(RT_SUCCESS(RtCircularBufferPop(&pathBuffer, RtCircularBufferSize(&pathBuffer))));
    }
}

int main(int argc, char const *argv[])
{
    tracka_lib_find_path();
    trackb_lib_find_path();

    return 0;
}
