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

# Speed up compression:
set(CPACK_ARCHIVE_THREADS 0)
set(CPACK_THREADS 0)

if(WIN32 AND NOT UNIX)
    ## Windows
    set(CPACK_GENERATOR "NSIS" "ZIP")
    set(CPACK_MONOLITHIC_INSTALL ON)
    set(CPACK_PACKAGE_FILE_NAME "fast_windows_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "FAST")
    set(CPACK_NSIS_MUI_ICON ${PROJECT_SOURCE_DIR}/doc/images/fast_icon.ico)
    set(CPACK_NSIS_MUI_UNICON ${PROJECT_SOURCE_DIR}/doc/images/fast_icon.ico)
    set(CPACK_NSIS_MODIFY_PATH ON)
    # Start menu items
    set(CPACK_NSIS_MENU_LINKS
        "fast\\\\bin\\\\systemCheck"
        "System Check"
        "fast\\\\bin\\\\UFFviewer"
        "Ultrasound File Format Viewer"
        "fast\\\\bin\\\\runPipeline"
        "Run pipelines"
        "fast\\\\bin\\\\OpenIGTLinkServer"
        "OpenIGTLink Server"
        "fast\\\\bin\\\\downloadTestData"
        "Download test data (~2GB)"
        "http://fast.eriksmistad.no"
        "FAST Webpage"
        "Uninstall.exe"
        "Uninstall"
    )
    # Some more shortcuts
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        CreateShortCut \\\"$SMPROGRAMS\\\\FAST\\\\Examples.lnk\\\" \\\"$INSTDIR\\\\fast\\\\bin\\\"
        CreateShortCut \\\"$SMPROGRAMS\\\\FAST\\\\Data Folder.lnk\\\" \\\"C:\\\\ProgramData\\\\FAST\\\"
    ")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA "
        Delete \\\"$SMPROGRAMS\\\\FAST\\\\Examples.lnk\\\"
        Delete \\\"$SMPROGRAMS\\\\FAST\\\\Data Folder.lnk\\\"
    ")
elseif(APPLE)
    set(CPACK_GENERATOR "TXZ")
    set(CPACK_PACKAGE_FILE_NAME "fast_macos${CMAKE_OSX_DEPLOYMENT_TARGET}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}_${CMAKE_OSX_ARCHITECTURES}")
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

    set(CPACK_GENERATOR "DEB" "TXZ")
    # Select components to avoid some cmake leftovers from built dependencies
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE https://eriksmistad.no/fast/)
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_SOURCE_DIR}/cmake/PackageConfig.txt)

    # Add debian package dependencies
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libopenslide0,libusb-1.0-0")

    set(CPACK_COMPONENTS_ALL fast)

    set(CPACK_PACKAGE_FILE_NAME "fast_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    set(CPACK_DEBIAN_FAST_FILE_NAME "fast_${DISTRO_NAME}${DISTRO_VERSION}_${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.deb")
    set(CPACK_DEBIAN_FAST_PACKAGE_NAME "fast")
endif()
include(CPack) # Must be before cpack_ macros
