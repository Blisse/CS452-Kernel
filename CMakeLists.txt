cmake_minimum_required(VERSION 2.8)

# verbose
set(CMAKE_VERBOSE_MAKEFILE OFF)

# cross compiler settings
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CROSSCOMPILING 1)
set(CMAKE_C_COMPILER_WORKS 1)

project("CS452" C ASM)

enable_testing()

# command-line parameters
option(LOCAL "LOCAL" OFF)

# build settings
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# libraries
set(LIB_BWIO "bwio")
set(LIB_RTOSC "rtosc")
set(LIB_OS "os")
set(LIB_KERNEL "kernel")
set(ALL_LIBS "-l${LIB_KERNEL} -l${LIB_OS} -l${LIB_RTOSC} -l${LIB_BWIO}")

# executables
set(EXE_RTOS "rtos.elf")
set(EXE_TEST_BUFFER "test_buffer")
set(EXE_TEST_STRING "test_string")

# includes
include_directories(
    "external"
    "inc"
    "inc/common"
    "inc/rt"
    "src/kernel"
    "src/os"
    )

# compile options

# -g: include hooks for gdb
# -c: only compile
# -fpic: emit position-independent code
# -Wall: report all warnings
# -Werror: report warnings as errors
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: use software for floating point
# -mapcs-32: always create a complete stack frame

if(LOCAL)
    set(CMAKE_C_COMPILER "/usr/bin/gcc")
    set(CMAKE_C_FLAGS "-Wall -Werror")
else()
    set(CMAKE_C_COMPILER "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc")
    set(CMAKE_C_FLAGS "-fPIC -O2 -Wall -Werror -mcpu=arm920t -mfloat-abi=soft")

    set(CMAKE_C_ARCHIVE_CREATE "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ar rcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ranlib <TARGET>")

    set(CMAKE_ASM_COMPILER "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as")
    set(CMAKE_ASM_FLAGS "-mcpu=arm920t -mapcs-32 -mfloat-abi=soft")
    set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <DEFINES> <FLAGS> -o <OBJECT> <SOURCE>")
    set(CMAKE_MODULE_LINKER_FLAGS "rcs")

    set(CMAKE_EXE_LINKER_FLAGS "-init main -Map rtos.map -N -T ${CMAKE_CURRENT_SOURCE_DIR}/src/orex.ld")
    set(CMAKE_EXE_LINKER_LIBS_DIRS "-L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -L${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(CMAKE_EXE_LINKER_LIBS_SEARCH "${ALL_LIBS} -lgcc")
    set(CMAKE_C_LINK_EXECUTABLE "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld <LINK_FLAGS> -o <TARGET> ${CMAKE_EXE_LINKER_LIBS_DIRS} <OBJECTS> ${CMAKE_EXE_LINKER_LIBS_SEARCH}")

    # get rid of rdynamic flag > https://cmake.org/Bug/view.php?id=9985
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
endif()

function(add_c_library LIBRARY_NAME LIBRARY_SOURCES LIBRARY_DEPENDENCIES)
    add_library("${LIBRARY_NAME}" STATIC ${LIBRARY_SOURCES})
    target_link_libraries("${LIBRARY_NAME}" "${LIBRARY_DEPENDENCIES}")
    if(argc EQUAL 3)
        set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C)
    else()
        set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C COMPILE_FLAGS "${ARGV3}")
    endif()
endfunction(add_c_library)

function(add_c_executable LIBRARY_NAME LIBRARY_SOURCES LIBRARY_DEPENDENCIES)
    add_executable("${LIBRARY_NAME}" "${LIBRARY_SOURCES}")
    target_link_libraries("${LIBRARY_NAME}" "${LIBRARY_DEPENDENCIES}")
    set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C)
endfunction(add_c_executable)

# build directories
add_subdirectory("external")
add_subdirectory("src")
add_subdirectory("tests")
