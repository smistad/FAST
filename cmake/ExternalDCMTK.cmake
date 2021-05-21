# Download and set up DCMTK

include(cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
set(MODULES ofstd oflog dcmdata dcmimgle)

ExternalProject_Add(dcmtk
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/dcmtk
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/dcmtk
        GIT_REPOSITORY "https://github.com/DCMTK/dcmtk.git"
        GIT_TAG "DCMTK-3.6.3"
        CMAKE_ARGS
            -DCMAKE_MACOSX_RPATH=ON
            -DBUILD_SHARED_LIBS=ON
            -DBUILD_APPS=OFF
            -DDCMTK_ENABLE_BUILTIN_DICTIONARY=ON
            -DDCMTK_WITH_DOXYGEN=OFF
            -DDCMTK_WITH_ICU=OFF
            -DCMAKE_INSTALL_RPATH:STRING=$ORIGIN/../lib
        CMAKE_CACHE_ARGS
            -DDCMTK_MODULES:STRING=${MODULES}
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
)
else(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
else()
    ExternalProject_Add(dcmtk
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/dcmtk
            URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL}/linux/dcmtk_3.6.3_glibc2.27.tar.xz
            URL_HASH SHA256=47df5591dcf7e624ab889d6896c56eefa611348aed1f0410171d6f02f8025b81
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            # On install: Copy contents of each subfolder to the build folder
            INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
                ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
                ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licences
    )
endif()
endif(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
    set(DCMTK_LIBRARIES ofstd.lib oflog.lib dcmdata.lib dcmimgle.lib)
else(WIN32)
    set(DCMTK_LIBRARIES
        ${CMAKE_SHARED_LIBRARY_PREFIX}ofstd${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CMAKE_SHARED_LIBRARY_PREFIX}oflog${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CMAKE_SHARED_LIBRARY_PREFIX}dcmdata${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${CMAKE_SHARED_LIBRARY_PREFIX}dcmimgle${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
endif(WIN32)
list(APPEND LIBRARIES ${DCMTK_LIBRARIES})
list(APPEND FAST_EXTERNAL_DEPENDENCIES dcmtk)
