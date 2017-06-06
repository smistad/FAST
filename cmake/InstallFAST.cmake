# This will install FAST binaries, libraries and necessary include files to the path given by CMAKE_INSTALL_PREFIX

# Install FAST library
install(TARGETS FAST
	DESTINATION fast/lib
)

if(FAST_BUILD_TESTS)
    # Install test executable
    install(TARGETS testFAST
        DESTINATION fast/bin
    )
endif()

# Examples are installed in the macro fast_add_example

# Install dependency libraries
if(WIN32)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION fast/bin
	)
	install(FILES ${PROJECT_BINARY_DIR}/FASTExport.hpp
		DESTINATION fast/include
	)
else()
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.so*)
	install(FILES ${SOs}
		DESTINATION fast/lib
	)
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
install(DIRECTORY ${PROJECT_BINARY_DIR}/include/
    DESTINATION fast/include/
)


# Install created headers
install(FILES ${PROJECT_BINARY_DIR}/ProcessObjectList.hpp
    DESTINATION fast/include/FAST/
)

# Install OpenCL kernels
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	FILES_MATCHING PATTERN "*.cl"
)

# Install CMake files
install(FILES ${PROJECT_BINARY_DIR}/FASTConfig.cmake ${PROJECT_BINARY_DIR}/FASTUse.cmake
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
if(FAST_MODULE_NeuralNetwork)
	install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/README_neural_network.md
        DESTINATION fast/
        RENAME README.md
    )
else()
    install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/README_default.md
        DESTINATION fast/
        RENAME README.md
    )
endif()

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

# NumPy (numpy.i file)
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/NumPy_LICENSE.txt
		DESTINATION fast/licenses/numpy/
)

# Semaphore implementation
install(FILES ${PROJECT_SOURCE_DIR}/cmake/InstallFiles/Semaphore_LICENSE.txt
		DESTINATION fast/licenses/semaphore/
)

if(FAST_MODULE_NeuralNetwork)
    # Tensorflow license
	install(FILES ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/LICENSE
        DESTINATION fast/licenses/tensorflow/
    )
endif()

if(FAST_BUILD_DOCS)
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION fast/doc
    )
endif()