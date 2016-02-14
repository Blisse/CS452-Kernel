set(SRC_OS
    ${CMAKE_CURRENT_SOURCE_DIR}/rtos.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/clock_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/courier.c
    ${CMAKE_CURRENT_SOURCE_DIR}/idle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/io_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/io_read.c
    ${CMAKE_CURRENT_SOURCE_DIR}/io_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/io_write.c
    ${CMAKE_CURRENT_SOURCE_DIR}/name_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shutdown.c
    ${CMAKE_CURRENT_SOURCE_DIR}/uart.c
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

add_c_library(
    "${LIB_OS}"
    "${SRC_OS}"
    "${LIB_RTOSC} ${LIB_BWIO}"
    )
