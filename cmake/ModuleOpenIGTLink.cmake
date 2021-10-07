# OpenIGTLink module

if(FAST_MODULE_OpenIGTLink)
    if(WIN32)
        fast_download_dependency(OpenIGTLink
                2.1
                8dd74058444f4dc903dee4a400ae13bc84e1d0f93255390ace034da988553296
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
                2.1
                404b6cb26923c9361d3ffb53210abc573696258e08c57ffed3d38aa9857f793b
                libOpenIGTLink.so
        )
    endif()
endif()
