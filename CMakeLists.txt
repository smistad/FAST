cmake_minimum_required(VERSION 2.8.11)

project(FAST)

include(cmake/Macros.cmake)

#### Options
option(FAST_BUILD_TESTS "Build FAST tests." ON)
option(FAST_BUILD_EXAMPLES "Build examples." OFF)
option(FAST_BUILD_TOOLS "Build tools." ON)
option(FAST_BUILD_DOCS "Build API documentation" OFF)
option(FAST_DOWNLOAD_TEST_DATA "Download test data automatically (> 1 GB)" OFF)
option(FAST_MODULE_VTK "Enable interoperability with VTK" OFF)
option(FAST_MODULE_ITK "Enable interoperability with ITK" OFF)
option(FAST_MODULE_Visualization "Enable visualization capabilities using Qt5.
Without this module, all visualization in FAST will be disabled." ON)
option(FAST_MODULE_OpenIGTLink "Build module OpenIGTLink" ON)
option(FAST_MODULE_NeuralNetwork "Build neural network module using tensorflow" OFF)
option(FAST_MODULE_Python "Build Python wrappers" OFF)
option(FAST_MODULE_Kinect "Build kinect module" OFF)
option(FAST_CONTINUOUS_INTEGRATION "Used for continuous integration tests" OFF)
option(FAST_BUILD_QT5 "Download and build Qt 5.8. Turn this OFF if you want to use pre-built binaries you already have on your machine." ON)

# Python version
set(FAST_Python_Version "" CACHE STRING "Specify which Python version to build python wrappers with. E.g. 2 or 3")

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
set(FAST_TEST_DATA_DIR "" CACHE PATH "Directory of test data. Default is ROOT/data/.")
set(FAST_OPENCL_KERNEL_BINARY_PATH "" CACHE PATH "Directory to put kernel binaries in. Default is build dir.")
set(FAST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/source/FAST/")

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
        source/FAST/PipelineEditor.hpp
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

if(FAST_BUILD_DOCS)
    include(cmake/Documentation.cmake)
endif()

#### Set include dirs
include_directories(${FAST_INCLUDE_DIRS})
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
endif()

# Set up RPATH with relative path so that binaries will find libraries in the lib folder (linux only?)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

if(WIN32)
	# Parallel build in MSVC
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /nologo /MD /EHsc")
endif()

#### Add all subdirs
fast_add_subdirectories(source/FAST)


if(FAST_BUILD_EXAMPLES)
    if(FAST_MODULE_Visualization)
        set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
        if(WIN32)
            file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.bat "")
        else()
            file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.sh "#!/bin/bash\n")
        endif()
        add_subdirectory(source/FAST/Examples)
    else()
        message("-- Not building examples since visualization module is disabled.")
    endif()
endif()



#### Create FAST library and executables
add_library(FAST SHARED ${FAST_SOURCE_FILES} ${HEADERS_MOC})
add_dependencies(FAST ${FAST_EXTERNAL_DEPENDENCIES})
include(GenerateExportHeader)
generate_export_header(FAST EXPORT_FILE_NAME FASTExport.hpp)


## Link everything
message("-- LIBS: ${LIBRARIES} ${FAST_SYSTEM_LIBRARIES}")
target_link_libraries(FAST PUBLIC ${LIBRARIES} ${FAST_SYSTEM_LIBRARIES})
if(FAST_MODULE_Visualization AND FAST_BUILD_QT5)
    qt5_use_modules(FAST Widgets OpenGL) # Add Qt 5 modules
endif()

## Build test executable
if(FAST_BUILD_TESTS)
    add_executable(testFAST ${FAST_TEST_SOURCE_FILES})
    target_link_libraries(testFAST FAST)
endif()

set(CONF_FAST_LIBRARIES FAST ${LIBRARIES})

# Create config and use file
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/FASTConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/FASTConfig.cmake"
        @ONLY
)
configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/FASTUse.cmake.in"
        "${PROJECT_BINARY_DIR}/FASTUse.cmake"
        @ONLY
)

# Set paths needed at runtime to be written to config file
set(CONFIG_TEST_DATA_PATH "TestDataPath = ${CMAKE_CURRENT_SOURCE_DIR}/data/")
set(CONFIG_DOCUMENTATION_PATH "DocumentationPath = ${CMAKE_CURRENT_SOURCE_DIR}/doc/")
set(CONFIG_KERNEL_SOURCE_PATH "KernelSourcePath = ${CMAKE_CURRENT_SOURCE_DIR}/source/FAST/")
set(CONFIG_PIPELINE_PATH "PipelinePath = ${CMAKE_CURRENT_SOURCE_DIR}/pipelines/")

# If test data dir has been set by user, use that instead
if(NOT FAST_TEST_DATA_DIR STREQUAL "")
    # Remove any existing "
    string(REPLACE "\"" "" FAST_TEST_DATA_DIR ${FAST_TEST_DATA_DIR})
    message("-- FAST test data directory set to: \"${FAST_TEST_DATA_DIR}\"")
    set(CONFIG_TEST_DATA_PATH "TestDataPath = ${FAST_TEST_DATA_DIR}/")
endif()
# If OpenCL kernel binary path has been set by user, write it to the config
if(NOT FAST_OPENCL_KERNEL_BINARY_PATH STREQUAL "")
    # Remove any existing "
    string(REPLACE "\"" "" FAST_OPENCL_KERNEL_BINARY_PATH ${FAST_OPENCL_KERNEL_BINARY_PATH})
    message("-- OpenCL Kernel binary path set to: ${FAST_OPENCL_KERNEL_BINARY_PATH}")
    set(CONFIG_KERNEL_BINARY_PATH "KernelBinaryPath = ${FAST_OPENCL_KERNEL_BINARY_PATH}/")
endif()

# configure the configuration file.
configure_file(
    "${PROJECT_SOURCE_DIR}/source/fast_configuration.txt.in"
    "${PROJECT_BINARY_DIR}/fast_configuration.txt"
)

foreach(FILE ${FAST_PROCESS_OBJECT_HEADER_FILES})
    set(FAST_PROCESS_OBJECT_INCLUDES "${FAST_PROCESS_OBJECT_INCLUDES}#include \"${FILE}\"\n")
endforeach()

foreach(NAME ${FAST_PROCESS_OBJECT_NAMES})
    set(FAST_PROCESS_OBJECT_REGISTRATION "${FAST_PROCESS_OBJECT_REGISTRATION}FAST_REGISTER_PO(${NAME})\n")
endforeach()

configure_file(
    "${PROJECT_SOURCE_DIR}/source/ProcessObjectList.hpp.in"
    "${PROJECT_BINARY_DIR}/ProcessObjectList.hpp"
)

include(cmake/ModulePython.cmake)
include(cmake/InstallFAST.cmake)
