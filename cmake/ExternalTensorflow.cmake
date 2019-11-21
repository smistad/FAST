# Download and set up Tensorflow

include(${PROJECT_SOURCE_DIR}/cmake/Externals.cmake)

if(WIN32)
    set(GIT_EXECUTABLE "git.exe")
    # Use CMake to build tensorflow on windows
    if(FAST_BUILD_TensorFlow_CPU OR FAST_BUILD_TensorFlow_CUDA)
        ExternalProject_Add(tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            GIT_REPOSITORY "https://github.com/smistad/tensorflow.git"
            GIT_TAG "fast-updated"
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
        )
    endif()
    if(FAST_BUILD_TensorFlow_CPU)

    ExternalProject_Add(tensorflow_CPU
            DEPENDS tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND
                echo "Configuring TensorFlow..." COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ COMMAND
                ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCPU.bat COMMAND
                echo "Done TF configure"
            BUILD_COMMAND
                echo "Building tensorflow with bazel for CPU.." COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ COMMAND
                bazel build --config=opt //tensorflow:tensorflow_cc.dll
            INSTALL_COMMAND
                echo "Installing tensorflow binary"  COMMAND
                ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/tensorflow_cc.dll.if.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/tensorflow_CPU.lib COMMAND
                ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/external/protobuf_archive/protobuf.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/protobuf.lib COMMAND
                ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/tensorflow_cc.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/tensorflow_cc.dll COMMAND
                echo "Installing tensorflow headers"  COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/tensorflow ${FAST_EXTERNAL_INSTALL_DIR}/include/tensorflow/ COMMAND
                echo "Installing tensorflow generated headers" COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-genfiles/tensorflow ${FAST_EXTERNAL_INSTALL_DIR}/include/tensorflow/  COMMAND
                echo "Installing tensorflow third party headers"  COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/third_party/ ${FAST_EXTERNAL_INSTALL_DIR}/include/third_party/  COMMAND
                echo "Installing protobuf headers"  COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-tensorflow_download/external/protobuf_archive/src/google/ ${FAST_EXTERNAL_INSTALL_DIR}/include/google/ COMMAND
                #echo "Installing nsync headers"  COMMAND
                #xcopy ${src} ${dest} /y COMMAND
                echo "Installing absl headers"  COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-tensorflow_download/external/com_google_absl/absl/ ${FAST_EXTERNAL_INSTALL_DIR}/include/absl/
    )
    endif()
else(WIN32)
    # Use bazel to build tensorflow on linux
    set(GIT_EXECUTABLE "git")
    if(FAST_BUILD_TensorFlow_CPU OR FAST_BUILD_TensorFlow_CUDA)
        ExternalProject_Add(tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            GIT_REPOSITORY "https://github.com/smistad/tensorflow.git"
            GIT_TAG "fast-updated"
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
        )
    endif()
    if(FAST_BUILD_TensorFlow_ROCm)
        # Need a seperate repo for rocm atm
        ExternalProject_Add(tensorflow_download_rocm
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm
            GIT_REPOSITORY "https://github.com/ROCmSoftwarePlatform/tensorflow-upstream"
            GIT_TAG "r1.14-rocm"
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
        )
        ExternalProject_Add(tensorflow_ROCm
            DEPENDS tensorflow_download_rocm
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            # Run TF configure in the form of a shell script.
            CONFIGURE_COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/ && sh ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureROCm.sh
            # Build using bazel
            BUILD_COMMAND
                echo "Building tensorflow with bazel and ROCm (AMD) GPU support" &&
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/ && bazel build -c opt --config=rocm //tensorflow:libtensorflow_cc.so
            INSTALL_COMMAND
                echo "Installing tensorflow binary" &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/bazel-bin/tensorflow/libtensorflow_cc.so.1.14.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_ROCm.so &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/bazel-bin/tensorflow/libtensorflow_framework.so.1.14.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_ROCm.so &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_ROCm.so &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_ROCm.so &&
                patchelf --set-soname libtensorflow_cc_ROCm.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_ROCm.so &&
                patchelf --set-soname libtensorflow_framework_ROCm.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_ROCm.so &&
                patchelf --replace-needed libtensorflow_framework.so.1 libtensorflow_framework_ROCm.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_ROCm.so &&
                echo "Installing tensorflow headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow generated headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/bazel-genfiles/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow third_party headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/third_party/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing protobuf headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/bazel-out/)/../../../external/protobuf_archive/src/google/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                #echo "Installing nsync headers" &&
                #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/bazel-out/)/../../../external/nsync/public/*.h ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                echo "Installing absl headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow_rocm/src/tensorflow_download_rocm/bazel-out/)/../../../external/com_google_absl/absl/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf"
    )
    endif()
    if(FAST_BUILD_TensorFlow_CUDA)
    ExternalProject_Add(tensorflow_CUDA
            DEPENDS tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            # Run TF configure in the form of a shell script. CUDA should be installed in /usr/local/cuda
            CONFIGURE_COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ && sh ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCUDA.sh
            # Build using bazel
            BUILD_COMMAND
                echo "Building tensorflow with bazel and CUDA GPU support" &&
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ && bazel build -c opt --config=cuda --copt=-mfpmath=both --copt=-march=core-avx2 //tensorflow:libtensorflow_cc.so
            INSTALL_COMMAND
                echo "Installing tensorflow binary" &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_cc.so.1.14.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CUDA.so &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_framework.so.1.14.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_CUDA.so &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CUDA.so &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_CUDA.so &&
                patchelf --set-soname libtensorflow_cc_CUDA.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CUDA.so &&
                patchelf --set-soname libtensorflow_framework_CUDA.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_CUDA.so &&
                patchelf --replace-needed libtensorflow_framework.so.1 libtensorflow_framework_CUDA.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CUDA.so &&
                #echo "Installing mkl binaries" &&
                #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/mkl/lib/libmklml_intel.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ -Rf" &&
                #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/mkl/lib/libiomp5.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ -Rf" &&
                echo "Installing tensorflow headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow generated headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-genfiles/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow third_party headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/third_party/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing protobuf headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/protobuf_archive/src/google/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                #echo "Installing nsync headers" &&
                #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/nsync/public/*.h ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                echo "Installing absl headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/com_google_absl/absl/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf"
    )
    endif()
    if(FAST_BUILD_TensorFlow_CPU)
    ExternalProject_Add(tensorflow_CPU
            DEPENDS tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            # Run TF configure in the form of a shell script.
            CONFIGURE_COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ && sh ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCPU.sh
            # Build using bazel
            BUILD_COMMAND
                echo "Building tensorflow with bazel for CPU" &&
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ && bazel build --config=opt --copt=-mfpmath=both --copt=-march=core-avx2 //tensorflow:libtensorflow_cc.so
            INSTALL_COMMAND
                echo "Installing tensorflow binary" &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_cc.so.1.14.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CPU.so &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_framework.so.1.14.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_CPU.so &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CPU.so &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_CPU.so &&
                patchelf --set-soname libtensorflow_cc_CPU.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CPU.so &&
                patchelf --set-soname libtensorflow_framework_CPU.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework_CPU.so &&
                patchelf --replace-needed libtensorflow_framework.so.1 libtensorflow_framework_CPU.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc_CPU.so &&
                echo "Installing tensorflow headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow generated headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-genfiles/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow third_party headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/third_party/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing protobuf headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/protobuf_archive/src/google/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                #echo "Installing nsync headers" &&
                #bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/nsync/public/*.h ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                echo "Installing absl headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/com_google_absl/absl/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf"
    )
    endif()
endif(WIN32)
