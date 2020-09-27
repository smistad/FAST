# Install script for directory: /home/andrep/workspace/forks/FAST/source/FAST/Visualization

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/BoundingBoxRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/HeatmapRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/ImagePyramidRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/ImageRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/LineRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/Plotting/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/SegmentationLabelRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/SegmentationPyramidRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/SegmentationRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/SliceRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/Tests/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/TextRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/TriangleRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/VectorFieldRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/VertexRenderer/cmake_install.cmake")
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/Visualization/VolumeRenderer/cmake_install.cmake")

endif()

