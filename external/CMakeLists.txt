set(SRC_BWIO
    "bwio/bwio.c"
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
    )
