# This will install FAST binaries, libraries and necessary include files to the path given by CMAKE_INSTALL_PREFIX

# Install FAST library
if(WIN32)
# DLL should be in binary folder
install(TARGETS FAST
	DESTINATION fast/bin
	COMPONENT fast
)
else()
install(TARGETS FAST
	DESTINATION fast/lib
	COMPONENT fast
)
endif()

if(FAST_BUILD_TESTS)
    # Install test executable
    install(TARGETS testFAST
        DESTINATION fast/bin
		COMPONENT fast
    )
endif()

# Examples are installed in the macro fast_add_example

# Install dependency libraries
install(FILES ${PROJECT_BINARY_DIR}/FASTExport.hpp
    DESTINATION fast/include
	COMPONENT fast
)
if(WIN32)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION fast/bin
		COMPONENT fast
	)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/lib/*.lib)
	install(FILES ${DLLs}
		DESTINATION fast/lib
		COMPONENT fast
	)
elseif(APPLE)
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.dylib)
	install(FILES ${SOs}
        DESTINATION fast/lib
		COMPONENT fast
    )
else()
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.so*)
	install(FILES ${SOs}
        DESTINATION fast/lib
		COMPONENT fast
    )
	# Fix RPaths on install
    install(SCRIPT cmake/FixRPaths.cmake)
endif()

# Install Qt plugins
install(DIRECTORY ${PROJECT_BINARY_DIR}/plugins/
    DESTINATION fast/plugins/
	COMPONENT fast
)

# Install qt moc
install(FILES ${PROJECT_BINARY_DIR}/bin/moc${CMAKE_EXECUTABLE_SUFFIX}
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
    DESTINATION fast/bin
	COMPONENT fast
)
if(WIN32)
		# Install qt idc https://doc.qt.io/qt-5/activeqt-idc.html
		install(FILES ${PROJECT_BINARY_DIR}/bin/idc.exe
		    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
		    DESTINATION fast/bin
		)
endif()

# Install headers
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/include/FAST/
	COMPONENT fast
	FILES_MATCHING PATTERN "*.hpp"
)
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/include/FAST/
	COMPONENT fast
	FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/source/CL/
	DESTINATION fast/include/CL/
	COMPONENT fast
	FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/source/CL/
	DESTINATION fast/include/CL/
	COMPONENT fast
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
		QtSvg
    QtTest
    QtThemeSupport
    QtWidgets
    QtXml
    QtZlib)
if(FAST_MODULE_Plotting)
	list(APPEND INCLUDE_FOLDERS jkqtplotter jkqtcommon jkqtfastplotter jkqtmathtext)
endif()
if(WIN32)
list(APPEND INCLUDE_FOLDERS
    ActiveQt
)
else()
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
		COMPONENT fast
        FILES_MATCHING PATTERN "*.h"
    )
    install(DIRECTORY ${PROJECT_BINARY_DIR}/include/${INCLUDE_FOLDER}/
        DESTINATION fast/include/${INCLUDE_FOLDER}/
		COMPONENT fast
        FILES_MATCHING PATTERN "*.hpp"
    )
    install(DIRECTORY ${PROJECT_BINARY_DIR}/include/${INCLUDE_FOLDER}/
        DESTINATION fast/include/${INCLUDE_FOLDER}/
		COMPONENT fast
        FILES_MATCHING REGEX "/[^.]+$" # Files with no extension
    )
endforeach()



# Install created headers
install(FILES ${PROJECT_BINARY_DIR}/ProcessObjectList.hpp
    DESTINATION fast/include/FAST/
	COMPONENT fast
)

# Install OpenCL kernels
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	COMPONENT fast
	FILES_MATCHING PATTERN "*.cl"
)

# Install GL shaders
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	COMPONENT fast
	FILES_MATCHING PATTERN "*.vert"
)

# Install GL shaders
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	COMPONENT fast
	FILES_MATCHING PATTERN "*.frag"
)

# Install CMake files
install(FILES ${PROJECT_BINARY_DIR}/FASTConfig.cmake ${PROJECT_BINARY_DIR}/FASTUse.cmake
    DESTINATION fast/cmake
	COMPONENT fast
)
install(FILES ${PROJECT_SOURCE_DIR}/cmake/FindOpenCL.cmake
    DESTINATION fast/cmake
	COMPONENT fast
)

# Install docs
install(DIRECTORY ${PROJECT_SOURCE_DIR}/doc/
    DESTINATION fast/doc/
	COMPONENT fast
)

# Install pipelines
install(DIRECTORY ${PROJECT_SOURCE_DIR}/pipelines/
    DESTINATION fast/pipelines/
	COMPONENT fast
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
if(WIN32)
		set(CONFIG_LIBRARY_PATH "LibraryPath = @ROOT@/bin/")
else()
		set(CONFIG_LIBRARY_PATH "LibraryPath = @ROOT@/lib/")
endif()

set(CONFIG_QT_PLUGINS_PATH "QtPluginsPath = @ROOT@/plugins/")
configure_file(
    "${PROJECT_SOURCE_DIR}/source/fast_configuration.txt.in"
    "${PROJECT_BINARY_DIR}/fast_configuration_install.txt"
)
install(FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
    RENAME fast_configuration.txt
    DESTINATION fast/bin/
	COMPONENT fast
)

# Install FAST license file
install(FILES ${PROJECT_SOURCE_DIR}/LICENSE
    DESTINATION fast/licenses/fast/
	COMPONENT fast
)
# Install README
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/README_default.md
    DESTINATION fast/
    RENAME README.md
	COMPONENT fast
)

# Install license files for depedencies
# Qt5
file(GLOB LICENSE_FILES ${FAST_EXTERNAL_BUILD_DIR}/qt5/src/qt5/LICENSE.*)
install(FILES ${LICENSE_FILES}
		DESTINATION fast/licenses/qt5/
		COMPONENT fast
)

# Eigen
file(GLOB LICENSE_FILES ${FAST_EXTERNAL_BUILD_DIR}/eigen/src/eigen/COPYING.*)
install(FILES ${LICENSE_FILES}
		DESTINATION fast/licenses/eigen/
		COMPONENT fast
)
# zlib
install(FILES ${FAST_EXTERNAL_BUILD_DIR}/zlib/src/zlib/README
		DESTINATION fast/licenses/zlib/
		COMPONENT fast
)
# OpenIGTLink
if(FAST_MODULE_OpenIGTLink)
install(FILES ${FAST_EXTERNAL_BUILD_DIR}/OpenIGTLink/src/OpenIGTLink/LICENSE.txt
		DESTINATION fast/licenses/OpenIGTLink/
		COMPONENT fast
)
endif()
# DCMTK
if(FAST_MODULE_Dicom)
install(FILES ${FAST_EXTERNAL_BUILD_DIR}/dcmtk/src/dcmtk/COPYRIGHT
		DESTINATION fast/licenses/dcmtk/
		COMPONENT fast
)
endif()
# NumPy (numpy.i file)
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/NumPy_LICENSE.txt
		DESTINATION fast/licenses/numpy/
		COMPONENT fast
)
# Semaphore implementation
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/Semaphore_LICENSE.txt
		DESTINATION fast/licenses/semaphore/
		COMPONENT fast
)

# Tensorflow license
if(FAST_MODULE_TensorFlow)
	install(FILES ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/LICENSE
        DESTINATION fast/licenses/tensorflow/
		COMPONENT fast
    )
endif()
if(FAST_MODULE_OpenVINO)
	install(FILES ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/LICENSE
		DESTINATION fast/licenses/openvino/
		COMPONENT fast
	)
	if(WIN32)
		install(FILES ${PROJECT_BINARY_DIR}/bin/plugins.xml
		  DESTINATION fast/bin/
			COMPONENT fast
	  )
	else()
		install(FILES ${PROJECT_BINARY_DIR}/lib/plugins.xml
			DESTINATION fast/lib/
			COMPONENT fast
		)
	endif()
endif()

if(FAST_MODULE_RealSense)
	install(FILES
        ${FAST_EXTERNAL_BUILD_DIR}/realsense/src/realsense/LICENSE
        ${FAST_EXTERNAL_BUILD_DIR}/realsense/src/realsense/NOTICE
        DESTINATION fast/licenses/realsense/
		COMPONENT fast
    )
endif()

if(FAST_MODULE_WholeSlideImaging AND WIN32)
    # Install openslide and related licences
    install(DIRECTORY
        ${FAST_EXTERNAL_BUILD_DIR}/openslide/src/openslide/licenses/
        DESTINATION fast/licenses/
		COMPONENT fast
    )
endif()

if(FAST_MODULE_HDF5)
	install(FILES
		${FAST_EXTERNAL_BUILD_DIR}/hdf5/src/hdf5/COPYING
		DESTINATION fast/licenses/hdf5/
		COMPONENT fast
	)
endif()

if(FAST_MODULE_Plotting)
		install(FILES
				${FAST_EXTERNAL_BUILD_DIR}/jkqtplotter/src/jkqtplotter/LICENSE
				DESTINATION fast/licenses/jkqtplotter/
				COMPONENT fast
		)
endif()

if(FAST_BUILD_DOCS)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION fast/doc
		COMPONENT fast
    )
endif()

# Create empty kernel_binaries folder for caching
install(DIRECTORY
	DESTINATION fast/kernel_binaries
	DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE OWNER_WRITE GROUP_READ GROUP_EXECUTE GROUP_WRITE WORLD_READ WORLD_WRITE WORLD_EXECUTE
	COMPONENT fast
)
