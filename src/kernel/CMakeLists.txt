set(EXE_RTOS "rtos.elf")

set(SRC_KERNEL
    "arm.asm"
    "cache.asm"
    "interrupt.asm"
    "interrupt.c"
    "ipc.c"
    "kernel.asm"
    "kernel.c"
    "scheduler.c"
    "stack.c"
    "syscall.c"
    "task.c"
    "task_descriptor.c"
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
