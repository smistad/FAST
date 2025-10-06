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

        fast_download_dependency(tiff
                4.3.0
                afc79045f37ee5a3a76912efb345935a7f8d7bf2ed30a19c38bd0816c551c3e7
                tiff.lib
        )
        find_package(JPEG REQUIRED)

        list(APPEND LIBRARIES ${TIFF_LIBRARIES} ${JPEG_LIBRARIES} libopenslide.dylib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/)
    else()
        fast_download_dependency(tiff
                4.3.0
                625dea4a6bf6460801e4d375348de135a8824f7bb675e1cd41bfaf0038cbf2ab
                libtiff.so
        )
        fast_download_dependency(openslide
                4.0.0
                200d1878bd28297309949bd4091c371380d9041cd9ec94e5336fbc3223f41445
                libopenslide.so
        )
    endif()
endif()
