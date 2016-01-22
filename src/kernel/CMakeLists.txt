set(SRC_KERNEL
    "arm.asm"
    "kernel.c"
    "scheduler.c"
    "task.c"
    "trap.asm"
    )

add_c_library(
    "${LIB_KERNEL}"
    "${SRC_KERNEL}"
    "${LIB_BWIO} ${LIB_RTOSC}"
    )

add_c_executable(
    "${EXE_RTOS}"
    "main.c"
    "${LIB_KERNEL}"
    )
