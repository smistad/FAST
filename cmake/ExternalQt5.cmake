# Download and build Qt5

include(cmake/Externals.cmake)

# List of modules can be found in git repo here: github.com/qt/qt5
set(MODULES_TO_EXCLUDE
        -skip qt3d
        -skip qtactiveqt
        -skip qtandroidextras
        -skip qtcanvas3d
        -skip qtcharts
        -skip qtconnectivity
        -skip qtdatavis3d
        -skip qtdeclarative
        -skip qtdoc
        -skip qtdocgallery
        -skip qtenginio
        -skip qtfeedback
        -skip qtgamepad
        -skip qtgraphicaleffects
        -skip qtimageformats
        -skip qtlocation
        -skip qtmacextras
        -skip qtnetworkauth
        -skip qtpim
        -skip qtpurchasing
        -skip qtqa
        -skip qtquick1
        -skip qtquickcontrols
        -skip qtquickcontrols2
	-skip qtremoteobjects
        -skip qtrepotools
        -skip qtscript
        -skip qtscxml
        -skip qtsensors
        -skip qtserialbus
        -skip qtserialport
        -skip qtspeech
        -skip qtsvg
        -skip qtsystems
        -skip qttools
        -skip qttranslations
        -skip qtvirtualkeyboard
        -skip qtwayland
        -skip qtwebchannel
        -skip qtwebengine
        -skip qtwebsockets
        -skip qtwebview
        -skip qtwinextras
        -skip qtx11extras
        -skip qtxmlpatterns
        )

if(WIN32)
	#set(BUILD_COMMAND set CL=/MP; nmake)
	set(BUILD_COMMAND nmake)
	set(CONFIGURE_COMMAND ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/configure.bat)
	set(URL "http://download.qt.io/archive/qt/5.9/5.9.4/single/qt-everywhere-opensource-src-5.9.4.zip")
	set(URL_HASH SHA256=37c5347e3c98a2ac02c2368cd5d9af0bb2c8f2f4632306188238db4f8c644f08)
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
            ${MODULES_TO_EXCLUDE}
	)
else()
	set(BUILD_COMMAND make -j4)
	set(CONFIGURE_COMMAND ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/configure)
	set(URL "http://download.qt.io/archive/qt/5.9/5.9.4/single/qt-everywhere-opensource-src-5.9.4.tar.xz")
	set(URL_HASH SHA256=e3acd9cbeafba3aed9f14592f4d70bf0b255e0203943e8d2b4235002268274d5)
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
            ${MODULES_TO_EXCLUDE}
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
            -qt-freetype;
            -qt-xcb;
            -qt-xkbcommon;
            -no-directfb;
            -no-linuxfb;
            ${MODULES_TO_EXCLUDE}
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
    set(Qt5Multimedia_LIBRARY Qt5Multimedia.lib)
    set(Qt5MultimediaWidgets_LIBRARY Qt5MultimediaWidgets.lib)
else()
    set(Qt5Gui_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Gui${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Core_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Core${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Widgets_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Widgets${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5OpenGL_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5OpenGL${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Multimedia_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Multimedia${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5MultimediaWidgets_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5MultimediaWidgets${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
list(APPEND FAST_EXTERNAL_DEPENDENCIES qt5)
