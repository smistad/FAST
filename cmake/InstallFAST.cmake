# This will install FAST binaries, libraries and necessary include files to the path given by CMAKE_INSTALL_PREFIX

# Install FAST library
if(WIN32)
# DLL should be in binary folder
install(TARGETS FAST
	DESTINATION fast/bin
)
else()
install(TARGETS FAST
	DESTINATION fast/lib
)
endif()

if(FAST_BUILD_TESTS)
    # Install test executable
    install(TARGETS testFAST
        DESTINATION fast/bin
    )
endif()

# Examples are installed in the macro fast_add_example

# Install dependency libraries
install(FILES ${PROJECT_BINARY_DIR}/FASTExport.hpp
    DESTINATION fast/include
)
if(WIN32)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION fast/bin
	)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/lib/*.lib)
	install(FILES ${DLLs}
		DESTINATION fast/lib
	)
elseif(APPLE)
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.dylib)
	install(FILES ${SOs}
        DESTINATION fast/lib
    )
else()
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.so*)
	install(FILES ${SOs}
        DESTINATION fast/lib
    )
	# Fix RPaths on install
    install(SCRIPT cmake/FixRPaths.cmake)
endif()

# Install Qt plugins
install(DIRECTORY ${PROJECT_BINARY_DIR}/plugins/
    DESTINATION fast/QtPlugins/
)

# Install qt.conf
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/qt.conf
    DESTINATION fast/bin
)

# Install headers
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/include/FAST/
	FILES_MATCHING PATTERN "*.hpp"
)
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/include/FAST/
	FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/source/CL/
	DESTINATION fast/include/CL/
	FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/source/CL/
	DESTINATION fast/include/CL/
	FILES_MATCHING PATTERN "*.hpp"
)

# External include files needed
set(INCLUDE_FOLDERS
    eigen3
    QtAccessibilitySupport
    QtConcurrent
    QtCore
    QtDBus
    QtDeviceDiscoverySupport
    QtEventDispatcherSupport
    QtFbSupport
    QtFontDatabaseSupport
    QtGui
    QtMultimedia
    QtMultimediaWidgets
    QtNetwork
    QtOpenGL
    QtOpenGLExtensions
    QtPlatformCompositorSupport
    QtPlatformHeaders
    QtPrintSupport
    QtSerialPort
    QtSql
    QtTest
    QtThemeSupport
    QtWidgets
    QtXml
    QtZlib)
if(NOT WIN32)
list(APPEND INCLUDE_FOLDERS
    QtGlxSupport
		QtServiceSupport
		QtInputSupport
		QtKmsSupport
)
endif()
foreach(INCLUDE_FOLDER ${INCLUDE_FOLDERS})
    install(DIRECTORY ${PROJECT_BINARY_DIR}/include/${INCLUDE_FOLDER}/
        DESTINATION fast/include/${INCLUDE_FOLDER}/
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY ${PROJECT_BINARY_DIR}/include/${INCLUDE_FOLDER}/
        DESTINATION fast/include/${INCLUDE_FOLDER}/
        FILES_MATCHING PATTERN "*.hpp"
    )
    install(DIRECTORY ${PROJECT_BINARY_DIR}/include/${INCLUDE_FOLDER}/
        DESTINATION fast/include/${INCLUDE_FOLDER}/
        FILES_MATCHING REGEX "/[^.]+$" # Files with no extension
    )
endforeach()



# Install created headers
install(FILES ${PROJECT_BINARY_DIR}/ProcessObjectList.hpp
    DESTINATION fast/include/FAST/
)

# Install OpenCL kernels
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	FILES_MATCHING PATTERN "*.cl"
)

# Install GL shaders
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	FILES_MATCHING PATTERN "*.vert"
)

# Install GL shaders
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	FILES_MATCHING PATTERN "*.frag"
)


# Install CMake files
install(FILES ${PROJECT_BINARY_DIR}/FASTConfig.cmake ${PROJECT_BINARY_DIR}/FASTUse.cmake
    DESTINATION fast/cmake
)
install(FILES ${PROJECT_SOURCE_DIR}/cmake/FindOpenCL.cmake
    DESTINATION fast/cmake
)

# Install docs
install(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/
    DESTINATION fast/doc/
)

# Install pipelines
install(DIRECTORY ${PROJECT_SOURCE_DIR}/pipelines/
    DESTINATION fast/pipelines/
)

# Install Python wrapper
if(FAST_MODULE_Python)
install(TARGETS _fast
    DESTINATION fast/python/fast
)
install(FILES ${PROJECT_BINARY_DIR}/lib/fast/fast.py
    DESTINATION fast/python/fast
)
install(FILES ${PROJECT_BINARY_DIR}/lib/fast/__init__.py
		DESTINATION fast/python/fast
		)
endif()

# Copy configuration file
# Create new configuration file for install

set(CONFIG_KERNEL_SOURCE_PATH "KernelSourcePath = @ROOT@/kernels/")
set(CONFIG_KERNEL_BINARY_PATH "KernelBinaryPath = @ROOT@/kernel_binaries/")
set(CONFIG_DOCUMENTATION_PATH "DocumentationPath = @ROOT@/doc/")
set(CONFIG_PIPELINE_PATH "PipelinePath = @ROOT@/pipelines/")
set(CONFIG_TEST_DATA_PATH "TestDataPath = @ROOT@/data/")
configure_file(
    "${PROJECT_SOURCE_DIR}/source/fast_configuration.txt.in"
    "${PROJECT_BINARY_DIR}/fast_configuration_install.txt"
)
install(FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
    RENAME fast_configuration.txt
    DESTINATION fast/
)

# Install FAST license file
install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION fast/licenses/fast/
)
# Install README
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/README_default.md
    DESTINATION fast/
    RENAME README.md
)

# Install license files for depedencies
# Qt5
file(GLOB LICENSE_FILES ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/LICENSE.*)
install(FILES ${LICENSE_FILES}
		DESTINATION fast/licenses/qt5/
)

# Eigen
file(GLOB LICENSE_FILES ${FAST_EXTERNAL_BUILD_DIR}/eigen/src/eigen/COPYING.*)
install(FILES ${LICENSE_FILES}
		DESTINATION fast/licenses/eigen/
)
# zlib
install(FILES ${FAST_EXTERNAL_BUILD_DIR}/zlib/src/zlib/README
		DESTINATION fast/licenses/zlib/
)
# OpenIGTLink
install(FILES ${FAST_EXTERNAL_BUILD_DIR}/OpenIGTLink/src/OpenIGTLink/LICENSE.txt
		DESTINATION fast/licenses/OpenIGTLink/
)
# DCMTK
install(FILES ${FAST_EXTERNAL_BUILD_DIR}/dcmtk/src/dcmtk/COPYRIGHT
		DESTINATION fast/licenses/dcmtk/
)
# NumPy (numpy.i file)
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/NumPy_LICENSE.txt
		DESTINATION fast/licenses/numpy/
)
# Semaphore implementation
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/Semaphore_LICENSE.txt
		DESTINATION fast/licenses/semaphore/
)

# Tensorflow license
if(FAST_MODULE_TensorFlow)
	install(FILES ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/LICENSE
        DESTINATION fast/licenses/tensorflow/
    )
endif()
if(FAST_MODULE_OpenVINO AND NOT WIN32)
	install(FILES ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/LICENSE
		DESTINATION fast/licenses/openvino/
	)
endif()

if(FAST_MODULE_RealSense)
	install(FILES
        ${FAST_EXTERNAL_BUILD_DIR}/realsense/src/realsense/LICENSE
        ${FAST_EXTERNAL_BUILD_DIR}/realsense/src/realsense/NOTICE
        DESTINATION fast/licenses/realsense/
    )
endif()

if(FAST_BUILD_DOCS)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION fast/doc
    )
endif()
