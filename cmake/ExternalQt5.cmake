# Download and build Qt5

include(cmake/Externals.cmake)

ExternalProject_Add(qt5
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/qt5
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/qt5
        URL "http://download.qt.io/official_releases/qt/5.7/5.7.1/single/qt-everywhere-opensource-src-5.7.1.tar.gz"
        CONFIGURE_COMMAND
            ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/configure
            -prefix ${FAST_EXTERNAL_INSTALL_DIR};
            -opensource;
            -confirm-license;
            -release;
            -optimized-qmake;
            -no-compile-examples;
            -no-openssl;
            -no-libproxy;
            -no-qml-debug;
            -no-pulseaudio;
            -no-alsa;
            -nomake tools;
            -nomake tests;
            -skip qt3d;
            -skip qtactiveqt;
            -skip qtandroidextras;
            -skip qtcanvas3d;
            -skip qtcharts;
            -skip qtconnectivity;
            -skip qtdatavis3d;
            -skip qtdeclarative;
            -skip qtdoc;
            -skip qtgamepad;
            -skip qtgraphicaleffects;
            -skip qtlocation;
            -skip qtmacextras;
            -skip qtmultimedia;
            -skip qtpurchasing;
            -skip qtquickcontrols;
            -skip qtquickcontrols2;
            -skip qtscript;
            -skip qtscxml;
            -skip qtsensors;
            -skip qtserialbus;
            -skip qtserialport;
            -skip qtsvg;
            -skip qttools;
            -skip qttranslations;
            -skip qtvirtualkeyboard;
            -skip qtwayland;
            -skip qtwebchannel;
            -skip webengine;
            -skip qtwebsockets;
            -skip qtwebview;
            -skip qtwinextras;
            -skip qtx11extras;
            -skip qtxmlpatterns;
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
        BUILD_COMMAND
            make -j4
        INSTALL_COMMAND
            make -j4 install
)

set(Qt5Widgets_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Widgets${CMAKE_SHARED_LIBRARY_SUFFIX})
set(Qt5OpenGL_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5OpenGL${CMAKE_SHARED_LIBRARY_SUFFIX})
#set(Qt5Gui_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Gui${CMAKE_SHARED_LIBRARY_SUFFIX})
#set(Qt5Core_LIBRARY ${FAST_EXTERNAL_INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}Qt5Core${CMAKE_SHARED_LIBRARY_SUFFIX})
list(APPEND LIBRARIES ${Qt5Widgets_LIBRARY})
list(APPEND LIBRARIES ${Qt5OpenGL_LIBRARY})
#list(APPEND LIBRARIES ${Qt5Core_LIBRARY})
#list(APPEND LIBRARIES ${Qt5Gui_LIBRARY})
#list(APPEND LIBRARIES Qt5::Widgets)
#list(APPEND LIBRARIES Qt5::OpenGL)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtWidgets)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtOpenGL)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtCore)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtGui)
list(APPEND FAST_EXTERNAL_DEPENDENCIES qt5)
