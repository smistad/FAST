#------------------------------------------------------------------------------
# OpenCLUtilityLibrary_LIBRARIES variable
#------------------------------------------------------------------------------
set (OpenCLUtilityLibrary_LIBRARIES ${OpenCLUtilityLibrary_LIBRARY} ${TestOpenCLUtilityLibrary_LIBRARY})

#------------------------------------------------------------------------------
# Where to look for includes and libraries
#------------------------------------------------------------------------------
include_directories( ${OpenCLUtilityLibrary_INCLUDE_DIRS})
link_directories (${OpenCLUtilityLibrary_LIBRARY_DIRS})