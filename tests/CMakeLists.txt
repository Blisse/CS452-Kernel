enable_testing()

add_executable(${EXE_TEST_BUFFER} "test_buffer_main.c")
target_link_libraries(${EXE_TEST_BUFFER} ${LIB_RTOSC})
set_target_properties(${EXE_TEST_BUFFER} PROPERTIES LINKER_LANGUAGE C)

add_executable(${EXE_TEST_STRING} "test_string_main.c")
target_link_libraries(${EXE_TEST_STRING} ${LIB_RTOSC})
set_target_properties(${EXE_TEST_STRING} PROPERTIES LINKER_LANGUAGE C)

add_test(NAME ${EXE_TEST_BUFFER} COMMAND $<TARGET_FILE:${EXE_TEST_BUFFER}>)
add_test(NAME ${EXE_TEST_STRING} COMMAND $<TARGET_FILE:${EXE_TEST_STRING}>)
