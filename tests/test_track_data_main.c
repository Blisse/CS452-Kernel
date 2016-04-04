#include <track/track_lib.h>
#include <rtosc/assert.h>

void test_track_print(RT_CIRCULAR_BUFFER* path) {
    for (UINT i = 0; i < (RtCircularBufferSize(path) / sizeof(TRACK_NODE*)); i++) {
        TRACK_NODE* node;
        VERIFY(RT_SUCCESS(RtCircularBufferElementAt(path, i, &node, sizeof(node))));
        PRINTF("%s ", node->name);
    }
    PRINTF("\n");
}

void test_trackb_lib_find_path() {
    TRACK_NODE nodes[TRACK_MAX];
    init_trackb(nodes);

    TRACK_NODE* underlyingPathBuffer[TRACK_MAX];
    RT_CIRCULAR_BUFFER pathBuffer;
    RtCircularBufferInit(&pathBuffer, underlyingPathBuffer, sizeof(underlyingPathBuffer));

    // trackb only has 140 nodes
    for (UINT i = 1; i < 140; i++)
    {
        for (UINT j = 1; j < 140; j++)
        {
            if (nodes[i].type == NODE_SENSOR && nodes[j].type == NODE_SENSOR)
            {
                PRINTF("find [%s, %s] path: ", nodes[i].name, nodes[j].name);
                BOOLEAN success  = FindPath(nodes, sizeof(nodes) / sizeof(nodes[0]), &nodes[i], &nodes[j], &pathBuffer);
                if (success) {
                    test_track_print(&pathBuffer);
                } else {
                    PRINTF("no path possible without reverse\n");
                }
                T_ASSERT(RT_SUCCESS(RtCircularBufferClear(&pathBuffer)));
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
            if (nodes[i].type == NODE_SENSOR && nodes[j].type == NODE_SENSOR)
            {
                BOOLEAN success = FindPath(nodes, sizeof(nodes) / sizeof(nodes[0]), &nodes[i], &nodes[j], &pathBuffer);
                #ifndef NLOCAL
                PRINTF("find [%s, %s] path: ", nodes[i].name, nodes[j].name);
                if (success) {
                    test_track_print(&pathBuffer);
                } else {
                    PRINTF("no path possible without reverse\n");
                }
                #endif
                UNREFERENCED_PARAMETER(success);
                T_ASSERT(RT_SUCCESS(RtCircularBufferClear(&pathBuffer)));
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
