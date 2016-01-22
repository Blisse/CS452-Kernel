set(SRC_KERNEL
    "arm.asm"
    "kernel.c"
    "scheduler.c"
    "task.c"
    "trap.asm"
    )

add_library(
    ${LIB_KERNEL}
    ${SRC_KERNEL}
    )

target_link_libraries(
    ${LIB_KERNEL}
    ${LIB_BWIO}
    ${LIB_OS}
    ${LIB_RTOSC}
    )
