# CMake setup for building a package of FAST (debian on Unix, NSIS installer for windows)

set(CPACK_PACKAGE_NAME "FAST")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FAST is an open-source cross-platform framework with the main goal of making it easier to do high performance processing and visualization of medical images on heterogeneous systems (CPU+GPU).")
set(CPACK_PACKAGE_VENDOR "Erik Smistad")
set(CPACK_PACKAGE_CONTACT "Erik Smistad ersmistad@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME "fast")

if(WIN32 AND NOT UNIX)
    ## Windows
else()
    ## UNIX

    # Get distro name and version
    find_program(LSB_RELEASE_EXEC lsb_release)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
        OUTPUT_VARIABLE DISTRO_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(TOLOWER ${DISTRO_NAME} DISTRO_NAME)
    execute_process(COMMAND ${LSB_RELEASE_EXEC} -rs
        OUTPUT_VARIABLE DISTRO_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(CPACK_GENERATOR "DEB")
    # Select components to avoid some cmake leftovers from built dependencies
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE https://eriksmistad.no/fast/)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/opt")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

    # Create data package (optional)
    option(FAST_BUILD_DATA_DEB_PACKAGE "Build a separate data debian package for FAST" OFF)
    if(FAST_BUILD_DATA_DEB_PACKAGE)
        message("-- Building of debian data package enabled")
        install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/
            DESTINATION fast/data
            COMPONENT data
        )
        set(CPACK_COMPONENTS_ALL fast data)
        set(CPACK_DEBIAN_DATA_FILE_NAME "fast-data_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.deb")
        set(CPACK_DEBIAN_DATA_PACKAGE_NAME "fast-data")
    else()
        set(CPACK_COMPONENTS_ALL fast)
    endif()

    set(CPACK_DEBIAN_FAST_FILE_NAME "fast_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.deb")
    set(CPACK_DEBIAN_FAST_PACKAGE_NAME "fast")

endif()
include(CPack)