# Setup all dependencies for FAST, both internal (have to be installed on the system)
# and external (downloaded and built automatically)

## OpenCL
find_package(OpenCL REQUIRED)
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS "${OpenCL_INCLUDE_DIRS}")
message("-- OpenCL include dir: ${OpenCL_INCLUDE_DIRS}")
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
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
qt5_wrap_cpp(HEADERS_MOC ${QT_HEADERS})
list(APPEND LIBRARIES ${QT_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS "${Qt5Widgets_INCLUDE_DIRS}")
list(APPEND FAST_INCLUDE_DIRS "${Qt5OpenGL_INCLUDE_DIRS}")

## External depedencies
include(cmake/ExternalEigen.cmake)
include(cmake/ExternalZlib.cmake)
include(cmake/ExternalGLEW.cmake)

# Optional modules
include(cmake/ModuleVTK.cmake)
include(cmake/ModuleITK.cmake)
include(cmake/ModuleOpenIGTLink.cmake)
include(cmake/ModuleNeuralNetwork.cmake)

# Make sure FAST can find external includes and libaries
link_directories(${FAST_EXTERNAL_INSTALL_DIR}/lib/)
list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include)
