# OpenIGTLink module

if(FAST_MODULE_OpenIGTLink)
    set(LIBRARY_OUTPUT_PATH  "${FAST_BINARY_DIR}") # Needed to output the libraries in the correct folder
    set(EXECUTABLE_OUTPUT_PATH "${FAST_BINARY_DIR}") # Needed to output the executables in correct folder
    add_subdirectory(source/OpenIGTLink)
    find_package(OpenIGTLink PATHS "${FAST_BINARY_DIR}/source/OpenIGTLink" REQUIRED)
    include(${OpenIGTLink_USE_FILE})
    list(APPEND FAST_INCLUDE_DIRS ${OpenIGTLink_INCLUDE_DIRS})
    list(APPEND LIBRARIES ${OpenIGTLink_LIBRARIES})

    ### IGT Link tests
    if(BUILD_IGTLINK_TESTS)
        set(BUILD_TESTING ON CACHE INTERNAL "Build OpenIGTLink tests." FORCE)
    else()
        set(BUILD_TESTING OFF CACHE INTERNAL "Build OpenIGTLink tests." FORCE)
    endif()
endif()
