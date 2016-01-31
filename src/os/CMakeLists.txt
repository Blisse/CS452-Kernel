set(SRC_OS
    "init.c"
    "io.c"
    "nameserver.c"
    "object.c"
    "rtos.asm"
    "uart.c"
    )

add_c_library(
    "${LIB_OS}"
    "${SRC_OS}"
    "${LIB_RTOSC}"
    )
