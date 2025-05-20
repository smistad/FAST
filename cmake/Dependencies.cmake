# Setup all dependencies for FAST, both internal (have to be installed on the system)
# and external (downloaded and built automatically)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/licenses/)

## ============ OpenCL
if(WIN32)
    fast_download_dependency(opencl
            3.0.8
            0b2d3098ed29fac09bbd7f831647e08e23324eaccbdb29c603617989e48c50b1
            OpenCL.lib
    )
elseif(APPLE)
	find_package(OpenCL REQUIRED) # Need to link with Apple OpenCL binary and not khronos..?
	list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
	list(APPEND FAST_INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS})
if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        fast_download_dependency(opencl
            3.0.8
            a97ebc4eb918ba0bd7ae4b1a01afdbd1097fa300854da433f6687eaa891f75fa
        )
else()
        fast_download_dependency(opencl
            3.0.8
            63d72f3af7a3362e48ea45b55c772f2717de55a27de5aaf38e55e0364a4ee45b
        )
endif()
else()
    fast_download_dependency(opencl
            3.0.8
            a9fc571dc6cb034145e1a9dcae63649762a8a4616eca617ffabc7c77a7fb1894
            libOpenCL.so
    )
endif()

## ============== OpenGL
find_package(OpenGL REQUIRED)
list(APPEND FAST_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
list(APPEND FAST_SYSTEM_LIBRARIES OpenGL::OpenGL)
# If OS is Linux, also need X, pthread and GLX
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${X11_INCLUDE_DIR})
    list(APPEND FAST_SYSTEM_LIBRARIES X11::X11)
    list(APPEND FAST_SYSTEM_LIBRARIES pthread)
    list(APPEND FAST_SYSTEM_LIBRARIES OpenGL::GLX)
endif()

## ============== Qt
if(FAST_MODULE_Visualization)
    if(FAST_BUILD_QT5)
        # Use FAST build of Qt
        if(WIN32)
            fast_download_dependency(qt5
                    5.15.2
		    3d025e412d4af7d4fe669677ac71614909eb44aac8eca6b8fb29459abf621dff
                    Qt5Core.lib Qt5Gui.lib Qt5Widgets.lib Qt5OpenGL.lib Qt5Multimedia.lib Qt5MultimediaWidgets.lib Qt5Network.lib Qt5PrintSupport.lib Qt5SerialPort.lib
            )
            fast_download_dependency(openssl
                    1.1.1
                    e0f9b5d26627c70abbfa9e3b0d731a81995bdd2af2177eebf1c4b32691643c9e
            )
      	elseif(APPLE)
		if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
			fast_download_dependency(qt5
			    5.15.8
			    2c5e30b1ae862e9d47f3e59ae2346eea0cb24c2208307bbade6ac2ec8bca3463
			    libQt5Core.dylib libQt5Gui.dylib libQt5Widgets.dylib libQt5OpenGL.dylib libQt5Multimedia.dylib libQt5MultimediaWidgets.dylib libQt5Network.dylib libQt5PrintSupport.dylib libQt5SerialPort.dylib
			    )
		else()
            fast_download_dependency(qt5
                    5.15.2
		    0f5ff6ea8a9299cda1f41922f310a1f8f5bb8bf8829c46e85eb68e2638e2a840
                    libQt5Core.dylib libQt5Gui.dylib libQt5Widgets.dylib libQt5OpenGL.dylib libQt5Multimedia.dylib libQt5MultimediaWidgets.dylib libQt5Network.dylib libQt5PrintSupport.dylib libQt5SerialPort.dylib
            )
		endif()
        else()
            fast_download_dependency(qt6
                    6.9.0
                    12c714bba41779a792c09a7e8611c23507ae140870f54c14ebc381e0fe39314d
                    libQt6Core.so libQt6Gui.so libQt6Widgets.so libQt6OpenGL.so libQt6OpenGLWidgets.so libQt6Multimedia.so libQt6MultimediaWidgets.so libQt6Network.so
            )
            fast_download_dependency(openssl
                    1.1.1
                    813d09d0e4fb8c03b4470692659d8600e5d56c77708aa27c0290e9be03cc7352
            )
        endif()
        # Need to set version manually to suppress warnings
        set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                PROPERTY Qt6Core_VERSION_MAJOR "6")
        set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                PROPERTY Qt6Core_VERSION_MINOR "9")
        set(Qt6Core_VERSION_MAJOR "6")
        set(Qt6Core_VERSION_MINOR "9")
        # MOC/AUTOGEN setup
        add_executable(Qt6::moc IMPORTED)
        add_executable(Qt6::rcc IMPORTED)
        add_executable(Qt6::uic IMPORTED)
        add_dependencies(Qt6::moc qt6)
        set(QT_FORCE_MIN_CMAKE_VERSION_FOR_USING_QT 3.20)

        #set(Qt6_DIR ${PROJECT_SOURCE_DIR}/cmake/Qt/Qt6)
        #find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Multimedia MultimediaWidgets Network PATHS ${PROJECT_SOURCE_DIR}/cmake/)

        set(MOC_FILENAME "${PROJECT_BINARY_DIR}/bin/moc${CMAKE_EXECUTABLE_SUFFIX}" )
        set_target_properties(Qt6::moc PROPERTIES IMPORTED_LOCATION ${MOC_FILENAME})
        # HACK: Create fake moc executable to avoid errors on newer cmake
        if(NOT EXISTS ${MOC_FILENAME})
            # Copy an executable we know exist: cmake(.exe)
            file(COPY ${CMAKE_COMMAND}
                    DESTINATION "${PROJECT_BINARY_DIR}/bin/"
                    FOLLOW_SYMLINK_CHAIN
                    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE)
            # Rename it to moc(.exe)
            file(RENAME ${PROJECT_BINARY_DIR}/bin/cmake${CMAKE_EXECUTABLE_SUFFIX} ${PROJECT_BINARY_DIR}/bin/moc${CMAKE_EXECUTABLE_SUFFIX})
        endif()

        set(QT_MODULES QtCore QtGui QtWidgets QtOpenGL QtOpenGLWidgets QtMultimedia QtMultimediaWidgets QtPrintSupport QtNetwork)
        foreach(ITEM ${QT_MODULES})
            list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/${ITEM})
        endforeach()
    else(FAST_BUILD_QT5)
        # Use system Qt
        find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL Multimedia MultimediaWidgets PrintSupport Network)
        list(APPEND LIBRARIES Qt6::Core)
        list(APPEND LIBRARIES Qt6::Gui)
        list(APPEND LIBRARIES Qt6::Widgets)
        list(APPEND LIBRARIES Qt6::OpenGL)
        list(APPEND LIBRARIES Qt6::Multimedia)
        list(APPEND LIBRARIES Qt6::MultimediaWidgets)
        list(APPEND LIBRARIES Qt6::PrintSupport)
        list(APPEND LIBRARIES Qt6::Network)

        list(APPEND FAST_INCLUDE_DIRS
                ${Qt6Widgets_INCLUDE_DIRS}
                ${Qt6Core_INCLUDE_DIRS}
                ${Qt6Gui_INCLUDE_DIRS}
                ${Qt6OpenGL_INCLUDE_DIRS}
                ${Qt6OpenGLWidgets_INCLUDE_DIRS}
                ${Qt6Multimedia_INCLUDE_DIRS}
                ${Qt6MultimediaWidgets_INCLUDE_DIRS}
                ${Qt6PrintSupport_INCLUDE_DIRS}
                ${Qt6Network_INCLUDE_DIRS}
        )
    endif(FAST_BUILD_QT5)

    set(CMAKE_AUTOMOC ON)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        add_definitions("-fPIC") # Get rid of Qt error with position independent code
    endif()
    add_definitions("-DFAST_MODULE_VISUALIZATION")
endif()

## ============= External dependencies
include(cmake/ExternalEigen.cmake)
add_definitions("-DEIGEN_MPL2_ONLY") # Avoid using LGPL code in eigen http://eigen.tuxfamily.org/index.php?title=Main_Page#License
include(cmake/ExternalZlib.cmake)
include(cmake/ExternalZip.cmake)
include(cmake/ImageCompressionDependencies.cmake)

## ============= Optional modules + dependencies
include(cmake/ModuleVTK.cmake)
include(cmake/ModuleITK.cmake)
include(cmake/ModuleOpenIGTLink.cmake)
include(cmake/ModuleKinect.cmake)
include(cmake/ModuleRealSense.cmake)
include(cmake/ModuleWholeSlideImaging.cmake)
include(cmake/ModuleClarius.cmake)
include(cmake/ModuleDicom.cmake)
include(cmake/ModuleHDF5.cmake)
include(cmake/ModulePlotting.cmake)

# Make sure FAST can find external includes and libaries
link_directories(${FAST_EXTERNAL_INSTALL_DIR}/lib/)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include)
