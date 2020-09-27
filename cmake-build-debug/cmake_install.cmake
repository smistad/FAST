# Install script for directory: /home/andrep/workspace/forks/FAST

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so.3.1.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so.1"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "$ORIGIN/../lib")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/lib" TYPE SHARED_LIBRARY FILES
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib/libFAST.so.3.1.1"
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib/libFAST.so.1"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so.3.1.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so.1"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib:"
           NEW_RPATH "$ORIGIN/../lib")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so"
         RPATH "$ORIGIN/../lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/lib" TYPE SHARED_LIBRARY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib/libFAST.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so"
         OLD_RPATH "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib:"
         NEW_RPATH "$ORIGIN/../lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/lib/libFAST.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST"
         RPATH "$ORIGIN/../lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/bin" TYPE EXECUTABLE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/bin/testFAST")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST"
         OLD_RPATH "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib:"
         NEW_RPATH "$ORIGIN/../lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/fast/bin/testFAST")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/FASTExport.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/andrep/workspace/forks/FAST/cmake/FixRPaths.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/plugins/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/plugins/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/bin" TYPE FILE PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/bin/moc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/FAST/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/FAST/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/FAST/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/FAST/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/CL/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/CL/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/CL/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/CL/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/eigen3/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/eigen3/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/eigen3/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/eigen3/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/eigen3/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/eigen3/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtAccessibilitySupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtAccessibilitySupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtAccessibilitySupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtAccessibilitySupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtAccessibilitySupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtAccessibilitySupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtConcurrent/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtConcurrent/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtConcurrent/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtConcurrent/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtConcurrent/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtConcurrent/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtCore/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtCore/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtCore/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtCore/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtCore/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtCore/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtDBus/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtDBus/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtDBus/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtDBus/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtDBus/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtDBus/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtDeviceDiscoverySupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtDeviceDiscoverySupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtDeviceDiscoverySupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtDeviceDiscoverySupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtDeviceDiscoverySupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtDeviceDiscoverySupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtEventDispatcherSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtEventDispatcherSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtEventDispatcherSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtEventDispatcherSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtEventDispatcherSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtEventDispatcherSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtFbSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtFbSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtFbSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtFbSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtFbSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtFbSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtFontDatabaseSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtFontDatabaseSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtFontDatabaseSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtFontDatabaseSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtFontDatabaseSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtFontDatabaseSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtGui/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtGui/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtGui/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtGui/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtGui/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtGui/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtMultimedia/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtMultimedia/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtMultimedia/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtMultimedia/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtMultimedia/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtMultimedia/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtMultimediaWidgets/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtMultimediaWidgets/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtMultimediaWidgets/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtMultimediaWidgets/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtMultimediaWidgets/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtMultimediaWidgets/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtNetwork/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtNetwork/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtNetwork/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtNetwork/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtNetwork/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtNetwork/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtOpenGL/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtOpenGL/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtOpenGL/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtOpenGL/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtOpenGL/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtOpenGL/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtOpenGLExtensions/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtOpenGLExtensions/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtOpenGLExtensions/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtOpenGLExtensions/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtOpenGLExtensions/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtOpenGLExtensions/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPlatformCompositorSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPlatformCompositorSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPlatformCompositorSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPlatformCompositorSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPlatformCompositorSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPlatformCompositorSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPlatformHeaders/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPlatformHeaders/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPlatformHeaders/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPlatformHeaders/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPlatformHeaders/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPlatformHeaders/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPrintSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPrintSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPrintSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPrintSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtPrintSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtPrintSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSerialPort/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSerialPort/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSerialPort/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSerialPort/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSerialPort/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSerialPort/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSql/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSql/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSql/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSql/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSql/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSql/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSvg/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSvg/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSvg/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSvg/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtSvg/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtSvg/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtTest/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtTest/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtTest/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtTest/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtTest/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtTest/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtThemeSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtThemeSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtThemeSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtThemeSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtThemeSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtThemeSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtWidgets/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtWidgets/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtWidgets/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtWidgets/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtWidgets/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtWidgets/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtXml/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtXml/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtXml/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtXml/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtXml/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtXml/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtZlib/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtZlib/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtZlib/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtZlib/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtZlib/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtZlib/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtGlxSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtGlxSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtGlxSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtGlxSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtGlxSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtGlxSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtServiceSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtServiceSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtServiceSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtServiceSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtServiceSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtServiceSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtInputSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtInputSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtInputSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtInputSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtInputSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtInputSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtKmsSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtKmsSupport/" FILES_MATCHING REGEX "/[^/]*\\.h$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtKmsSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtKmsSupport/" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/QtKmsSupport/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/include/QtKmsSupport/" FILES_MATCHING REGEX "/[^.]+$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/include/FAST" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/ProcessObjectList.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/kernels/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/FAST/" FILES_MATCHING REGEX "/[^/]*\\.cl$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/kernels/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/FAST/" FILES_MATCHING REGEX "/[^/]*\\.vert$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/kernels/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/source/FAST/" FILES_MATCHING REGEX "/[^/]*\\.frag$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/cmake" TYPE FILE FILES
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/FASTConfig.cmake"
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/FASTUse.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/cmake" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake/FindOpenCL.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/doc/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/doc/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/pipelines/" TYPE DIRECTORY FILES "/home/andrep/workspace/forks/FAST/pipelines/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/bin" TYPE FILE RENAME "fast_configuration.txt" FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/fast_configuration_install.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/fast" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/LICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast" TYPE FILE RENAME "README.md" FILES "/home/andrep/workspace/forks/FAST/cmake/InstallFiles/README_default.md")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/zlib" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/zlib/src/zlib/README")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/zip" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/zip/src/zip/UNLICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/OpenIGTLink" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenIGTLink/src/OpenIGTLink/LICENSE.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/dcmtk" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/dcmtk/src/dcmtk/COPYRIGHT")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/numpy" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake/InstallFiles/NumPy_LICENSE.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/eigen-swig" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake/InstallFiles/Eigen_SWIG_interface_LICENSE.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/semaphore" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake/InstallFiles/Semaphore_LICENSE.txt")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/openvino" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/OpenVINO/src/OpenVINO/LICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/lib" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/lib/plugins.xml")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/realsense" TYPE FILE FILES
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/realsense/src/realsense/LICENSE"
    "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/realsense/src/realsense/NOTICE"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/licenses/clarius" TYPE FILE FILES "/home/andrep/workspace/forks/FAST/cmake-build-debug/external/clarius/src/clarius_headers/LICENSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xfastx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/fast/kernel_binaries" TYPE DIRECTORY DIR_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE GROUP_READ GROUP_EXECUTE GROUP_WRITE WORLD_READ WORLD_WRITE WORLD_EXECUTE FILES "")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/andrep/workspace/forks/FAST/cmake-build-debug/source/FAST/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/andrep/workspace/forks/FAST/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
