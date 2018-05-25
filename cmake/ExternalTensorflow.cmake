# Download and set up Tensorflow

include(cmake/Externals.cmake)

if(WIN32)
    set(GIT_EXECUTABLE "git.exe")
    # Use CMake to build tensorflow on windows
    ExternalProject_Add(tensorflow
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            GIT_REPOSITORY "https://github.com/smistad/tensorflow.git"
            GIT_TAG "fast"
            # Need to override this because tensorflow creates a file in the source dir
            # and cmake files to stash these files
            UPDATE_COMMAND
                ${GIT_EXECUTABLE} pull origin fast
            # Must use CONFIGURE_COMMAND because CMakeLists.txt is not in the src root of tensorflow
            CONFIGURE_COMMAND
                ${CMAKE_COMMAND}
                ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/tensorflow/contrib/cmake/
                -G${CMAKE_GENERATOR}
		-Thost=x64
                -DCMAKE_BUILD_TYPE:STRING=Release
                -Dtensorflow_BUILD_PYTHON_BINDINGS=OFF
                -Dtensorflow_ENABLE_SNAPPY_SUPPORT=ON
                -Dtensorflow_BUILD_CC_EXAMPLE=OFF
                -Dtensorflow_BUILD_SHARED_LIB=ON
                -Dtensorflow_BUILD_CONTRIB_KERNELS=OFF
                -Dtensorflow_ENABLE_GRPC_SUPPORT=OFF
                -Dtensorflow_WIN_CPU_SIMD_OPTIONS=/arch:AVX
                #-Dtensorflow_ENABLE_GPU=ON
                -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
                -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
                -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        )
    list(APPEND LIBRARIES
        tensorflow.lib
        #libprotobuf.lib # For windows we need this protobuf static lib for some reason..
    )
else(WIN32)
    # Use bazel to build tensorflow on linux
    set(GIT_EXECUTABLE "git")
    ExternalProject_Add(tensorflow
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
        GIT_REPOSITORY "https://github.com/smistad/tensorflow.git"
        GIT_TAG "fast"
        UPDATE_COMMAND ""
        # Run TF configure in the form of a shell script. CUDA should be installed in /usr/local/cuda
        CONFIGURE_COMMAND
            cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/ && sh ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigure.sh
        # Build using bazel
        BUILD_COMMAND
            echo "Building tensorflow with bazel and CUDA GPU support" &&
            cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/ && bazel build -c opt --config=cuda --copt=-mfpmath=both --copt=-march=native //tensorflow:libtensorflow_framework.so
        INSTALL_COMMAND
            echo "Installing tensorflow binary" &&
            cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/bazel-bin/tensorflow/libtensorflow_framework.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ &&
            #echo "Installing mkl binaries" &&
            #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/bazel-out/)/../../../external/mkl/lib/libmklml_intel.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ -Rf" &&
            #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/bazel-out/)/../../../external/mkl/lib/libiomp5.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ -Rf" &&
            echo "Installing tensorflow headers" &&
            cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
            echo "Installing tensorflow generated headers" &&
            cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/bazel-genfiles/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
            echo "Installing tensorflow third_party headers" &&
            cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/third_party/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
            echo "Installing protobuf headers" &&
            bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/bazel-out/)/../../../external/protobuf_archive/src/google/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
            echo "Installing nsync headers" &&
            bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow/bazel-out/)/../../../external/nsync/public/*.h ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf"
    )
    list(APPEND LIBRARIES
        libtensorflow_framework.so
    )
endif(WIN32)

list(APPEND FAST_EXTERNAL_DEPENDENCIES tensorflow)

