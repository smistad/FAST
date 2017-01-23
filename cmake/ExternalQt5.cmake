# Download and build Qt5

include(cmake/Externals.cmake)

if(WIN32)
	#set(BUILD_COMMAND set CL=/MP; nmake)
	set(BUILD_COMMAND nmake)
	set(URL "http://download.qt.io/official_releases/qt/5.7/5.7.1/submodules/qtbase-opensource-src-5.7.1.zip")
	set(URL_HASH SHA256=3570b8a107903615fef0faddcdbe80b35d788a80979a9f6d0de420c3339f45bf)
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
	set(URL "http://download.qt.io/official_releases/qt/5.7/5.7.1/submodules/qtbase-opensource-src-5.7.1.tar.gz")
	set(URL_HASH SHA256=95f83e532d23b3ddbde7973f380ecae1bac13230340557276f75f2e37984e410)
    if(APPLE)
        set(OPTIONS
            -opensource;
            -confirm-license;
            -release;
            -no-compile-examples;
            -no-openssl;
            -no-libproxy;
            -no-qml-debug;
            -no-pulseaudio;
            -no-alsa;
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
            -no-pulseaudio;
            -no-alsa;
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
            ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/configure
            -prefix ${FAST_EXTERNAL_INSTALL_DIR};
            ${OPTIONS}
        BUILD_COMMAND
            ${BUILD_COMMAND}
        INSTALL_COMMAND
            ${BUILD_COMMAND} install

)

if(WIN32)
    set(Qt5Gui_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/Qt5Gui.lib)
    set(Qt5Core_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/Qt5Core.lib)
    set(Qt5Widgets_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/Qt5Widgets.lib)
    set(Qt5OpenGL_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/Qt5OpenGL.lib)
else()
    set(Qt5Gui_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Gui${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Core_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Core${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Widgets_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Widgets${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5OpenGL_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5OpenGL${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND LIBRARIES ${Qt5Core_LIBRARY})
list(APPEND LIBRARIES ${Qt5Gui_LIBRARY})
list(APPEND LIBRARIES ${Qt5Widgets_LIBRARY})
list(APPEND LIBRARIES ${Qt5OpenGL_LIBRARY})
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtWidgets)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtOpenGL)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtCore)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtGui)
list(APPEND FAST_EXTERNAL_DEPENDENCIES qt5)
