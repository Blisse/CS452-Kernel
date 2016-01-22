set(SRC_OS
    "init.c"
    "io.c"
    "object.c"
    "rtos.asm"
    "uart.c"
    )

add_library(
    ${LIB_OS}
    STATIC
    ${SRC_OS}
    )

target_link_libraries(
    ${LIB_OS}
    ${LIB_RTOSC}
    )

set_target_properties(
    ${LIB_OS}
    PROPERTIES
    LINKER_LANGUAGE C
    )
