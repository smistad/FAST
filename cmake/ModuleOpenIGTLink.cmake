# OpenIGTLink module

if(FAST_MODULE_OpenIGTLink)
    if(WIN32)
        fast_download_dependency(OpenIGTLink
                3.1
                c6eeb156cc445a7a0d0765868f17b8ca0230470b8db26dfd970d4d659bf70d2b
                OpenIGTLink.lib
        )
    elseif(APPLE)
        fast_download_dependency(OpenIGTLink
                2.1
                48f0eb32d45153a3bbb7d6529c4f64a41a618d7df3c6903f82c22b3788324020
                libOpenIGTLink.dylib
        )
    else()
        fast_download_dependency(OpenIGTLink
                3.1
                99d45eb03c5fddacdbd5ac424347ede2907394e180e6aba03fb335480cde858f
                libOpenIGTLink.so
        )
    endif()
endif()
