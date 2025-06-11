# Get MCSS and specifix version of doxygen:
include(${TOP_SOURCE_DIR}/cmake/ExternalMCSS.cmake)
include(${TOP_SOURCE_DIR}/cmake/ExternalDoxygen.cmake)

find_package(PythonInterp 3 REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documentation/)

set(doxyfile_in ${TOP_SOURCE_DIR}/doc/Doxyfile.in)
set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/documentation/Doxyfile)

configure_file(${doxyfile_in} ${doxyfile} @ONLY)
configure_file(${TOP_SOURCE_DIR}/doc/conf.py ${CMAKE_CURRENT_BINARY_DIR}/documentation/conf.py COPYONLY)

add_custom_target(documentation
	COMMAND ${PYTHON_EXECUTABLE} ${FAST_EXTERNAL_BUILD_DIR}/mcss/src/mcss/documentation/doxygen.py ${CMAKE_CURRENT_BINARY_DIR}/documentation/conf.py --doxygen-executable ${FAST_EXTERNAL_BUILD_DIR}/doxygen/install/bin/doxygen
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/documentation/
    COMMENT "Generating API documentation with Doxygen and M.CSS"
    DEPENDS mcss doxygen
    VERBATIM
)
