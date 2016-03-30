set(SRC_TRACK
    "track_data.c"
    "track_lib.c"
    )

add_c_library(
    "${LIB_TRACK}"
    "${SRC_TRACK}"
    "${LIB_RTOSC}"
    )
