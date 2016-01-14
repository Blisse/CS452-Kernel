set(SRC_OS
    "io.c"
    "object.c"
    "uart.c"
    )

add_library(
    ${LIB_OS}
    STATIC
    ${SRC_OS}
    )

target_link_libraries(
    ${LIB_OS}
    ${LIB_UTILITIES}
    )

set_target_properties(
    ${LIB_OS}
    PROPERTIES
    LINKER_LANGUAGE C
    )
