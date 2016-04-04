set(SRC_USER
    ${SRC_USER}
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/calibration.c
    PARENT_SCOPE
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
