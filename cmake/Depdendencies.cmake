# Setup all dependencies for FAST, both internal (have to be installed on the system)
# and external (downloaded and built automatically)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/lib/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/bin/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/include/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/licenses/)

## OpenCL
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
        fast_download_dependency(opencl
            3.0.8
            63d72f3af7a3362e48ea45b55c772f2717de55a27de5aaf38e55e0364a4ee45b
        )
else()
    fast_download_dependency(opencl
            3.0.8
            a9fc571dc6cb034145e1a9dcae63649762a8a4616eca617ffabc7c77a7fb1894
            libOpenCL.so
    )
endif()

## OpenGL
find_package(OpenGL REQUIRED)
list(APPEND FAST_SYSTEM_LIBRARIES ${OPENGL_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
# If OS is Linux, also need X
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${X11_INCLUDE_DIR})
    list(APPEND FAST_SYSTEM_LIBRARIES ${X11_LIBRARIES})
    list(APPEND FAST_SYSTEM_LIBRARIES pthread)
endif()

## Qt
if(FAST_MODULE_Visualization)
    if(FAST_BUILD_QT5)
        # Let FAST build Qt 5
        if(WIN32)
            fast_download_dependency(qt5
                    5.15.5
                    0e253ac91463f3d722f100402c24b8ec2363bb28954636068d0b5bd30e0476a0
                    Qt5Core.lib Qt5Gui.lib Qt5Widgets.lib Qt5OpenGL.lib Qt5Multimedia.lib Qt5MultimediaWidgets.lib Qt5Network.lib Qt5PrintSupport.lib Qt5SerialPort.lib
            )
            fast_download_dependency(openssl
                    1.1.1
                    e0f9b5d26627c70abbfa9e3b0d731a81995bdd2af2177eebf1c4b32691643c9e
            )
      	elseif(APPLE)
            fast_download_dependency(qt5
                    5.14.0
                    9751aa43880c0193c59836410cbb6b343796b1b4cd5f86861300370014eac162
                    libQt5Core.dylib libQt5Gui.dylib libQt5Widgets.dylib libQt5OpenGL.dylib libQt5Multimedia.dylib libQt5MultimediaWidgets.dylib libQt5Network.dylib libQt5PrintSupport.dylib libQt5SerialPort.dylib
            )
        else()
            fast_download_dependency(qt5
                    5.14.0
                    3281e50a545c1a7275cf7626b348468049ac41c05944bcc9fd08708831c2d28f
                    libQt5Core.so libQt5Gui.so libQt5Widgets.so libQt5OpenGL.so libQt5Multimedia.so libQt5MultimediaWidgets.so libQt5Network.so libQt5PrintSupport.so libQt5SerialPort.so
            )
        endif()
        # MOC setup
        set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                PROPERTY Qt5Core_VERSION_MAJOR "5")
        set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                PROPERTY Qt5Core_VERSION_MINOR "14")
        add_executable(Qt5::moc IMPORTED)
        add_dependencies(Qt5::moc qt5)
        set(MOC_FILENAME "${PROJECT_BINARY_DIR}/bin/moc${CMAKE_EXECUTABLE_SUFFIX}" )
        set_target_properties(Qt5::moc PROPERTIES IMPORTED_LOCATION ${MOC_FILENAME})
        # HACK: Create fake moc executable to avoid errors on newer cmake
        if(NOT EXISTS ${MOC_FILENAME})
            # Copy an executable we know exist: cmake(.exe)
            file(COPY ${CMAKE_COMMAND}
                DESTINATION "${PROJECT_BINARY_DIR}/bin/"
                FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE)
            # Rename it to moc(.exe)
            file(RENAME ${PROJECT_BINARY_DIR}/bin/cmake${CMAKE_EXECUTABLE_SUFFIX} ${PROJECT_BINARY_DIR}/bin/moc${CMAKE_EXECUTABLE_SUFFIX})
        endif()
        set(Qt5_DIR ${PROJECT_SOURCE_DIR}/cmake/Qt5/)
        find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL Multimedia MultimediaWidgets PrintSupport Network PATHS ${PROJECT_SOURCE_DIR}/cmake/)
        set(Qt5Core_VERSION "5.14.0")
        set(Qt5Core_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtCore)
        set(Qt5Gui_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtGui)
        set(Qt5Widgets_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtWidgets)
        set(Qt5OpenGL_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtOpenGL)
        set(Qt5Multimedia_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtMultimedia)
        set(Qt5MultimediaWidgets_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtMultimediaWidgets)
        set(Qt5PrintSupport_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtPrintSupport)
        set(Qt5Network_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/QtNetwork)
    else(FAST_BUILD_QT5)
        # Use system Qt
        find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL Multimedia MultimediaWidgets PrintSupport Network)
        list(APPEND LIBRARIES Qt5::Core)
        list(APPEND LIBRARIES Qt5::Gui)
        list(APPEND LIBRARIES Qt5::Widgets)
        list(APPEND LIBRARIES Qt5::OpenGL)
        list(APPEND LIBRARIES Qt5::Multimedia)
        list(APPEND LIBRARIES Qt5::MultimediaWidgets)
        list(APPEND LIBRARIES Qt5::PrintSupport)
        list(APPEND LIBRARIES Qt5::Network)
    endif(FAST_BUILD_QT5)

    list(APPEND FAST_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Gui_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5OpenGL_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Multimedia_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5MultimediaWidgets_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5PrintSupport_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Network_INCLUDE_DIRS})
    set(CMAKE_AUTOMOC ON)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        add_definitions("-fPIC") # Get rid of Qt error with position independent code
    endif()
    add_definitions("-DFAST_MODULE_VISUALIZATION")
endif()

## External depedencies
include(cmake/ExternalEigen.cmake)
add_definitions("-DEIGEN_MPL2_ONLY") # Avoid using LGPL code in eigen http://eigen.tuxfamily.org/index.php?title=Main_Page#License
include(cmake/ExternalZlib.cmake)
include(cmake/ExternalZip.cmake)

# Optional modules
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
