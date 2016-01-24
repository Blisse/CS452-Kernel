
add_c_executable(${EXE_TEST_BUFFER} "test_buffer_main.c" "${LIB_RTOSC}")
add_c_executable(${EXE_TEST_SCHEDULER} "test_scheduler_main.c" "${LIB_RTOSC}")
add_c_executable(${EXE_TEST_STRING} "test_string_main.c" "${LIB_RTOSC}")

add_test(NAME ${EXE_TEST_BUFFER} COMMAND $<TARGET_FILE:${EXE_TEST_BUFFER}>)
add_test(NAME ${EXE_TEST_SCHEDULER} COMMAND $<TARGET_FILE:${EXE_TEST_SCHEDULER}>)
add_test(NAME ${EXE_TEST_STRING} COMMAND $<TARGET_FILE:${EXE_TEST_STRING}>)
