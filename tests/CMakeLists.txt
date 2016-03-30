set(EXE_TEST_SCHEDULER "tscheduler")
set(EXE_TEST_BITSET "tbitset")
set(EXE_TEST_BUFFER "tbuffer")
set(EXE_TEST_LINKED_LIST "tlinkedlist")
set(EXE_TEST_STRING "tstring")
set(EXE_TEST_TRACK "ttrack")
set(EXE_TEST_TASK_DESCRIPTOR "ttaskdescriptor")
set(EXE_TEST_PRIORITY_QUEUE "tpriorityqueue")

function(add_c_test TEST_NAME TEST_MAIN TEST_DEPENDENCIES)
    add_c_executable(${TEST_NAME} "${TEST_MAIN}" "${TEST_DEPENDENCIES}")
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE:${TEST_NAME}>)
endfunction()

include_directories(
    ${CMAKE_SOURCE_DIR}/src/kernel
    ${CMAKE_SOURCE_DIR}/src/os
    )

add_c_test("${EXE_TEST_BITSET}" "test_bitset_main.c" "${LIB_RTOSC}")
add_c_test("${EXE_TEST_BUFFER}" "test_buffer_main.c" "${LIB_RTOSC}")
add_c_test("${EXE_TEST_STRING}" "test_string_main.c" "${LIB_RTOSC}")
add_c_test("${EXE_TEST_LINKED_LIST}" "test_linked_list_main.c" "${LIB_RTOSC}")
add_c_test("${EXE_TEST_PRIORITY_QUEUE}" "test_priority_queue_main.c" "${LIB_RTOSC}")
add_c_test("${EXE_TEST_TRACK}" "test_track_data_main.c" "${LIB_TRACK}")

if (NOT LOCAL)

    add_c_test("${EXE_TEST_SCHEDULER}" "test_scheduler_main.c" "${LIB_KERNEL}")
    add_c_test("${EXE_TEST_TASK_DESCRIPTOR}" "test_task_descriptor_main.c" "${LIB_KERNEL}")

endif()
