# Build Python bindings (requires SWIG installed)

if(FAST_MODULE_Python)
    find_package(SWIG REQUIRED)
    message("-- SWIG found, creating python bindings...")
    include(${SWIG_USE_FILE})

    if(FAST_Python_Version)
        find_package(PythonLibs ${FAST_Python_Version} EXACT REQUIRED)
    else()
        find_package(PythonLibs REQUIRED)
    endif()
    include_directories(${PYTHON_INCLUDE_DIRS})

    find_package(NumPy REQUIRED)
    include_directories(${PYTHON_NUMPY_INCLUDE_DIR})

    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    set(CMAKE_SWIG_FLAGS "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSWIG_PYTHON_INTERPRETER_NO_DEBUG") # Avoid a error on windows when compiling in debug mode

    # Generate the PyFAST interface file
    # Include all header files
    foreach(FILE ${FAST_SOURCE_FILES})
        if(${FILE} MATCHES "^.*hpp$")
            set(PYFAST_HEADER_INCLUDES "${PYFAST_HEADER_INCLUDES}#include \"${FILE}\"\n")
        endif()
    endforeach()

    # Include all python interface files
    foreach(FILE ${FAST_PYTHON_INTERFACE_FILES})
        set(PYFAST_INTERFACE_INCLUDES "${PYFAST_INTERFACE_INCLUDES}%include \"${FILE}\"\n")
    endforeach()

    set(PYFAST_FILE "${PROJECT_BINARY_DIR}/PyFAST.i")
    configure_file(
            "${PROJECT_SOURCE_DIR}/source/PyFAST.i.in"
            ${PYFAST_FILE}
    )

    # Build it
    set_source_files_properties(${PYFAST_FILE} PROPERTIES GENERATED TRUE)
    set_source_files_properties(${PYFAST_FILE} PROPERTIES CPLUSPLUS ON)
    set(CMAKE_SWIG_OUTDIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/fast/)
    file(MAKE_DIRECTORY ${CMAKE_SWIG_OUTDIR})
    swig_add_module(fast python ${PYFAST_FILE})
    swig_link_libraries(fast ${PYTHON_LIBRARIES} FAST)
    set_target_properties(_fast PROPERTIES INSTALL_RPATH "$ORIGIN/../../lib")

    configure_file(
            "${PROJECT_SOURCE_DIR}/source/__init__.py.in"
            ${CMAKE_SWIG_OUTDIR}__init__.py
    )
else()
    message("-- Python module not enabled in CMake, Python bindings will NOT be created.")
endif()
