cmake_minimum_required(VERSION 2.8.9)
project(FAST)
include(cmake/Macros.cmake)

#### Options
option(FAST_BUILD_TESTS "Build FAST tests." ON)
option(BUILD_IGTLINK_TESTS "Build OpenIGTLink tests." OFF)
option(BUILD_EXAMPLES "Build OpenIGTLink examples." OFF)
option(BUILD_SHARED_LIBS "Build shared libs" ON)
option(FAST_BUILD_EXAMPLES "Build examples." OFF)
option(FAST_MODULE_VTK "Enable interoperability with VTK" OFF)
option(FAST_MODULE_ITK "Enable interoperability with ITK" OFF)
option(FAST_MODULE_OpenIGTLink "Build module OpenIGTLink" ON)
option(FAST_MODULE_NeuralNetwork "Build neural network module using tensorflow" OFF)
option(FAST_MODULE_Python "Build Python wrappers" ON)
option(FAST_CONTINUOUS_INTEGRATION "Used for continuous integration tests" OFF)

#### Set build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("-- Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build types for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

## Set continous integration define, used to enabled auto-close application on examples
if(FAST_CONTINUOUS_INTEGRATION)
    add_definitions("-DFAST_CONTINUOUS_INTEGRATION")
endif()

## Set build folders
# First for the generic no-config case (e.g. with mingw)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

## Turn off shared libs on windows
if(WIN32)
    set(BUILD_SHARED_LIBS OFF)
endif()

#### Paths
set(FAST_TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/TestData/" CACHE PATH "Directory of test data")
set(FAST_TEST_DATA_DIR "${FAST_TEST_DATA_DIR}/") # Add a slash to the back
set(FAST_OPENCL_KERNEL_BINARY_PATH "${PROJECT_BINARY_DIR}/" CACHE PATH "Directory to put kernel binaries in. Default is build dir.")
set(FAST_OPENCL_KERNEL_BINARY_PATH "${FAST_OPENCL_KERNEL_BINARY_PATH}/") # Add a slash to the back
set(FAST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source/FAST/")
# Remove any existing "
string(REPLACE "\"" "" FAST_OPENCL_KERNEL_BINARY_PATH ${FAST_OPENCL_KERNEL_BINARY_PATH})
string(REPLACE "\"" "" FAST_TEST_DATA_DIR ${FAST_TEST_DATA_DIR})
message("-- OpenCL Kernel binary path set to: ${FAST_OPENCL_KERNEL_BINARY_PATH}")
message("-- FAST test data directory set to: \"${FAST_TEST_DATA_DIR}\"")

### Create the header files Paths.hpp and TestDataPath.hpp which include all useful paths
# TODO don't use preprocessor defines for this, use constant string or static functions
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Paths.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/FAST/Paths.hpp
)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/source/TestDataPath.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/FAST/TestDataPath.hpp
)

#### Module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH}) # For finding the custom Find modules

# Headers for Qt objects
# TODO make a macro for adding these
set(QT_HEADERS
        source/FAST/Visualization/View.hpp
        source/FAST/Visualization/WindowWidget.hpp
        source/FAST/Visualization/Window.hpp
        source/FAST/Visualization/ComputationThread.hpp
        source/FAST/Examples/Interoperability/qtInteroperability.hpp
)

# Set debug define if debug mode is set
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message("-- FAST Debug mode set")
    add_definitions("-DFAST_DEBUG")
endif()

#### Set up library dependencies
set(FAST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/ ${CMAKE_CURRENT_BINARY_DIR})


## OpenCL
find_package(OpenCL REQUIRED)
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS "${OpenCL_INCLUDE_DIRS}")
message("-- OpenCL include dir: ${OpenCL_INCLUDE_DIRS}")

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

## GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_PATH})
list(APPEND LIBRARIES ${GLEW_LIBRARY})


## Neural network module
if(FAST_MODULE_NeuralNetwork)
    ## Tensorflow
    find_package(Tensorflow REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${Tensorflow_INCLUDE_DIRS})
    list(APPEND LIBRARIES ${Tensorflow_LIBRARIES})
    message("-- Neural network module enabled: ${Tensorflow_INCLUDE_DIRS} ${Tensorflow_LIBRARIES}")
    if(WIN32)
        # Some definitions needed to compile with tensorflow on windows
        # These are taken from tensorflow/contrib/cmake/CMakeLists.txt
        add_definitions(-DEIGEN_AVOID_STL_ARRAY)
        add_definitions(-DNOMINMAX -D_WIN32_WINNT=0x0A00 -DLANG_CXX11 -DCOMPILER_MSVC -D__VERSION__=\"MSVC\")
        add_definitions(-DWIN32 -DOS_WIN -D_MBCS -DWIN64 -DPLATFORM_WINDOWS)
        add_definitions(-DTENSORFLOW_USE_EIGEN_THREADPOOL -DEIGEN_HAS_C99_MATH -D_ITERATOR_DEBUG_LEVEL=0)
        # Suppress warnings to reduce build log size.
        add_definitions(/wd4267 /wd4244 /wd4800 /wd4503 /wd4554 /wd4996 /wd4348 /wd4018)
        add_definitions(/wd4099 /wd4146 /wd4267 /wd4305 /wd4307)
        add_definitions(/wd4715 /wd4722 /wd4723 /wd4838 /wd4309 /wd4334)
        # To get rid of this error: Not found: No session factory registered for the given session options: {target: "" config: } Registered factories are {}.
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WHOLEARCHIVE:\"${Tensorflow_LIBRARY}\"")
    endif()

	## Z lib for compressed data (zraw)
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        add_definitions("-DZLIB_ENABLED") # Tell FAST to use ZLIB
        list(APPEND FAST_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
    endif()
else()
    # If tensorflow neural network module is enabled, a newer version of Eigen is already included in the tensorflow include dir
    ## Eigen
    find_package(Eigen3 REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
    message("-- Using Eigen ${EIGEN3_VERSION}")
	
	## Z lib for compressed data (zraw)
    find_package(ZLIB)
    if(ZLIB_FOUND)
        add_definitions("-DZLIB_ENABLED") # Tell FAST to use ZLIB
        list(APPEND FAST_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
        list(APPEND LIBRARIES ${ZLIB_LIBRARIES})
    endif()
endif()

include(cmake/ModuleVTK.cmake)
include(cmake/ModuleITK.cmake)
include(cmake/ModuleOpenIGTLink.cmake)

include(cmake/Cpp11.cmake)
include(cmake/OpenMP.cmake)

#### Set include dirs
include_directories(${FAST_INCLUDE_DIRS})
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set (FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
endif()

#### Add all subdirs
fast_add_subdirectories(source/FAST)

if(FAST_BUILD_EXAMPLES)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
    if(WIN32)
        file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.bat "")
    else()
        file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.sh "#!/bin/bash\n")
    endif()
    add_subdirectory(source/FAST/Examples)
endif()

if(WIN32)
    # Build with multiple processes on windows
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()


#### Create FAST library and executables
add_library(FAST ${FAST_SOURCE_FILES} ${HEADERS_MOC})

## Link everything
target_link_libraries(FAST ${LIBRARIES})
qt5_use_modules(FAST Widgets OpenGL) # Add Qt 5 modules

## Build test executable
if(FAST_BUILD_TESTS)
    add_executable(testFAST ${FAST_TEST_SOURCE_FILES})
    target_link_libraries(testFAST FAST)
endif()


set(FAST_LIBRARY FAST)
set(FAST_LIBRARY_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create config file
configure_file (
        "${PROJECT_SOURCE_DIR}/source/FASTConfig.cmake.in"
        "${PROJECT_BINARY_DIR}/FASTConfig.cmake"
)

include(cmake/ModulePython.cmake)
include(cmake/InstallFAST.cmake)
