set(EXE_RTOS "rtos.elf")

set(SRC_KERNEL
    ${CMAKE_CURRENT_SOURCE_DIR}/arm.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/cache.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ipc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/performance.c
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall.c
    ${CMAKE_CURRENT_SOURCE_DIR}/task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/task_descriptor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/trap.asm
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
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
