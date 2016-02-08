set(SRC_OS
    ${CMAKE_CURRENT_SOURCE_DIR}/rtos.asm
    ${CMAKE_CURRENT_SOURCE_DIR}/clock_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/idle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/name_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/object.c
    ${CMAKE_CURRENT_SOURCE_DIR}/uart.c
    )

add_subdirectory("tasks")

add_c_library(
    "${LIB_OS}"
    "${SRC_OS}"
    "${LIB_RTOSC}"
    )
