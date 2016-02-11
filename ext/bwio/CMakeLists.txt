set(SRC_BWIO
    "bwio.c"
    )

add_c_library(
    "${LIB_BWIO}"
    "${SRC_BWIO}"
    ""
    "-O0"
    )
