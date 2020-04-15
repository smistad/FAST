# Download and build Qt5

include(cmake/Externals.cmake)

# List of modules can be found in git repo here: github.com/qt/qt5
set(MODULES_TO_EXCLUDE
        -skip qt3d
        #-skip qtactiveqt
        -skip qtandroidextras
        -skip qtcanvas3d
        -skip qtcharts
        -skip qtconnectivity
        -skip qtdatavis3d
        -skip qtdeclarative
        -skip qtdoc
        -skip qtdocgallery
        -skip qtfeedback
        -skip qtgamepad
        -skip qtgraphicaleffects
        -skip qtimageformats
        -skip qtlocation
        -skip qtlottie
        -skip qtmacextras
        -skip qtnetworkauth
        -skip qtpim
        -skip qtpurchasing
        -skip qtqa
        -skip qtquick3d
        -skip qtquickcontrols
        -skip qtquickcontrols2
        -skip qtquicktimeline
        -skip qtremoteobjects
        -skip qtrepotools
        -skip qtscript
        -skip qtscxml
        -skip qtsensors
        -skip qtserialbus
        #-skip qtserialport
        -skip qtspeech
        #-skip qtsvg
        -skip qtsystems
        -skip qttools
        -skip qttranslations
        -skip qtvirtualkeyboard
        -skip qtwayland
        -skip qtwebchannel
        -skip qtwebengine
        -skip qtwebglplugin
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
	set(URL "http://download.qt.io/archive/qt/5.14/5.14.0/single/qt-everywhere-src-5.14.0.zip")
	set(URL_HASH SHA256=0e4a3f096a3f331393240570fb4271f3f1f5a3b9c041252f6245e8dd789c96df)
	set(OPTIONS
            -opensource;
            -confirm-license;
            -release;
            -no-compile-examples;
            -no-openssl;
            -no-libproxy;
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
	set(URL "http://download.qt.io/archive/qt/5.14/5.14.0/single/qt-everywhere-src-5.14.0.tar.xz")
	set(URL_HASH SHA256=be9a77cd4e1f9d70b58621d0753be19ea498e6b0da0398753e5038426f76a8ba)
    if(APPLE)
        set(OPTIONS
            -opensource;
            -confirm-license;
            -release;
            -no-compile-examples;
            -no-openssl;
            -no-libproxy;
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
            -nomake tools;
            -nomake tests;
            -opengl desktop;
            -qt-zlib;
            -qt-libpng;
            -qt-libjpeg;
            -qt-freetype;
            -qt-harfbuzz;
            -qt-pcre;
            -qt-xcb;
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
    set(Qt5SerialPort_LIBRARY Qt5SerialPort.lib)
    set(Qt5PrintSupport_LIBRARY Qt5PrintSupport.lib)
else()
    set(Qt5Gui_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Gui${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Core_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Core${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Widgets_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Widgets${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5OpenGL_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5OpenGL${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5Multimedia_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Multimedia${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5MultimediaWidgets_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5MultimediaWidgets${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5SerialPort_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5SerialPort${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(Qt5PrintSupport_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}Qt5PrintSupport${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
set(Qt5Core_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtCore)
set(Qt5Gui_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtGui)
set(Qt5Widgets_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtWidgets)
set(Qt5OpenGL_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtOpenGL)
set(Qt5Multimedia_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtMultimedia)
set(Qt5MultimediaWidgets_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtMultimediaWidgets)
set(Qt5PrintSupport_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtPrintSupport)
list(APPEND FAST_EXTERNAL_DEPENDENCIES qt5)
