# Download and set up zip library

if(WIN32)
    fast_download_dependency(zip
            0.2.0
            63454fc97117e3f336b0fdc56308ddc7d3d4c2d775ee5d82916cf0d7f34e6bbc
            zip.lib
    )
else()
    fast_download_dependency(zip
            0.2.0
            4969fd278910a83efbada9aeb496b656135ba7009c0a08c3cfde3543d5b89a86
            libzip.a
    )
endif()
