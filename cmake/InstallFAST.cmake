# This will install FAST binaries, libraries and necessary include files to the path given by CMAKE_INSTALL_PREFIX

# Install FAST library
install(TARGETS FAST
	DESTINATION fast/lib
)

# Install test executable
install(TARGETS testFAST
	DESTINATION fast/bin
)

# Examples are installed in the macro fast_add_example

# Install dependency libraries
if(WIN32)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION fast/bin
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
install(FILES ${PROJECT_SOURCE_DIR}/cmake/qt.conf
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

# Install OpenCL kernels
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION fast/kernels/
	FILES_MATCHING PATTERN "*.cl"
)

# Install CMake files
install(FILES ${PROJECT_BINARY_DIR}/FASTConfig.cmake
    DESTINATION fast/cmake
)

# Install Python wrapper
if(FAST_MODULE_Python)
install(TARGETS _fast
    DESTINATION fast/python
)
install(FILES ${PROJECT_BINARY_DIR}/lib/fast.py
    DESTINATION fast/python
)
endif()

# Copy configuration file
# Create new configuration file for install

set(CONFIG_KERNEL_SOURCE_PATH "KernelSourcePath = @ROOT@/kernels/")
set(CONFIG_KERNEL_BINARY_PATH "KernelBinaryPath = @ROOT@/kernel_binaries/")
set(CONFIG_KERNEL_TEST_PATH "")
configure_file(
    "${PROJECT_SOURCE_DIR}/source/fast_configuration.txt.in"
    "${PROJECT_BINARY_DIR}/fast_configuration_install.txt"
)
install(FILES ${PROJECT_BINARY_DIR}/fast_configuration_install.txt
    RENAME fast_configuration.txt
    DESTINATION fast/
)

