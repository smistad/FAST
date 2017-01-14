cmake_minimum_required(VERSION 2.8.12)
project(FAST)
include(cmake/Macros.cmake)

#### Options
option(FAST_BUILD_TESTS "Build FAST tests." ON)
option(BUILD_IGTLINK_TESTS "Build OpenIGTLink tests." OFF)
option(BUILD_EXAMPLES "Build OpenIGTLink examples." OFF)
option(BUILD_SHARED_LIBS "Build shared libs" ON)
option(FAST_BUILD_EXAMPLES "Build examples." OFF)
option(FAST_DOWNLOAD_TEST_DATA "Download test data automatically (> 1 GB)" ON)
option(FAST_MODULE_VTK "Enable interoperability with VTK" OFF)
option(FAST_MODULE_ITK "Enable interoperability with ITK" OFF)
option(FAST_MODULE_Visualization "Enable visualization capabilities using Qt5.
Without this module, all visualization in FAST will be disabled." ON)
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
cmake_policy(SET CMP0043 OLD)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    # Force libs and bins to go into bin and lib directory instead of Release/Debug
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

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

# Set FAST include dirs
set(FAST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/ ${CMAKE_CURRENT_BINARY_DIR})

# Some extra build settings (C++11 and OpenMP)
include(cmake/Cpp11.cmake)
include(cmake/OpenMP.cmake)

# This will setup all of FAST's depedencies
include(cmake/Depdendencies.cmake)

# This will download the test data
include(cmake/DownloadTestData.cmake)

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

#### Create FAST library and executables
if(WIN32)
	set(BUILD_SHARED_LIBS OFF)
	# Parallel build in MSVC
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "/MP")
endif()
add_library(FAST ${FAST_SOURCE_FILES} ${HEADERS_MOC})
add_dependencies(FAST ${FAST_EXTERNAL_DEPENDENCIES})

## Link everything
target_link_libraries(FAST ${LIBRARIES})
if(${FAST_MODULE_Visualization})
    qt5_use_modules(FAST Widgets OpenGL) # Add Qt 5 modules
endif()

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
