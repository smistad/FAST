# Download dependencies required for various image compression
if(WIN32)
    fast_download_dependency(jpegxl
        0.11.0
        51aec03201152d99ca9c6a561b3c28e0e1c1043d9488c9bd7b590b03412347e0
        jxl.lib jxl_threads.lib
    )
elseif(APPLE)
    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        fast_download_dependency(jpegxl
            0.11.0
            dc05ced17948ed02e2c646e9480758ed439aa061d70c9d820df3e3239ea1dadd
            jxl.dylib jxl_threads.dylib
        )
    else()
        fast_download_dependency(jpegxl
            0.11.0
            f743d0fb5cdb6b8d63028d14a633c5ae250dbeca8b4577ad9c55b98a024035b3
            jxl.dylib jxl_threads.dylib
        )
    endif()
else()
    fast_download_dependency(jpegxl
        0.11.0
        a9bdb10ceddceb57892e9f6526cd8f62ad4a469e9f6941a687f8af6eb425b172
        libjxl.so libjxl_threads.so
    )
endif()
