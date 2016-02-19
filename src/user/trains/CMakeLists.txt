set(SRC_USER
    ${SRC_USER}
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/input_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/switch_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/train_server.c
    PARENT_SCOPE
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
