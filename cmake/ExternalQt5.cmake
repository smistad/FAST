# Download and build Qt5

include(cmake/Externals.cmake)

if(WIN32)
	#set(BUILD_COMMAND set CL=/MP; nmake)
	set(BUILD_COMMAND nmake)
	set(URL "http://download.qt.io/official_releases/qt/5.7/5.7.1/single/qt-everywhere-opensource-src-5.7.1.zip")
	set(URL_HASH SHA256=4e50c645ff614d831712f5ef19a4087b4c00824920c79e96fee17d9373b42cf3)
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
            -qt-freetype;
	)
else()
	set(BUILD_COMMAND make -j4)
	set(URL "http://download.qt.io/official_releases/qt/5.7/5.7.1/single/qt-everywhere-opensource-src-5.7.1.tar.gz")
	set(URL_HASH SHA256=c86684203be61ae7b33a6cf33c23ec377f246d697bd9fb737d16f0ad798f89b7)
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
	)
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
#list(APPEND LIBRARIES Qt5::Widgets)
#list(APPEND LIBRARIES Qt5::OpenGL)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtWidgets)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtOpenGL)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtCore)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtGui)
list(APPEND FAST_EXTERNAL_DEPENDENCIES qt5)
