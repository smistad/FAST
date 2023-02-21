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
    find_package(NumPy REQUIRED)

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

    set(PYFAST_FILE "${PROJECT_BINARY_DIR}/PyFAST.i")
    configure_file(
            "${PROJECT_SOURCE_DIR}/source/FAST/Python/PyFAST.i.in"
            ${PYFAST_FILE}
    )

    # Build it
    set_source_files_properties(${PYFAST_FILE} PROPERTIES GENERATED TRUE)
    set_source_files_properties(${PYFAST_FILE} PROPERTIES CPLUSPLUS ON)
    set_property(SOURCE ${PYFAST_FILE} PROPERTY SWIG_MODULE_NAME fast)
    set(CMAKE_SWIG_OUTDIR ${PROJECT_BINARY_DIR}/python/fast/)
    file(MAKE_DIRECTORY ${CMAKE_SWIG_OUTDIR})
    swig_add_library(fast LANGUAGE python SOURCES ${PYFAST_FILE})
    if(WIN32)
        get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)
        target_link_directories(_fast PRIVATE ${PYTHON_LIBRARY_DIR})
        swig_link_libraries(fast python3 FAST)
    else()
        swig_link_libraries(fast ${PYTHON_LIBRARIES} FAST)
    endif()
    set_property(TARGET _fast PROPERTY SWIG_COMPILE_OPTIONS -py3 -doxygen -py3-stable-abi -keyword -threads) # Enable Python 3 specific features and doxygen comment translation in SWIG
    set_target_properties(_fast PROPERTIES INSTALL_RPATH "$ORIGIN/../lib")
    target_include_directories(_fast PRIVATE ${PYTHON_NUMPY_INCLUDE_DIR})
    target_include_directories(_fast PRIVATE ${PYTHON_INCLUDE_DIRS})
    target_include_directories(_fast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # Trigger install operation
    add_custom_target(install_to_wheel
        COMMAND ${CMAKE_COMMAND}
        -D CMAKE_INSTALL_PREFIX:STRING=${PROJECT_BINARY_DIR}/python/
        -P ${PROJECT_BINARY_DIR}/cmake_install.cmake
    )
    add_dependencies(install_to_wheel _fast)
    message("PYTHON LIBRARIES: ${PYTHON_LIBRARIES}")

    if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
        set(OSX_DEPLOYMENT_TARGET "11.0")
        set(OSX_ARCHITECTURE "arm64")
    else()
        set(OSX_DEPLOYMENT_TARGET "10.13")
        set(OSX_ARCHITECTURE "x86_64")
    endif()

    add_custom_target(python-wheel
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/source/FAST/Python/__init__.py ${PROJECT_BINARY_DIR}/python/fast/
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/source/FAST/Python/entry_points.py ${PROJECT_BINARY_DIR}/python/fast/
    # Remove this lib file as we don't need it: the setup script will build a new one
    COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE:_fast> ${PROJECT_BINARY_DIR}/_unused_fast_python_lib
    COMMAND ${CMAKE_COMMAND}
        -D FAST_VERSION=${FAST_VERSION}
        -D FAST_SOURCE_DIR:STRING=${PROJECT_SOURCE_DIR}
        -D FAST_BINARY_DIR:STRING=${PROJECT_BINARY_DIR}
        -D PYTHON_VERSION:STRING=${PYTHONLIBS_VERSION_STRING}
        -D PYTHON_EXECUTABLE:STRING=${PYTHON_EXECUTABLE}
        -D PYTHON_LIBRARIES:STRING=${PYTHON_LIBRARIES}
        -D NUMPY_INCLUDE_DIR:STRING=${PYTHON_NUMPY_INCLUDE_DIR}
        -D OpenCL_LIBRARIES:STRING=${OpenCL_LIBRARIES}
        -D OPENGL_LIBRARIES:STRING=${OPENGL_LIBRARIES}
        -D OSX_DEPLOYMENT_TARGET:STRING=${OSX_DEPLOYMENT_TARGET}
        -D OSX_ARCHITECTURE:STRING=${OSX_ARCHITECTURE}
        -P "${PROJECT_SOURCE_DIR}/cmake/PythonWheel.cmake")
    add_dependencies(python-wheel install_to_wheel)
else()
    message("-- Python module not enabled in CMake, Python bindings will NOT be created.")
endif()
