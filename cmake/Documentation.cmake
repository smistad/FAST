find_package(Doxygen REQUIRED)
find_package(PythonInterp 3 REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documentation/)

set(doxyfile_in ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in)
set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/documentation/Doxyfile)

configure_file(${doxyfile_in} ${doxyfile} @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/doc/conf.py ${CMAKE_CURRENT_BINARY_DIR}/documentation/conf.py COPYONLY)

add_custom_target(documentation
    COMMAND ${PYTHON_EXECUTABLE} ${FAST_EXTERNAL_BUILD_DIR}/mcss/src/mcss/documentation/doxygen.py ${CMAKE_CURRENT_BINARY_DIR}/documentation/conf.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documentation/
    COMMENT "Generating API documentation with Doxygen and M.CSS"
    DEPENDS mcss
    VERBATIM
)
