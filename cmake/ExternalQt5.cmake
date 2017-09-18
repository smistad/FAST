# Download and build Qt5

include(cmake/Externals.cmake)

if(WIN32)
	#set(BUILD_COMMAND set CL=/MP; nmake)
	set(BUILD_COMMAND nmake)
	set(CONFIGURE_COMMAND ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/configure.bat)
	set(URL "http://download.qt.io/official_releases/qt/5.8/5.8.0/submodules/qtbase-opensource-src-5.8.0.zip")
	set(URL_HASH SHA256=4899c64c27249690a0a6b46208408d9dcc7043f42e175417e2f7634710f8f3fa)
	set(OPTIONS
            -opensource;
            -confirm-license;
            -release;
            -no-compile-examples;           
            -no-openssl;
            -no-libproxy;
            -no-qml-debug;
            -nomake tools;
            -nomake tests;
              -opengl desktop;
            -qt-zlib;
            -qt-libpng;
            -qt-libjpeg;
            -qt-freetype;
	)
else()
	set(BUILD_COMMAND make -j4)
	set(CONFIGURE_COMMAND ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/configure)
	set(URL "http://download.qt.io/official_releases/qt/5.8/5.8.0/submodules/qtbase-opensource-src-5.8.0.tar.gz")
	set(URL_HASH SHA256=0f6ecd94abd148f1ea4ad08905308af973c6fad9e8fca7491d68dbc8fbd88872)
    if(APPLE)
        set(OPTIONS
            -opensource;
            -confirm-license;
            -release;
            -no-compile-examples;
            -no-openssl;
            -no-libproxy;
            -no-qml-debug;
            -nomake tools;
            -nomake tests;
            -opengl desktop;
            -qt-zlib;
            -qt-libpng;
            -qt-libjpeg;
            -no-directfb;
            -no-framework;
        )
    else()
        set(OPTIONS
            -opensource;
            -confirm-license;
            -release;
            -no-compile-examples;         
            -no-openssl;
            -no-libproxy;
            -no-qml-debug;
            -nomake tools;
            -nomake tests;
            -opengl desktop;
            -qt-zlib;
            -qt-libpng;
            -qt-libjpeg;
            -system-freetype;
            -qt-xcb;
            -qt-xkbcommon;
            -no-directfb;
            -no-linuxfb;
            -fontconfig;
        )
    endif()
endif()

ExternalProject_Add(qt5
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/qt5
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/qt5
        URL ${URL}
        URL_HASH ${URL_HASH}
        CONFIGURE_COMMAND
            ${CONFIGURE_COMMAND}
            -prefix ${FAST_EXTERNAL_INSTALL_DIR};
            ${OPTIONS}
        BUILD_COMMAND
            ${BUILD_COMMAND}
        INSTALL_COMMAND
            ${BUILD_COMMAND} install

)
if(WIN32)
    set(Qt5Gui_LIBRARY Qt5Gui.lib)
    set(Qt5Core_LIBRARY Qt5Core.lib)
    set(Qt5Widgets_LIBRARY Qt5Widgets.lib)
    set(Qt5OpenGL_LIBRARY Qt5OpenGL.lib)
else()
    set(Qt5Gui_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Gui${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Core_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Core${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Widgets_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Widgets${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5OpenGL_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5OpenGL${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES qt5)
