# Build Python bindings (requires SWIG installed)

if(FAST_MODULE_Python)
    # Download and set up swig for building python bindings
    # Do it at configure time:
    include(cmake/FetchSwig.cmake)

    set(SWIG_EXECUTABLE ${swig_SOURCE_DIR}/bin/swig)
    set(SWIG_DIR ${swig_SOURCE_DIR}/swig-lib/)
    find_package(SWIG 4 REQUIRED)
    message("-- SWIG found, creating python bindings...")
    include(${SWIG_USE_FILE})

    if(FAST_Python_Version)
        find_package(PythonInterp ${FAST_Python_Version} EXACT REQUIRED)
        if(FAST_Python_Library AND FAST_Python_Include)
            set(PYTHON_LIBRARIES ${FAST_Python_Library})
            set(PYTHON_INCLUDE_DIRS ${FAST_Python_Include})
            message("Manually set Python library and include to: ${PYTHON_LIBRARIES} ${PYTHON_INCLUDE_DIRS}")
        else()
            find_package(PythonLibs ${FAST_Python_Version} EXACT REQUIRED)
        endif()
    else()
        find_package(PythonInterp 3 REQUIRED)
        find_package(PythonLibs 3 REQUIRED)
    endif()

    set(CMAKE_SWIG_FLAGS "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWIG_PYTHON_INTERPRETER_NO_DEBUG") # Avoid a error on windows when compiling in debug mode

    # Generate the PyFAST interface file
    # Include all header files
    list(REMOVE_DUPLICATES FAST_PYTHON_HEADER_FILES)
    foreach(FILE ${FAST_PYTHON_HEADER_FILES})
        if(${FILE} MATCHES "^.*hpp$")
            set(PYFAST_HEADER_INCLUDES "${PYFAST_HEADER_INCLUDES}#include <${FILE}>\n")
        endif()
    endforeach()

    # Create shared_ptr defines
    list(REMOVE_DUPLICATES FAST_PYTHON_SHARED_PTR_OBJECTS)
    foreach(OBJECT ${FAST_PYTHON_SHARED_PTR_OBJECTS})
        set(PYFAST_SHARED_PTR_DEFS "${PYFAST_SHARED_PTR_DEFS}%shared_ptr(fast::${OBJECT})\n")
    endforeach()

    # Include all python interface files
    foreach(FILE ${FAST_PYTHON_HEADER_FILES})
        set(PYFAST_INTERFACE_INCLUDES "${PYFAST_INTERFACE_INCLUDES}%include <${FILE}>\n")
    endforeach()

    set(PYFAST_SOURCES Core.i ProcessObjects.i)
    foreach(SRC ${PYFAST_SOURCES})
        set(PYFAST_FILE "${PROJECT_BINARY_DIR}/${SRC}")
        configure_file(
                "${PROJECT_SOURCE_DIR}/source/FAST/Python/${SRC}.in"
                ${PYFAST_FILE}
        )

        set_source_files_properties(${PYFAST_FILE} PROPERTIES GENERATED TRUE)
        set_source_files_properties(${PYFAST_FILE} PROPERTIES CPLUSPLUS ON)
        set_source_files_properties(${PYFAST_FILE} PROPERTIES USE_TARGET_INCLUDE_DIRECTORIES ON)
        if(NOT ${SRC} STREQUAL Common.i)
            list(APPEND PYFAST_CONFIGURED_SOURCES ${PYFAST_FILE})
        endif()
    endforeach()
    # Build it
    set(CMAKE_SWIG_OUTDIR ${PROJECT_BINARY_DIR}/python/fast/)
    file(MAKE_DIRECTORY ${CMAKE_SWIG_OUTDIR})
    swig_add_library(fast LANGUAGE python SOURCES ${PROJECT_BINARY_DIR}/Core.i)
    # Use python limited api 3.6
    target_compile_definitions(_fast PRIVATE Py_LIMITED_API=0x03060000)
    if(WIN32)
        set(OUTPUT_FOLDER bin)
        # On windows, to use ABI3, it will link against python3.dll which is located in the python library dir
        get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)
        target_link_directories(_fast PRIVATE ${PYTHON_LIBRARY_DIR})
        swig_link_libraries(fast FAST)
    else()
        set(OUTPUT_FOLDER lib)
        swig_link_libraries(fast FAST)
        set_target_properties(_fast PROPERTIES SUFFIX ".abi3.so")
        set_target_properties(_fast PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
	# To use ABI3 on unix systems we will not link with the python library. Instead we tell the linker that undefined
	# symbols should be ignored for now, and will be supplied during runtime instead:
        if(APPLE)
            target_link_options(_fast PRIVATE 
                "LINKER:-bundle"
                "LINKER:-undefined" "LINKER:dynamic_lookup"
            )
        else()
            target_link_options(_fast PRIVATE 
                "LINKER:-Bsymbolic-functions"
                )
        endif()
    endif()
    set_property(TARGET _fast PROPERTY SWIG_COMPILE_OPTIONS -py3 -doxygen -py3-stable-abi -keyword -threads) # Enable Python 3 specific features and doxygen comment translation in SWIG
    set_target_properties(_fast PROPERTIES EXCLUDE_FROM_ALL TRUE)
    target_include_directories(_fast PRIVATE ${PYTHON_INCLUDE_DIRS})
    target_include_directories(_fast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # Trigger install operation
    add_custom_target(install_to_wheel
        COMMAND ${CMAKE_COMMAND}
        -D CMAKE_INSTALL_PREFIX:STRING=${PROJECT_BINARY_DIR}/python/
        -D CMAKE_INSTALL_COMPONENT:STRING=fast
        -P ${PROJECT_BINARY_DIR}/cmake_install.cmake
    )
    add_dependencies(install_to_wheel FAST)
    message("PYTHON LIBRARIES: ${PYTHON_LIBRARIES}")

    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(OSX_DEPLOYMENT_TARGET "11.0")
        set(OSX_ARCHITECTURE "arm64")
    else()
        set(OSX_DEPLOYMENT_TARGET "10.13")
        set(OSX_ARCHITECTURE "x86_64")
    endif()

    add_custom_target(python-wheel
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/source/FAST/Python/__init__fast.py ${PROJECT_BINARY_DIR}/python/fast/__init__.py
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/source/FAST/Python/entry_points.py ${PROJECT_BINARY_DIR}/python/fast/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_fast> ${PROJECT_BINARY_DIR}/python/fast/${OUTPUT_FOLDER}/
        COMMAND ${CMAKE_COMMAND}
            -D FAST_VERSION=${FAST_VERSION}
            -D FAST_SOURCE_DIR:STRING=${PROJECT_SOURCE_DIR}
            -D FAST_BINARY_DIR:STRING=${PROJECT_BINARY_DIR}
            -D PYTHON_VERSION:STRING=${PYTHONLIBS_VERSION_STRING}
            -D PYTHON_EXECUTABLE:STRING=${PYTHON_EXECUTABLE}
            -D OSX_DEPLOYMENT_TARGET:STRING=${OSX_DEPLOYMENT_TARGET}
            -D OSX_ARCHITECTURE:STRING=${OSX_ARCHITECTURE}
            -P "${PROJECT_SOURCE_DIR}/cmake/PythonWheel.cmake")

    add_dependencies(python-wheel install_to_wheel _fast)
else()
    message("-- Python module not enabled in CMake, Python bindings will NOT be created.")
endif()
