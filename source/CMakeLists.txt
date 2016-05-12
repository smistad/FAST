cmake_minimum_required(VERSION 2.8.9)
project(FAST)

#### Options
option(FAST_VTK_INTEROP "Enable interoperability with VTK" OFF)
option(FAST_ITK_INTEROP "Enable interoperability with ITK" OFF)
option(FAST_BUILD_TESTS "Build FAST tests." ON)
option(BUILD_IGTLINK_TESTS "Build OpenIGTLink tests." OFF)
option(BUILD_EXAMPLES "Build OpenIGTLink examples." OFF)
option(BUILD_SHARED_LIBS "Build shared libs" ON)
option(FAST_BUILD_EXAMPLES "Build examples." OFF)
option(FAST_MODULE_OpenIGTLink "Build module OpenIGTLink" ON)
option(FAST_MODULE_DeepLearning "Build deep learning module with caffe" OFF)
option(FAST_MODULE_Python "Build Python wrappers" ON)
option(FAST_CONTINUOUS_INTEGRATION "Used for continuous integration tests" OFF)

#### Set build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("-- Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

## Set continous integration define
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

#### Paths
set(FAST_TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/../TestData/" CACHE PATH "Directory of test data")
set(FAST_TEST_DATA_DIR "${FAST_TEST_DATA_DIR}/") # Add a slash to the back
set(FAST_OPENCL_KERNEL_BINARY_PATH "${PROJECT_BINARY_DIR}/" CACHE PATH "Directory to put kernel binaries in. Default is build dir.")
set(FAST_OPENCL_KERNEL_BINARY_PATH "${FAST_OPENCL_KERNEL_BINARY_PATH}/") # Add a slash to the back
set(FAST_SOURCE_DIR "${PROJECT_SOURCE_DIR}/FAST/")
# Remove any existing "
string(REPLACE "\"" "" FAST_OPENCL_KERNEL_BINARY_PATH ${FAST_OPENCL_KERNEL_BINARY_PATH})
string(REPLACE "\"" "" FAST_TEST_DATA_DIR ${FAST_TEST_DATA_DIR})
message("-- OpenCL Kernel binary path set to: ${FAST_OPENCL_KERNEL_BINARY_PATH}")
message("-- FAST test data directory set to: \"${FAST_TEST_DATA_DIR}\"")

### Create the header files Paths.hpp and TestDataPath.hpp which include all useful paths
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Paths.hpp.in 
	${CMAKE_CURRENT_BINARY_DIR}/FAST/Paths.hpp
)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/TestDataPath.hpp.in 
	${CMAKE_CURRENT_BINARY_DIR}/FAST/TestDataPath.hpp
)

#### Module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../ ${CMAKE_MODULE_PATH}) # For finding the custom Find modules

### IGT Link tests
if(BUILD_IGTLINK_TESTS)
    set(BUILD_TESTING ON CACHE INTERNAL "Build OpenIGTLink tests." FORCE)
else()
    set(BUILD_TESTING OFF CACHE INTERNAL "Build OpenIGTLink tests." FORCE)
endif()


#### Macro for adding source files and directories
macro (fast_add_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND FAST_SOURCE_FILES "${_relPath}/${_src}")
        else()
            list (APPEND FAST_SOURCE_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate FAST_SOURCE_FILES to parent directory
        set (FAST_SOURCE_FILES ${FAST_SOURCE_FILES} PARENT_SCOPE)
    endif()
endmacro()
macro (fast_add_test_sources)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND FAST_TEST_SOURCE_FILES "${_relPath}/${_src}")
        else()
            list (APPEND FAST_TEST_SOURCE_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate FAST_TEST_SOURCE_FILES to parent directory
        set (FAST_TEST_SOURCE_FILES ${FAST_TEST_SOURCE_FILES} PARENT_SCOPE)
    endif()
endmacro()
macro (fast_add_python_interfaces)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND FAST_PYTHON_INTERFACE_FILES "${_relPath}/${_src}")
        else()
            list (APPEND FAST_PYTHON_INTERFACE_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate FAST_PYTHON_INTERFACE_FILES to parent directory
        set (FAST_PYTHON_INTERFACE_FILES ${FAST_PYTHON_INTERFACE_FILES} PARENT_SCOPE)
    endif()
endmacro()
macro (fast_add_subdirectories)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        add_subdirectory(${_src})
    endforeach()
    if (_relPath)
        # propagate to parent directory
        set (FAST_TEST_SOURCE_FILES ${FAST_TEST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_SOURCE_FILES ${FAST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_PYTHON_INTERFACE_FILES ${FAST_PYTHON_INTERFACE_FILES} PARENT_SCOPE)
	endif()
endmacro()
macro (fast_add_all_subdirectories)
	file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
	foreach(child ${children})
		if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
			add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${child})
		endif()
    endforeach()
    if (_relPath)
        # propagate to parent directory
        set (FAST_TEST_SOURCE_FILES ${FAST_TEST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_SOURCE_FILES ${FAST_SOURCE_FILES} PARENT_SCOPE)
        set (FAST_PYTHON_INTERFACE_FILES ${FAST_PYTHON_INTERFACE_FILES} PARENT_SCOPE)
	endif()
endmacro()	


# Headers for Qt objects
set(QT_HEADERS
    FAST/Visualization/View.hpp
    FAST/Visualization/WindowWidget.hpp
    FAST/Visualization/Window.hpp
    FAST/Visualization/ComputationThread.hpp
    FAST/Examples/Interoperability/qtInteroperability.hpp
)

# Set debug define if debug mode is set
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message("-- FAST Debug mode set")
    add_definitions("-DFAST_DEBUG")
endif()

#### Set up library dependencies
set(FAST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

## OpenIGTLink
if(FAST_MODULE_OpenIGTLink)
    set(CMAKE_PREFIX_PATH "${FAST_BINARY_DIR}") # Needed to find the cmake config file of IGTL
    set(LIBRARY_OUTPUT_PATH  "${FAST_BINARY_DIR}") # Needed to output the libraries in the correct folder
    set(EXECUTABLE_OUTPUT_PATH "${FAST_BINARY_DIR}") # Needed to output the executables in correct folder
    add_subdirectory(OpenIGTLink)
    find_package(OpenIGTLink PATHS "${FAST_BINARY_DIR}/OpenIGTLink" REQUIRED)
    include(${OpenIGTLink_USE_FILE})
    list(APPEND FAST_INCLUDE_DIRS ${OpenIGTLink_INCLUDE_DIRS})
    list(APPEND LIBRARIES ${OpenIGTLink_LIBRARIES})
endif()

## OpenCL
find_package(OpenCL REQUIRED)
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS "${OpenCL_INCLUDE_DIRS}")
message("-- OpenCL include dir: ${OpenCL_INCLUDE_DIRS}")

## OpenGL
find_package(OpenGL REQUIRED)
list(APPEND LIBRARIES ${OPENGL_LIBRARIES})
list(APPEND FAST_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
# If OS is Linux, add X explicitly to library list
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    list(APPEND LIBRARIES X11)
endif()


## Qt
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
qt5_wrap_cpp(HEADERS_MOC ${QT_HEADERS})
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})
list (APPEND FAST_INCLUDE_DIRS "${Qt5Widgets_INCLUDE_DIRS}")
list (APPEND FAST_INCLUDE_DIRS "${Qt5OpenGL_INCLUDE_DIRS}")

## Boost
find_package(Boost COMPONENTS system thread iostreams signals filesystem chrono REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})
set(LIBRARIES ${LIBRARIES} ${Boost_LIBRARIES})

## GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_PATH})
set(LIBRARIES ${LIBRARIES} ${GLEW_LIBRARY})

## Eigen
find_package(Eigen3 REQUIRED)
list(APPEND FAST_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR}) 
message("-- Using Eigen ${EIGEN3_VERSION}")

## Deep learning module
if(FAST_MODULE_DeepLearning)
	# Set some variables for viennaCL, mark these as internal as we don't want user to change them
    set(ViennaCL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/viennacl-dev/" CACHE INTERNAL "")
    set(OPENCL_INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS} CACHE INTERNAL "")
    set(OPENCL_LIBRARIES ${OpenCL_LIBRARIES} CACHE INTERNAL "")
	add_subdirectory(viennacl-dev)
    find_package(ViennaCL PATHS "${FAST_BINARY_DIR}/viennacl-dev" REQUIRED)
    
    # Turn off a bunch of caffe things FAST don't need
    # Use CACHE here so that their value will show in cmake-gui
    set(BUILD_TOOLS OFF CACHE BOOL "Build caffe tools")
    set(BUILD_docs OFF CACHE BOOL "Build caffe docs")
    set(BUILD_python OFF CACHE BOOL "Build caffe python")
    set(BUILD_python_layers OFF CACHE BOOL "Build caffe python layers")
    set(USE_HDF5 OFF CACHE BOOL "Build caffe with HDF5 support")
    set(USE_CUDA OFF CACHE INTERNAL "") # FAST uses OpenCL
    set(USE_CUDNN OFF CACHE INTERNAL "") # FAST uses OpenCL
    set(USE_OPENCV OFF CACHE BOOL "Build caffe with OpenCV support")
    set(USE_LEVELDB OFF CACHE BOOL "Build caffe with leveldb support")
    set(USE_LMDB OFF CACHE BOOL "Build caffe with LMDB support")

	add_subdirectory(caffe)
    find_package(Caffe PATHS "${FAST_BINARY_DIR}/caffe" REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/caffe/include/")
    list(APPEND FAST_INCLUDE_DIRS "${FAST_BINARY_DIR}/caffe/include/")
    list(APPEND LIBRARIES ${Caffe_LIBRARIES})
endif()

## VTK
if(FAST_VTK_INTEROP)
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
    message("-- Enabling VTK interoperability. VTK version ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}")
    set(LIBRARIES ${LIBRARIES} ${VTK_LIBRARIES})
endif()

## ITK
if(FAST_ITK_INTEROP)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
    message("-- Enabling ITK interoperability")
    set(LIBRARIES ${LIBRARIES} ${ITK_LIBRARIES})
endif()

## Z lib for compressed data (zraw)
find_package(ZLIB)
if(ZLIB_FOUND)
    add_definitions("-DZLIB_ENABLED") # Tell FAST to use ZLIB
    include_directories(${ZLIB_INCLUDE_DIRS})
    set(LIBRARIES ${LIBRARIES} ${ZLIB_LIBRARIES})
endif()

#### Set include dirs
include_directories(${FAST_INCLUDE_DIRS})
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set (FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
endif()

#### Enable C++11
# Note that Microsoft Visual C++ compiler enables C++11 by default

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=300") # Fix for a bug with boost signals2
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations") # Remove deprecated warnings GCC
endif()
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_STD_CXX11)
    check_cxx_compiler_flag(--std=c++0x SUPPORTS_STD_CXX01)
    if(SUPPORTS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c++11")
        # Permissive flag is currently needed to make ITK work with C++11
        if(FAST_ITK_INTEROP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
        endif()
    elseif(SUPPORTS_STD_CXX01)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c++0x")
        # Permissive flag is currently needed to make ITK work with C++11
        if(FAST_ITK_INTEROP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
        endif()
    else()
        message(ERROR "Compiler does not support --std=c++11 or --std=c++0x.")
    endif()
endif()

if(FAST_BUILD_EXAMPLES)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
	if(WIN32)
		file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.bat "")
	else()
		file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.sh "#!/bin/bash\n")
	endif()

	### Macro for add examples
	macro (fast_add_example NAME)
		add_executable(${NAME} ${ARGN})
		target_link_libraries(${NAME} FAST)
		if(WIN32)
			file(APPEND ${PROJECT_BINARY_DIR}/runAllExamples.bat "bin\\${CMAKE_BUILD_TYPE}\\${NAME}.exe\r\n")
		else()
			file(APPEND ${PROJECT_BINARY_DIR}/runAllExamples.sh "./bin/${NAME}\n")
		endif()
	endmacro()
else()
	macro (fast_add_example NAME)
	endmacro()
endif()

#### Add all subdirs
fast_add_subdirectories(FAST)
add_subdirectory(FAST/Examples)




#### OpenMP
find_package(OpenMP)
if(OPENMP_FOUND)
    message("-- OpenMP was detected. Using OpenMP to speed up some calculations.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}" )
endif()



#### Create FAST library and executables
add_library(FAST ${FAST_SOURCE_FILES} ${HEADERS_MOC})
if(FAST_BUILD_TESTS)
	add_executable(testFAST ${FAST_TEST_SOURCE_FILES})
endif()

## Link everything
target_link_libraries(FAST ${LIBRARIES})
qt5_use_modules(FAST Widgets OpenGL) # Add Qt 5 modules

if(FAST_BUILD_TESTS)
    target_link_libraries(testFAST FAST)
endif()


set(FAST_LIBRARY FAST)
set(FAST_LIBRARY_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create config file
configure_file (
    "${PROJECT_SOURCE_DIR}/FASTConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/FASTConfig.cmake"
)

find_package(SWIG)
if(SWIG_FOUND AND FAST_MODULE_Python)
	message("-- SWIG found, creating python bindings...")
	include(${SWIG_USE_FILE})
	find_package(PythonLibs REQUIRED)
	include_directories(${PYTHON_INCLUDE_PATH})

	include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    set(CMAKE_SWIG_FLAGS "") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWIG_PYTHON_INTERPRETER_NO_DEBUG") # Avoid a error on windows when compiling in debug mode
	
	# Generate the PyFAST interface file
	# Include all header files
	foreach(FILE ${FAST_SOURCE_FILES})
		if(${FILE} MATCHES "^.*hpp$")
			set(PYFAST_HEADER_INCLUDES "${PYFAST_HEADER_INCLUDES}#include \"${FILE}\"\n")
		endif()
	endforeach()
	
	# Include all python interface files
	foreach(FILE ${FAST_PYTHON_INTERFACE_FILES})
		set(PYFAST_INTERFACE_INCLUDES "${PYFAST_INTERFACE_INCLUDES}%include \"${FILE}\"\n")
	endforeach()
	
	set(PYFAST_FILE "${PROJECT_BINARY_DIR}/PyFAST.i")
	configure_file(
		"${PROJECT_SOURCE_DIR}/PyFAST.i.in"
		${PYFAST_FILE}
	)
	
	# Build it
	set_source_files_properties(${PYFAST_FILE} PROPERTIES GENERATED TRUE)
	set_source_files_properties(${PYFAST_FILE} PROPERTIES CPLUSPLUS ON)
	swig_add_module(fast python ${PYFAST_FILE})
	swig_link_libraries(fast ${PYTHON_LIBRARIES} FAST)
	
	# Move python file to lib folder
	add_custom_command(TARGET _fast POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E rename fast.py ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/fast.py
	)
else()
    message("-- SWIG not found or Python module not enabled in CMake, Python bindings will NOT be created.")
endif()
