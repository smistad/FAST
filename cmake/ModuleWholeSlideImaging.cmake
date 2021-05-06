if(FAST_MODULE_WholeSlideImaging)
    message("-- Whole slide imaging module enabled")
    include(cmake/ExternalOpenSlide.cmake)
    if(WIN32)
        list(APPEND LIBRARIES libopenslide.lib)
        list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openslide/)

        # Build TIFF
        include(cmake/ExternalTIFF.cmake)
        set(TIFF_LIBRARY tiff.lib)
    else()
        find_package(OpenSlide REQUIRED)

        list(APPEND FAST_INCLUDE_DIRS ${OPENSLIDE_INCLUDE_DIRS})
        list(APPEND LIBRARIES ${OPENSLIDE_LIBRARIES})

        # Use TIFF from Qt
        set(TIFF_INCLUDE_DIRS ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/qtimageformats/src/3rdparty/libtiff/libtiff/)
        set(TIFF_LIBRARY libtiff.so)
     endif()
    message(STATUS "TIFF FOUND: ${TIFF_LIBRARY} ${TIFF_INCLUDE_DIRS}")
    list(APPEND FAST_INCLUDE_DIRS ${TIFF_INCLUDE_DIRS})
    list(APPEND FAST_LIBRARY_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/plugins/imageformats/)
    list(APPEND LIBRARIES ${TIFF_LIBRARY})
endif()
