# OpenIGTLink module

if(FAST_MODULE_OpenIGTLink)
    if(WIN32)
        fast_download_dependency(OpenIGTLink
                2.1
                f3a024f501df6daa5b91edc75c77a8c46bab3574583ade49a7c83c801bb2fca3
                OpenIGTLink.lib
        )
    else()
        fast_download_dependency(OpenIGTLink
                2.1
                e88b83e9c6ea295a90c7926b0cbde26f86b6e38eb5682b0685c6c8fcea3711ed
                libOpenIGTLink.so
        )
    endif()
endif()
