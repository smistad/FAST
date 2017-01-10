cmake_minimum_required(VERSION 2.8.9)
project(FAST)
include(../cmake/macros.cmake)

#### Options
option(FAST_VTK_INTEROP "Enable interoperability with VTK" OFF)
option(FAST_ITK_INTEROP "Enable interoperability with ITK" OFF)
option(FAST_BUILD_TESTS "Build FAST tests." ON)
option(BUILD_IGTLINK_TESTS "Build OpenIGTLink tests." OFF)
option(BUILD_EXAMPLES "Build OpenIGTLink examples." OFF)
option(BUILD_SHARED_LIBS "Build shared libs" ON)
option(FAST_BUILD_EXAMPLES "Build examples." OFF)
option(FAST_MODULE_OpenIGTLink "Build module OpenIGTLink" ON)
option(FAST_MODULE_NeuralNetwork "Build neural network module using tensorflow" OFF)
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

## Turn off shared libs on windows
if(WIN32)
	set(BUILD_SHARED_LIBS OFF)
endif()

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

## GLEW
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_PATH})
set(LIBRARIES ${LIBRARIES} ${GLEW_LIBRARY})


## Neural network module
if(FAST_MODULE_NeuralNetwork)
    ## Tensorflow
    find_package(Tensorflow REQUIRED)
    list(APPEND FAST_INCLUDE_DIRS ${Tensorflow_INCLUDE_DIRS})
    list(APPEND LIBRARIES ${Tensorflow_LIBRARIES})
    message("-- Neural network module enabled: ${Tensorflow_INCLUDE_DIRS} ${Tensorflow_LIBRARIES}")
	add_definitions("-DZLIB_ENABLED") # Tell FAST to use ZLIB
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
		include_directories(${ZLIB_INCLUDE_DIRS})
		set(LIBRARIES ${LIBRARIES} ${ZLIB_LIBRARIES})
	endif()
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



#### Set include dirs
include_directories(${FAST_INCLUDE_DIRS})
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set (FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
endif()

#### Enable C++11
# Note that Microsoft Visual C++ compiler enables C++11 by default
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations") # Remove deprecated warnings GCC
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=300") # Fix for a bug with boost signals2
endif()
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
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

#### Add all subdirs
fast_add_subdirectories(FAST)

if(FAST_BUILD_EXAMPLES)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
	if(WIN32)
		file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.bat "")
	else()
		file(WRITE ${PROJECT_BINARY_DIR}/runAllExamples.sh "#!/bin/bash\n")
	endif()
    add_subdirectory(FAST/Examples)
endif()

if(WIN32)
	# Build with multiple processes on windows
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()


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
