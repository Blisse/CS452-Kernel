set(SRC_UTILITIES
    "assert.c"
    "buffer.c"
    "string.c"
    )

add_library(
    ${LIB_RTOSC}
    STATIC
    ${SRC_UTILITIES}
    )

target_link_libraries(
    ${LIB_RTOSC}
    ${LIB_BWIO}
    )

set_target_properties(
    ${LIB_RTOSC}
    PROPERTIES
    LINKER_LANGUAGE C
    )
