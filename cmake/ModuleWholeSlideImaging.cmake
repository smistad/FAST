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

        list(APPEND LIBRARIES ${OPENSLIDE_LIBRARIES} ${TIFF_LIBRARIES})
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
                6cd181e43057d8f67b31a71239fd3295a3ee43baaaedce827b8570ba95478ea2
                libopenslide.so
        )
    endif()
endif()
