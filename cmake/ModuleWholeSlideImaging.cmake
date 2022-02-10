if(FAST_MODULE_WholeSlideImaging)
    message("-- Whole slide imaging module enabled")
    include(cmake/ExternalOpenSlide.cmake)
    add_definitions(-DFAST_MODULE_WSI)
    if(WIN32)
        # Build TIFF
        include(cmake/ExternalTIFF.cmake)
        list(APPEND LIBRARIES libopenslide.lib tiff.lib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/ ${TIFF_INCLUDE_DIRS})
        message(STATUS "TIFF FOUND: ${TIFF_LIBRARY} ${TIFF_LIBRARIES} ${TIFF_INCLUDE_DIRS}")
    elseif(APPLE)
        # Get OpenSlide and TIFF from OS
        find_package(OpenSlide REQUIRED)
        find_package(TIFF REQUIRED)
        find_package(JPEG REQUIRED)

        list(APPEND LIBRARIES ${OPENSLIDE_LIBRARIES} ${TIFF_LIBRARIES} ${JPEG_LIBRARIES})
        list(APPEND FAST_INCLUDE_DIRS ${OPENSLIDE_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS})
        message(STATUS "TIFF FOUND: ${TIFF_LIBRARY} ${TIFF_LIBRARIES} ${TIFF_INCLUDE_DIRS}")
    else()
        fast_download_dependency(tiff
                4.3.0
                625dea4a6bf6460801e4d375348de135a8824f7bb675e1cd41bfaf0038cbf2ab
                libtiff.so
        )
        fast_download_dependency(openslide
                3.4.1
                a39521846a132b0c375047150e108e5ccbaa1d4f1d8cc36350bd36fb60889841
                libopenslide.so
        )
    endif()
endif()
