cmake_minimum_required(VERSION 2.8)

# verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)

# cross compiler settings
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_C_COMPILER_WORKS 1)

project("CS452" C ASM)

# compiler flags
option(LOCAL "LOCAL" OFF)

if (LOCAL)
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_C_FLAGS "-Wall")
    set(LDFLAGS "-init main -Map a0.map -N -T ${CMAKE_CURRENT_SOURCE_DIR}/src/orex.ld")
    set(CMAKE_ASM_FLAGS "-mapcs-32")
else()
    set(CMAKE_C_COMPILER "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc")
    set(CMAKE_C_FLAGS "-fPIC -Wall -mcpu=arm920t -msoft-float")
    set(LDFLAGS "-init main -Map a0.map -N -T ${CMAKE_CURRENT_SOURCE_DIR}/src/orex.ld")
    set(CMAKE_ASM_FLAGS "-mcpu=arm920t -mapcs-32")

    set(CMAKE_C_LINK_EXECUTABLE "ld ${LDFLAGS} <OBJECTS> -o <TARGET>")

    link_directories("/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2")
endif()

# -g: include hooks for gdb
# -c: only compile
# -fpic: emit position-independent code
# -Wall: report all warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: use software for floating point
# -mapcs-32: always create a complete stack frame

set(CMAKE_AR_FLAGS "rcs")

# get rid of rdynamic flag > https://cmake.org/Bug/view.php?id=9985
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

# output variables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(EXE_KERNEL "cs452_kernel")
set(EXE_TEST_BUFFER "test_buffer")
set(EXE_TEST_STRING "test_string")

set(LIB_BWIO "bwio")
set(LIB_KERNEL "kernel")
set(LIB_OS "os")
set(LIB_UTILITIES "utilities")

# include directories
include_directories(
    "external"
    "inc"
    "inc/common"
    "inc/rt"
    "src/kernel"
    "src/os"
    "src/utilities"
    )

# build directories
add_subdirectory("external")
add_subdirectory("src/utilities")
add_subdirectory("src/kernel")
add_subdirectory("src/os")

# executables
add_executable(${EXE_KERNEL} "src/kernel/main.c")
target_link_libraries(${EXE_KERNEL} ${LIB_KERNEL})
set_target_properties(${LIB_KERNEL} PROPERTIES LINKER_LANGUAGE C)

add_executable(${EXE_TEST_BUFFER} "src/tests/test_buffer_main.c")
target_link_libraries(${EXE_TEST_BUFFER} ${LIB_UTILITIES})
set_target_properties(${EXE_TEST_BUFFER} PROPERTIES LINKER_LANGUAGE C)

add_executable(${EXE_TEST_STRING} "src/tests/test_string_main.c")
target_link_libraries(${EXE_TEST_STRING} ${LIB_UTILITIES})
set_target_properties(${EXE_TEST_STRING} PROPERTIES LINKER_LANGUAGE C)


# test
enable_testing()

add_test(NAME ${EXE_TEST_BUFFER} COMMAND $<TARGET_FILE:${EXE_TEST_BUFFER}>)
add_test(NAME ${EXE_TEST_STRING} COMMAND $<TARGET_FILE:${EXE_TEST_STRING}>)