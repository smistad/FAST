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
                9dbe0a096b6b036a34491f1401ad9d7e6c9e218ecd8af7a7cc558690f97c84ce
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
