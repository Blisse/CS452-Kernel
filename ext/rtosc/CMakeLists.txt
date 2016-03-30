set(SRC_UTILITIES
    "assert.c"
    "buffer.c"
    "linked_list.c"
    "math.c"
    "priority_queue.c"
    "stdlib.c"
    "string.c"
    )

add_c_library(
    "${LIB_RTOSC}"
    "${SRC_UTILITIES}"
    "${LIB_BWIO}"
    )
