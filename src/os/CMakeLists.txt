set(SRC_OS
    "rtos.asm"
    "init.c"
    "io.c"
    "nameserver.c"
    "object.c"
    "uart.c"
    )

add_c_library(
    "${LIB_OS}"
    "${SRC_OS}"
    "${LIB_RTOSC}"
    )
