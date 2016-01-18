set(SRC_KERNEL
    "scheduler.c"
    "swi.asm"
    )

add_library(
    ${LIB_KERNEL}
    ${SRC_KERNEL}
    )

target_link_libraries(
    ${LIB_KERNEL}
    ${LIB_BWIO}
    ${LIB_OS}
    ${LIB_UTILITIES}
    )
