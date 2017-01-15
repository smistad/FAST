# This will install FAST binaries, libraries and necessary include files to the path given by CMAKE_INSTALL_PREFIX

# Install FAST library
install(TARGETS FAST
	DESTINATION lib
)

# Install test executable
install(TARGETS testFAST
	DESTINATION bin
)

# TODO Install examples
install(TARGETS ${FAST_EXAMPLES}
	DESTINATION bin
)

# Install dependency libraries
if(WIN32)
	file(GLOB DLLs ${PROJECT_BINARY_DIR}/bin/*.dll)
	install(FILES ${DLLs}
		DESTINATION bin
	)
else()
	file(GLOB SOs ${PROJECT_BINARY_DIR}/lib/*.so)
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

# TODO Install CMake files
