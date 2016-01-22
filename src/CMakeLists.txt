if (NOT LOCAL)

add_subdirectory("kernel")
add_subdirectory("os")

add_executable(${EXE_RTOS} "kernel/main.c")
target_link_libraries(${EXE_RTOS} ${LIB_KERNEL} ${LIB_OS})
set_target_properties(${LIB_KERNEL} PROPERTIES LINKER_LANGUAGE C)

endif()
