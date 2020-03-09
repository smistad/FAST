## FAST Use cmake file

# Enable C++ 17
set(CMAKE_CXX_STANDARD 17)

# Position independent code
if(${CMAKE_COMPILER_IS_GNUCXX})
    add_definitions("-fPIC")
endif()

include_directories(${FAST_INCLUDE_DIRS})
link_directories(${FAST_LIBRARY_DIRS})

# Qt MOC setup
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        PROPERTY Qt5Core_VERSION_MAJOR "5")
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        PROPERTY Qt5Core_VERSION_MINOR "14")

add_executable(Qt5::moc IMPORTED)
set_target_properties(Qt5::moc PROPERTIES IMPORTED_LOCATION "${FAST_BINARY_DIR}moc${CMAKE_EXECUTABLE_SUFFIX}")
set(CMAKE_AUTOMOC TRUE)

if(WIN32)
    # Copy all DLLs from FAST to current binary folder
    file(GLOB DLLs ${FAST_BINARY_DIR}*.dll)

    if(EXISTS ${FAST_BINARY_DIR}/plugins.xml) # plugins.xml file is needed by OpenVINO
        add_custom_target(fast_copy
            COMMAND ${CMAKE_COMMAND} -E copy ${DLLs} ${PROJECT_BINARY_DIR}/Release/ COMMAND ${CMAKE_COMMAND} -E echo "FAST DLLs copied (Release)."
            COMMAND ${CMAKE_COMMAND} -E copy ${DLLs} ${PROJECT_BINARY_DIR}/Debug/ COMMAND ${CMAKE_COMMAND} -E echo "FAST DLLs copied (Debug)."
            COMMAND ${CMAKE_COMMAND} -E copy ${FAST_BINARY_DIR}/plugins.xml ${PROJECT_BINARY_DIR}/Release/ COMMAND ${CMAKE_COMMAND} -E echo "OpenVINO plugins.xml copied (Release)."
            COMMAND ${CMAKE_COMMAND} -E copy ${FAST_BINARY_DIR}/plugins.xml ${PROJECT_BINARY_DIR}/Debug/ COMMAND ${CMAKE_COMMAND} -E echo "OpenVINO plugins.xml copied (Debug)."
        )
    else()
        add_custom_target(fast_copy
            COMMAND ${CMAKE_COMMAND} -E copy ${DLLs} ${PROJECT_BINARY_DIR}/Release/ COMMAND ${CMAKE_COMMAND} -E echo "FAST DLLs copied (Release)."
            COMMAND ${CMAKE_COMMAND} -E copy ${DLLs} ${PROJECT_BINARY_DIR}/Debug/ COMMAND ${CMAKE_COMMAND} -E echo "FAST DLLs copied (Debug)."
        )
    endif()

    # Create fast config file
    set(FILE_CONTENT "TestDataPath = ${FAST_BINARY_DIR}/data/
    KernelSourcePath = ${FAST_BINARY_DIR}../kernels/
    KernelBinaryPath = ${FAST_BINARY_DIR}../kernel_binaries/
    DocumentationPath = ${FAST_BINARY_DIR}../doc/
    PipelinePath = ${FAST_BINARY_DIR}../pipelines/
    LibraryPath = ${FAST_BINARY_DIR}../bin/
    QtPluginsPath = ${FAST_BINARY_DIR}../plugins/")
    file(WRITE ${PROJECT_BINARY_DIR}/Release/fast_configuration.txt ${FILE_CONTENT})
    file(WRITE ${PROJECT_BINARY_DIR}/Debug/fast_configuration.txt ${FILE_CONTENT})
else()
    # NoOp command
    add_custom_target(fast_copy COMMENT "noop")
endif()
