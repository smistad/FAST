cmake_minimum_required(VERSION 3.11)
project(FAST-Documentation)

set(RELATIVE_TOP_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../)
get_filename_component(TOP_SOURCE_DIR "${RELATIVE_TOP_SOURCE_DIR}" ABSOLUTE)
include(${TOP_SOURCE_DIR}/cmake/Documentation.cmake)
