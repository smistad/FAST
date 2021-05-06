if(FAST_MODULE_WholeSlideImaging)
    message("-- Whole slide imaging module enabled")
    include(cmake/ExternalOpenSlide.cmake)
    if(WIN32)
        # Build TIFF
        include(cmake/ExternalTIFF.cmake)
        list(APPEND LIBRARIES libopenslide.lib tiff.lib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/ ${TIFF_INCLUDE_DIRS})
    else()
        # Get OpenSlide and TIFF from OS
        find_package(OpenSlide REQUIRED)
        find_package(TIFF REQUIRED)

        list(APPEND LIBRARIES ${OPENSLIDE_LIBRARIES} ${TIFF_LIBRARIES})
        list(APPEND FAST_INCLUDE_DIRS ${OPENSLIDE_INCLUDE_DIRS} ${TIFF_INCLUDE_DIRS})
     endif()
    message(STATUS "TIFF FOUND: ${TIFF_LIBRARY} ${TIFF_LIBRARIES} ${TIFF_INCLUDE_DIRS}")
endif()
