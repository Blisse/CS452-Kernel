set(SRC_BWIO
    "bwio.c"
    )

add_library(
    ${LIB_BWIO}
    STATIC
    ${SRC_BWIO}
    )

set_target_properties(
    ${LIB_BWIO}
    PROPERTIES
    LINKER_LANGUAGE C
    COMPILE_FLAGS "-O0"
    )
