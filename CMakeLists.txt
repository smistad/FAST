cmake_minimum_required(VERSION 2.8)
project(FAST)

option(VTK_INTEROP "Enable interoperability with VTK" OFF)
option(ITK_INTEROP "Enable interoperability with ITK" OFF)

set(SOURCES 
    main.cpp
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
    Algorithms/GaussianSmoothingFilter.cpp
    Algorithms/GaussianSmoothingFilter.hpp
    Data/DataObject.cpp
    Data/DataObject.hpp
    Data/ImageData.cpp
    Data/ImageData.hpp
    Data/DynamicImage.cpp
    Data/DynamicImage.hpp
    Data/Image.cpp
    Data/Image.hpp
    Data/DataTypes.cpp
    Data/DataTypes.hpp
    Data/Access/Access.hpp
    Data/Access/OpenCLBufferAccess.cpp
    Data/Access/OpenCLBufferAccess.hpp
    Data/Access/OpenCLImageAccess2D.cpp
    Data/Access/OpenCLImageAccess2D.hpp
    Data/Access/OpenCLImageAccess3D.cpp
    Data/Access/OpenCLImageAccess3D.hpp
    Data/Access/ImageAccess.cpp
    Data/Access/ImageAccess.hpp
    Streamers/Streamer.cpp
    Streamers/Streamer.hpp
    Streamers/ImageStreamer2D.cpp
    Streamers/ImageStreamer2D.hpp
    Streamers/MetaImageStreamer.cpp
    Streamers/MetaImageStreamer.hpp
    Importers/MetaImageImporter.cpp
    Importers/MetaImageImporter.hpp
    Importers/ImageImporter2D.cpp
    Importers/ImageImporter2D.hpp
    Importers/Importer.hpp
    Exporters/ImageExporter2D.cpp
    Exporters/ImageExporter2D.hpp
    Exporters/MetaImageExporter.cpp
    Exporters/MetaImageExporter.hpp
    Visualization/SimpleWindow.cpp
    Visualization/SimpleWindow.hpp
    Visualization/View.cpp
    Visualization/View.hpp
    Visualization/Renderer.cpp
    Visualization/Renderer.hpp
    Visualization/ImageRenderer.cpp
    Visualization/ImageRenderer.hpp
    Visualization/SliceRenderer.cpp
    Visualization/SliceRenderer.hpp
    Visualization/SurfaceRenderer.cpp
    Visualization/SurfaceRenderer.hpp
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
        Importers/ITKImageImporter.hpp
    )
endif()

include_directories(
    Data/
    Importers/
    Exporters/
    Streamers/
    Algorithms/
    Data/Access/
    Visualization/
)

# Headers for Qt objects
set(HEADERS 
    Visualization/SimpleWindow.hpp
    Visualization/View.hpp
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

find_package(Boost COMPONENTS system thread iostreams REQUIRED)
include_directories(${Boost_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(examples ${SOURCES} ${HEADERS_MOC})
add_library(FAST ${SOURCES} ${HEADERS_MOC})

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

target_link_libraries(examples ${LIBRARIES})
target_link_libraries(FAST ${LIBRARIES})

add_subdirectory(Tests)
