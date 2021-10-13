message(STATUS "Creating setup.py for python ${PYTHON_VERSION} wheel...")

configure_file(${FAST_SOURCE_DIR}/source/FAST/Python/setup.py.in ${FAST_BINARY_DIR}/python/setup.py @ONLY IMMEDIATE)

execute_process(COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel --py-limited-api cp36 WORKING_DIRECTORY "${FAST_BINARY_DIR}/python/")
