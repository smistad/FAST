# This will install FAST binaries, libraries and necessary include files to the path given by CMAKE_INSTALL_PREFIX

# Install FAST library
install(TARGETS FAST
	DESTINATION lib
)

# Install test executable
install(TARGETS testFAST
	DESTINATION bin
)

# Examples are installed in the macro fast_add_example

# Install dependency libraries
if(WIN32)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION bin
	)
else()
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.so*)
	install(FILES ${SOs}
		DESTINATION lib
	)
endif()

# Install headers
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION include/FAST/
	FILES_MATCHING PATTERN "*.hpp"
)
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION include/FAST/
	FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/source/CL/
	DESTINATION include/CL/
	FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/source/CL/
	DESTINATION include/CL/
	FILES_MATCHING PATTERN "*.hpp"
)

# Install OpenCL kernels
install(DIRECTORY ${FAST_SOURCE_DIR}
	DESTINATION kernels/FAST/
	FILES_MATCHING PATTERN "*.cl"
)

# Install CMake files
install(FILES ${PROJECT_BINARY_DIR}/FASTConfig.cmake
    DESTINATION cmake
)

# Install Python wrapper
install(TARGETS _fast
    DESTINATION python
)
install(FILES ${PROJECT_BINARY_DIR}/lib/fast.py
    DESTINATION python
)

# Copy configuration file
install(FILES ${PROJECT_BINARY_DIR}/fast_configuration.txt
    DESTINATION bin
)
