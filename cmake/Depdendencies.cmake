# Setup all dependencies for FAST, both internal (have to be installed on the system)
# and external (downloaded and built automatically)

## OpenCL
find_package(OpenCL REQUIRED)
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
#list(APPEND FAST_INCLUDE_DIRS "${OpenCL_INCLUDE_DIRS}")
#message("-- OpenCL include dir: ${OpenCL_INCLUDE_DIRS}")
message("-- OpenCL library: ${OpenCL_LIBRARIES}")

## OpenGL
find_package(OpenGL REQUIRED)
list(APPEND LIBRARIES ${OPENGL_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
# If OS is Linux, also need X
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_package(X11 REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${X11_INCLUDE_DIR})
    list(APPEND LIBRARIES ${X11_LIBRARIES})
endif()

## Qt
if(${FAST_MODULE_Visualization})
    include(cmake/ExternalQt5.cmake)
    # TODO issue: These Config files are not available until after build of Qt5
    # One possibility is to copy cmake files to our cmake folder, but some of them seem to be generated
    # If not possible, need to trigger build of qt5 in the configure step of FAST
    # Main issue is that MOC doesn't work without either CMAKE_AUTOMOC or the qt5_wrap_cpp macro
        # ExtraIncludeSources.cmake has absolute paths, but its optional?
        set(Qt5Core_DIR ${PROJECT_SOURCE_DIR}/cmake/Qt5Core)
    set(Qt5Gui_DIR ${PROJECT_SOURCE_DIR}/cmake/Qt5Gui)
    set(Qt5Widgets_DIR ${PROJECT_SOURCE_DIR}/cmake/Qt5Widgets)
    set(Qt5OpenGL_DIR ${PROJECT_SOURCE_DIR}/cmake/Qt5OpenGL)
    find_package(Qt5Widgets REQUIRED PATHS ${PROJECT_SOURCE_DIR}/cmake/)
    find_package(Qt5OpenGL REQUIRED PATHS ${PROJECT_SOURCE_DIR}/cmake/)
    #set(CMAKE_AUTOMOC ON)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        add_definitions("-fPIC") # Get rid of Qt error with position independent code
    endif()
    #include(cmake/Qt5CoreMacros.cmake) # Need this for the qt5_use_modules macro
    #include(${PROJECT_BINARY_DIR}/lib/cmake/Qt5Gui/Qt5GuiConfig.cmake)
    #include(${PROJECT_BINARY_DIR}/lib/cmake/Qt5Widgets/Qt5WidgetsConfig.cmake)
    qt5_wrap_cpp(HEADERS_MOC ${QT_HEADERS})
    #list(APPEND LIBRARIES ${QT_LIBRARIES})
    #list(APPEND FAST_INCLUDE_DIRS "${Qt5Widgets_INCLUDE_DIRS}")
    #list(APPEND FAST_INCLUDE_DIRS "${Qt5OpenGL_INCLUDE_DIRS}")
else()
    #add_definitions("-DFAST_NO_VISUALIZATION")
endif()

## External depedencies
include(cmake/ExternalEigen.cmake)
include(cmake/ExternalZlib.cmake)

# Optional modules
include(cmake/ModuleVTK.cmake)
include(cmake/ModuleITK.cmake)
include(cmake/ModuleOpenIGTLink.cmake)
include(cmake/ModuleNeuralNetwork.cmake)
include(cmake/ModuleKinect.cmake)

# Make sure FAST can find external includes and libaries
link_directories(${FAST_EXTERNAL_INSTALL_DIR}/lib/)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include)
