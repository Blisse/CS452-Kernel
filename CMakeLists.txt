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
set(LIB_TRACK "track")
set(LIB_IO "io")
set(LIB_BWIO "bwio")
set(LIB_RTOSC "rtosc")
set(LIB_USER "user")
set(LIB_OS "os")
set(LIB_KERNEL "kernel")
set(ALL_LIBS "-l${LIB_KERNEL} -l${LIB_OS} -l${LIB_USER} -l${LIB_RTOSC} -l${LIB_BWIO} -l${LIB_TRACK}")

# includes
include_directories(
    "ext"
    "inc/rt"
    "inc/rtos"
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
    set(CMAKE_C_FLAGS "-Wall -Werror -std=gnu99")
else()
    set(CMAKE_C_COMPILER "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc")
    set(CMAKE_C_FLAGS "-fPIC -Wall -Werror -mcpu=arm920t -mfloat-abi=soft -std=gnu99")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -DNLOCAL")
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -DNLOCAL")

    set(CMAKE_C_ARCHIVE_CREATE "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ar rcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ranlib <TARGET>")

    set(CMAKE_ASM_COMPILER "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as")
    set(CMAKE_ASM_COMPILE_FLAGS "-mcpu=arm920t -mapcs-32 -mfloat-abi=soft")
    set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <DEFINES> ${CMAKE_ASM_COMPILE_FLAGS} -o <OBJECT> <SOURCE>")
    set(CMAKE_MODULE_LINKER_FLAGS "rcs")

    set(CMAKE_EXE_LINKER_FLAGS "-init main -Map rtos.map -N -T ${CMAKE_CURRENT_SOURCE_DIR}/src/orex.ld")
    set(CMAKE_EXE_LINKER_LIBS_DIRS "-L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -L${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(CMAKE_EXE_LINKER_LIBS_SEARCH "${ALL_LIBS} ${ALL_LIBS} -lgcc")
    set(CMAKE_C_LINK_EXECUTABLE "/u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld <LINK_FLAGS> -o <TARGET> <OBJECTS> ${CMAKE_EXE_LINKER_LIBS_DIRS} ${CMAKE_EXE_LINKER_LIBS_SEARCH}")

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
endfunction()

function(add_c_executable EXEC_NAME EXEC_SOURCES EXEC_DEPENDENCIES)
    add_executable("${EXEC_NAME}" "${EXEC_SOURCES}")
    target_link_libraries("${EXEC_NAME}" "${EXEC_DEPENDENCIES}")
    set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)
    if (NOT LOCAL)
        set(TFTP_EXEC_PATH "/u/cs452/tftp/ARM/v6lai/shared/${EXEC_NAME}")
        add_custom_command(
            TARGET "${EXEC_NAME}"
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXEC_NAME}"
                "${TFTP_EXEC_PATH}"
            COMMENT "Copying ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXEC_NAME} to ${TFTP_EXEC_PATH}"
            )
        add_custom_command(
            TARGET "${EXEC_NAME}"
            POST_BUILD
            COMMAND chmod 777 ${TFTP_EXEC_PATH}
            COMMENT "chmod 777 ${TFTP_EXEC_PATH}"
            )
    endif()
endfunction()

# build directories
add_subdirectory("ext")
add_subdirectory("src")
add_subdirectory("tests")
