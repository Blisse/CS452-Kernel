set(SRC_USER
    ${SRC_USER}
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/display.c
    ${CMAKE_CURRENT_SOURCE_DIR}/input_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/performance.c
    PARENT_SCOPE
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
