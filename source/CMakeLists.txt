cmake_minimum_required(VERSION 2.8.9)
project(FAST)

option(VTK_INTEROP "Enable interoperability with VTK" OFF)
option(ITK_INTEROP "Enable interoperability with ITK" OFF)

#### Macro for adding source files and directories
macro (fast_add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
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
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
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
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND FAST_INCLUDE_DIRS "${_relPath}/${_src}")
        else()
            list (APPEND FAST_INCLUDE_DIRS "${_src}")
        endif()
        add_subdirectory(${_src})
    endforeach()
    if (_relPath)
        # propagate FAST_INCLUDE_DIRS to parent directory
        set (FAST_INCLUDE_DIRS ${FAST_INCLUDE_DIRS} PARENT_SCOPE)
    endif()
endmacro()

fast_add_sources(
    main.cpp
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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFAST_ROOT_DIR=\\\"${PROJECT_SOURCE_DIR}/../\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFAST_SOURCE_DIR=\\\"${PROJECT_SOURCE_DIR}/\\\"")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFAST_TEST_DATA_DIR=\\\"${PROJECT_SOURCE_DIR}/TestData/\\\"")

#### Set up library dependencies

## OpenCL Utility Library
add_subdirectory(OpenCLUtilityLibrary)
find_package(OpenCLUtilityLibrary PATHS "${FAST_BINARY_DIR}/OpenCLUtilityLibrary" REQUIRED)
include(${OpenCLUtilityLibrary_USE_FILE})
set(LIBRARIES ${LIBRARIES} OpenCLUtilityLibrary)

## Qt
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
qt5_wrap_cpp(HEADERS_MOC ${QT_HEADERS})
set(LIBRARIES ${LIBRARIES} ${QT_LIBRARIES})

## Boost
find_package(Boost COMPONENTS system thread iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})
set(LIBRARIES ${LIBRARIES} ${Boost_LIBRARIES})

## GLEW
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_PATH})
set(LIBRARIES ${LIBRARIES} ${GLEW_LIBRARY})

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Tests)

#### Add all subdirs
fast_add_subdirectories(Algorithms Data Streamers Importers Exporters Visualization Tests)
include_directories(${FAST_INCLUDE_DIRS})

#### Create FAST library and executables
add_library(FAST ${FAST_SOURCE_FILES} ${HEADERS_MOC})
add_executable(examples main.cpp)
add_executable(test ${FAST_TEST_SOURCE_FILES})

## Link everything
target_link_libraries(FAST ${LIBRARIES})
qt5_use_modules(FAST Widgets OpenGL) # Add Qt 5 modules
target_link_libraries(examples FAST)
target_link_libraries(test FAST)