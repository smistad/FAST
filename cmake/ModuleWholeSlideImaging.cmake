if(FAST_MODULE_WholeSlideImaging)
    message("-- Whole slide imaging module enabled")
    add_definitions(-DFAST_MODULE_WSI)
    if(WIN32)
        include(cmake/ExternalOpenSlide.cmake)
        list(APPEND LIBRARIES libopenslide.lib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/)
        fast_download_dependency(tiff
                4.3.0
                873569ee85ba8dcd3cc4225d93fabf0ad588e71db444cb1cad91d991f6a635ec
                tiff.lib
        )
    elseif(APPLE)
        include(cmake/ExternalOpenSlide.cmake)
        list(APPEND LIBRARIES libopenslide.dylib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/)
        if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            fast_download_dependency(tiff
                    4.3.0
                    a2ae30d8d2252959a4062262648e8293f45dc9f15a86e57ccbbb7fc2625543e0
                    tiff.dylib
            )
        else()
            fast_download_dependency(tiff
                    4.3.0
                    afc79045f37ee5a3a76912efb345935a7f8d7bf2ed30a19c38bd0816c551c3e7
                    tiff.dylib
            )
        endif()

    else()
        fast_download_dependency(tiff
                4.3.0
                625dea4a6bf6460801e4d375348de135a8824f7bb675e1cd41bfaf0038cbf2ab
                libtiff.so
        )
        fast_download_dependency(openslide
                4.0.0
                367d8ec527358a293a71830d88fa467c7eef04a928afc8ce89db254b1ef9e004
                libopenslide.so
        )
    endif()
endif()
