# OpenIGTLink module

if(FAST_MODULE_OpenIGTLink)
    if(WIN32)
        fast_download_dependency(OpenIGTLink
                2.1
                8dd74058444f4dc903dee4a400ae13bc84e1d0f93255390ace034da988553296
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
