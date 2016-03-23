set(SRC_USER
    )

add_subdirectory("io")
add_subdirectory("trains")
add_subdirectory("users")

add_c_library(
    "${LIB_USER}"
    "${SRC_USER}"
    "${LIB_RTOSC} ${LIB_BWIO}"
    )
