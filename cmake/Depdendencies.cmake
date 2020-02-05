# Setup all dependencies for FAST, both internal (have to be installed on the system)
# and external (downloaded and built automatically)

## OpenCL
find_package(OpenCL REQUIRED)
list(APPEND FAST_SYSTEM_LIBRARIES ${OpenCL_LIBRARIES})
#list(APPEND FAST_INCLUDE_DIRS "${OpenCL_INCLUDE_DIRS}")
#message("-- OpenCL include dir: ${OpenCL_INCLUDE_DIRS}")
message("-- OpenCL library: ${OpenCL_LIBRARIES}")

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
        include(cmake/ExternalQt5.cmake)
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
        find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL Multimedia MultimediaWidgets PATHS ${PROJECT_SOURCE_DIR}/cmake/)
        list(APPEND LIBRARIES ${Qt5Core_LIBRARY})
        list(APPEND LIBRARIES ${Qt5Gui_LIBRARY})
        list(APPEND LIBRARIES ${Qt5Widgets_LIBRARY})
        list(APPEND LIBRARIES ${Qt5OpenGL_LIBRARY})
        list(APPEND LIBRARIES ${Qt5Multimedia_LIBRARY})
        list(APPEND LIBRARIES ${Qt5MultimediaWidgets_LIBRARY})
    else(FAST_BUILD_QT5)
        # Use system Qt
        find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL Multimedia MultimediaWidgets )
        list(APPEND LIBRARIES Qt5::Core)
        list(APPEND LIBRARIES Qt5::Gui)
        list(APPEND LIBRARIES Qt5::Widgets)
        list(APPEND LIBRARIES Qt5::OpenGL)
        list(APPEND LIBRARIES Qt5::Multimedia)
        list(APPEND LIBRARIES Qt5::MultimediaWidgets)
    endif(FAST_BUILD_QT5)

    list(APPEND FAST_INCLUDE_DIRS ${Qt5Widgets_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Gui_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5OpenGL_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5Multimedia_INCLUDE_DIRS})
    list(APPEND FAST_INCLUDE_DIRS ${Qt5MultimediaWidgets_INCLUDE_DIRS})
    set(CMAKE_AUTOMOC ON)

    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        add_definitions("-fPIC") # Get rid of Qt error with position independent code
    endif()
    add_definitions("-DFAST_MODULE_VISUALIZATION")
endif()

## External depedencies
include(cmake/ExternalEigen.cmake)
include(cmake/ExternalZlib.cmake)

# Optional modules
include(cmake/ModuleVTK.cmake)
include(cmake/ModuleITK.cmake)
include(cmake/ModuleOpenIGTLink.cmake)
include(cmake/ModuleKinect.cmake)
include(cmake/ModuleRealSense.cmake)
include(cmake/ModuleWholeSlideImaging.cmake)
include(cmake/ModuleClarius.cmake)
include(cmake/ModuleDicom.cmake)

# Make sure FAST can find external includes and libaries
link_directories(${FAST_EXTERNAL_INSTALL_DIR}/lib/)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include)
