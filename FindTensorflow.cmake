# Locates the tensorflow library and include directories.
include(FindPackageHandleStandardArgs)
unset(TENSORFLOW_FOUND)

find_path(Tensorflow_INCLUDE_DIR
        NAMES
        tensorflow/core
        tensorflow/cc
        third_party
        HINTS
        /usr/local/include/tensorflow
        /usr/include/tensorflow)

find_library(Tensorflow_LIBRARY NAMES tensorflow
        HINTS
        /usr/lib
        /usr/local/lib)

# set TENSORFLOW_FOUND
find_package_handle_standard_args(Tensorflow DEFAULT_MSG Tensorflow_INCLUDE_DIR Tensorflow_LIBRARY)

if(TENSORFLOW_FOUND)
    message("-- Tensorflow found. Include dir: ${Tensorflow_INCLUDE_DIR} Library: ${Tensorflow_LIBRARY}")
endif()
