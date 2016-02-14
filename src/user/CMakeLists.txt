set(SRC_USER
    )

add_subdirectory("users")
add_subdirectory("trains")

add_c_library(
    "${LIB_USER}"
    "${SRC_USER}"
    "${LIB_RTOSC} ${LIB_BWIO}"
    )
