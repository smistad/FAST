if(FAST_MODULE_WholeSlideImaging)
    message("-- Whole slide imaging module enabled")
    add_definitions(-DFAST_MODULE_WSI)
    if(WIN32)
        include(cmake/ExternalOpenSlide.cmake)
        list(APPEND LIBRARIES libopenslide.lib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/)
        fast_download_dependency(tiff
                4.3.0
                ee1d3157ee59d6e05cfad1c5ce7867ed0c4005693c67540397c99110c76db77c
                tiff.lib
        )
    elseif(APPLE)
        include(cmake/ExternalOpenSlide.cmake)

        set(CMAKE_FIND_FRAMEWORK LAST) # Avoid wrong TIFF header from Mono.framework being used
        # Get OpenSlide and TIFF from OS
        find_package(TIFF REQUIRED)
        find_package(JPEG REQUIRED)

        list(APPEND LIBRARIES ${TIFF_LIBRARIES} ${JPEG_LIBRARIES} libopenslide.dylib)
        list(APPEND FAST_INCLUDE_DIRS ${TIFF_INCLUDE_DIRS} ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/)
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
