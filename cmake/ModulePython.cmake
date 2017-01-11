# Build Python bindings if module is enabled and SWIG is found

find_package(SWIG)
if(SWIG_FOUND AND FAST_MODULE_Python)
    message("-- SWIG found, creating python bindings...")
    include(${SWIG_USE_FILE})

    find_package(PythonLibs REQUIRED)
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
    swig_add_module(fast python ${PYFAST_FILE})
    swig_link_libraries(fast ${PYTHON_LIBRARIES} FAST)

    # Move python file to lib folder
    add_custom_command(TARGET _fast POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rename fast.py ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/fast.py
            )
else()
    message("-- SWIG not found or Python module not enabled in CMake, Python bindings will NOT be created.")
endif()
