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
    swig_link_libraries(fast ${PYTHON_LIBRARIES} FAST)
    set_target_properties(_fast PROPERTIES INSTALL_RPATH "$ORIGIN/../../lib")
    target_include_directories(_fast PRIVATE ${PYTHON_NUMPY_INCLUDE_DIR})
    target_include_directories(_fast PRIVATE ${PYTHON_INCLUDE_DIRS})
    target_include_directories(_fast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    if(WIN32)
        add_custom_command(TARGET _fast POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:_fast> ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif()
    configure_file(
            "${PROJECT_SOURCE_DIR}/source/FAST/Python/__init__.py.in"
            ${CMAKE_SWIG_OUTDIR}__init__.py
    )
else()
    message("-- Python module not enabled in CMake, Python bindings will NOT be created.")
endif()
