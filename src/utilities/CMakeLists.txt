set(SRC_UTILITIES
    "assert.c"
    "buffer.c"
    "string.c"
    )

add_library(
    ${LIB_UTILITIES}
    STATIC
    ${SRC_UTILITIES}
    )

target_link_libraries(
    ${LIB_UTILITIES}
    ${LIB_BWIO}
    )

set_target_properties(
    ${LIB_UTILITIES}
    PROPERTIES
    LINKER_LANGUAGE C
    )
