set(SRC_USER
    ${SRC_USER}
    ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/calibration.c
    ${CMAKE_CURRENT_SOURCE_DIR}/location_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/physics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sensor_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/switch_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/track_reserver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/track_server.c
    ${CMAKE_CURRENT_SOURCE_DIR}/train_server.c
    PARENT_SCOPE
    )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
