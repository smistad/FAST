cmake_minimum_required(VERSION 2.8)
project(FAST)

option(VTK_INTEROP "Enable interoperability with VTK" OFF)
option(ITK_INTEROP "Enable interoperability with ITK" OFF)

set(SOURCES 
    main.cpp
    SmartPointers.hpp
    PipelineObject.cpp
    PipelineObject.hpp
    ExecutionDevice.cpp
    ExecutionDevice.hpp
    DeviceManager.cpp
    DeviceManager.hpp
    Exception.cpp
    Exception.hpp
    Algorithms/GaussianSmoothingFilter2D.cpp
    Algorithms/GaussianSmoothingFilter2D.hpp
    Data/ImageData.cpp
    Data/ImageData.hpp
    Data/Image2D.cpp
    Data/Image2D.hpp
    Data/Image2Dt.cpp
    Data/Image2Dt.hpp
    Data/DataTypes.hpp
    Data/Access/Access.hpp
    Data/Access/OpenCLImageAccess2D.cpp
    Data/Access/OpenCLImageAccess2D.hpp
    Data/Access/ImageAccess2D.cpp
    Data/Access/ImageAccess2D.hpp
    Streamers/Streamer.cpp
    Streamers/Streamer.hpp
    Streamers/ImageStreamer2D.cpp
    Streamers/ImageStreamer2D.hpp
    Importers/ImageImporter2D.cpp
    Importers/ImageImporter2D.hpp
    Importers/Importer.hpp
    Exporters/ImageExporter2D.cpp
    Exporters/ImageExporter2D.hpp
)

if(VTK_INTEROP)
    set(SOURCES ${SOURCES}
        Exporters/VTKImageExporter.cpp
        Exporters/VTKImageExporter.hpp
        Importers/VTKImageImporter.cpp
        Importers/VTKImageImporter.hpp
    )
endif()

if(ITK_INTEROP)
    set(SOURCES ${SOURCES}
        Exporters/ITKImageExporter.hpp
    )
endif()

include_directories(
    Data/
    Importers/
    Exporters/
    Streamers/
    Algorithms/
    Data/Access/
)

# Headers for Qt objects
set(HEADERS 
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFAST_ROOT_DIR=\\\"${PROJECT_SOURCE_DIR}/\\\"")


add_subdirectory(OpenCLUtilityLibrary)
find_package(OpenCLUtilityLibrary PATHS "${FAST_BINARY_DIR}/OpenCLUtilityLibrary" REQUIRED)
include(${OpenCLUtilityLibrary_USE_FILE})

set(QT_USE_QTOPENGL TRUE)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
QT4_WRAP_CPP(HEADERS_MOC ${HEADERS})

find_package(Boost COMPONENTS thread iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test ${SOURCES} ${HEADERS_MOC})
set(LIBRARIES
    OpenCLUtilityLibrary
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES}
)

if(VTK_INTEROP)
    find_package(VTK REQUIRED)
    include(${VTK_USE_FILE})
    message("-- Enabling VTK interoperability")
    set(LIBRARIES ${LIBRARIES} ${VTK_LIBRARIES})
endif()

if(ITK_INTEROP)
    find_package(ITK REQUIRED)
    include(${ITK_USE_FILE})
    message("-- Enabling ITK interoperability")
    set(LIBRARIES ${LIBRARIES} ${ITK_LIBRARIES})
endif()

target_link_libraries(test ${LIBRARIES})
