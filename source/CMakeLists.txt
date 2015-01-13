cmake_minimum_required(VERSION 2.8.9)
project(FAST)

#### Options
option(VTK_INTEROP "Enable interoperability with VTK" OFF)
option(ITK_INTEROP "Enable interoperability with ITK" OFF)
option(BUILD_TESTS "Build tests." ON)
option(BUILD_EXAMPLES "Build examples." OFF)
set(FAST_TEST_DATA_DIR "${PROJECT_SOURCE_DIR}/../TestData/" CACHE STRING "Directory of test data")

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
macro (fast_add_subdirectories)
    file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        list (APPEND FAST_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${_src}")
        add_subdirectory(${_src})
    endforeach()
    if (_relPath)
        # propagate FAST_INCLUDE_DIRS to parent directory
        set (FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
    endif()
endmacro()


fast_add_sources(
    Object.cpp
    Object.hpp
    SmartPointers.hpp
    ProcessObject.cpp
    ProcessObject.hpp
    ExecutionDevice.cpp
    ExecutionDevice.hpp
    DeviceManager.cpp
    DeviceManager.hpp
    Exception.cpp
    Exception.hpp
    Utility.cpp
    Utility.hpp
    SceneGraph.cpp
    SceneGraph.hpp
    LinearTransformation.cpp
    LinearTransformation.hpp
)

# Headers for Qt objects
set(QT_HEADERS
    Visualization/View.hpp
    Visualization/WindowWidget.hpp
)

# Set up some directory defines
add_definitions("-DFAST_ROOT_DIR=\"${PROJECT_SOURCE_DIR}/../\"")
add_definitions("-DFAST_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}/\"")
# Remove any existing "
string(REPLACE "\"" "" FAST_TEST_DATA_DIR ${FAST_TEST_DATA_DIR})
message("-- FAST test data directory set to: \"${FAST_TEST_DATA_DIR}\"")
add_definitions("-DFAST_TEST_DATA_DIR=\"${FAST_TEST_DATA_DIR}\"")

#### Set up library dependencies
set(FAST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

## OpenCL Utility Library
add_subdirectory(OpenCLUtilityLibrary)
find_package(OpenCLUtilityLibrary PATHS "${FAST_BINARY_DIR}/OpenCLUtilityLibrary" REQUIRED)
include(${OpenCLUtilityLibrary_USE_FILE})
set(LIBRARIES ${LIBRARIES} OpenCLUtilityLibrary)
list (APPEND FAST_INCLUDE_DIRS "${OpenCLUtilityLibrary_INCLUDE_DIRS}")

## Qt
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
qt5_wrap_cpp(HEADERS_MOC ${QT_HEADERS})
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})
list (APPEND FAST_INCLUDE_DIRS "${Qt5Widgets_INCLUDE_DIRS}")
list (APPEND FAST_INCLUDE_DIRS "${Qt5OpenGL_INCLUDE_DIRS}")

## Boost
find_package(Boost COMPONENTS system thread iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})
set(LIBRARIES ${LIBRARIES} ${Boost_LIBRARIES})

## GLEW
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_PATH})
set(LIBRARIES ${LIBRARIES} ${GLEW_LIBRARY})

## Eigen
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

## VTK
if(VTK_INTEROP)
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
    message("-- Enabling VTK interoperability")
    set(LIBRARIES ${LIBRARIES} ${VTK_LIBRARIES})
endif()

## ITK
if(ITK_INTEROP)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
    message("-- Enabling ITK interoperability")
    set(LIBRARIES ${LIBRARIES} ${ITK_LIBRARIES})
endif()


#### Add all subdirs
fast_add_subdirectories(Algorithms Data Streamers Importers Exporters Visualization Tests)

#### Set include dirs
include_directories(${FAST_INCLUDE_DIRS})
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set (FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
endif()

#### Enable C++11
# Note that Microsoft Visual C++ compiler enables C++11 by default

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(--std=c++11 SUPPORTS_STD_CXX11)
    check_cxx_compiler_flag(--std=c++0x SUPPORTS_STD_CXX01)
    if(SUPPORTS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c++11")
        # Permissive flag is currently needed to make ITK work with C++11
        if(ITK_INTEROP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
        endif()
    elseif(SUPPORTS_STD_CXX01)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c++0x")
        # Permissive flag is currently needed to make ITK work with C++11
        if(ITK_INTEROP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")
        endif()
    else()
        message(ERROR "Compiler does not support --std=c++11 or --std=c++0x.")
    endif()
endif()

#### Create FAST library and executables
add_library(FAST ${FAST_SOURCE_FILES} ${HEADERS_MOC})
if(BUILD_TESTS)
    add_executable(test ${FAST_TEST_SOURCE_FILES} ${OUL_TEST_SOURCE_FILES})
endif()

## Link everything
target_link_libraries(FAST ${LIBRARIES})
qt5_use_modules(FAST Widgets OpenGL) # Add Qt 5 modules
if(BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif()
if(BUILD_TESTS)
    target_link_libraries(test FAST)
endif()
