# Download and set up zip library

if(WIN32)
    fast_download_dependency(zip
            0.2.0
            f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
            zip.lib
    )
else()
    fast_download_dependency(zip
            0.2.0
            4969fd278910a83efbada9aeb496b656135ba7009c0a08c3cfde3543d5b89a86
            libzip.a
    )
endif()
