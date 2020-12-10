# Download and set up Tensorflow

include(${PROJECT_SOURCE_DIR}/cmake/Externals.cmake)

ExternalProject_Add(tensorflow_download
    PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
    BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
    GIT_REPOSITORY "https://github.com/smistad/tensorflow.git"
    GIT_TAG "066a509e6a14687cf1e59a1fd0e1b346071271ce"
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

if(WIN32)
    set(GIT_EXECUTABLE "git.exe")
    # Use CMake to build tensorflow on windows
    file(TO_NATIVE_PATH ${FAST_EXTERNAL_BUILD_DIR} FAST_EXTERNAL_BUILD_DIR_WIN)
    file(TO_NATIVE_PATH ${FAST_EXTERNAL_INSTALL_DIR} FAST_EXTERNAL_INSTALL_DIR_WIN)
    if(FAST_BUILD_TensorFlow_CUDA)
        set(CONFIGURE_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCUDA.bat)
        set(BUILD_COMMAND echo "Building tensorflow with bazel and CUDA GPU support" &&
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ &&
                bazel build --config=cuda --jobs=4 //tensorflow:tensorflow_cc.dll
        )
    else(FAST_BUILD_TensorFlow_CUDA)
        set(CONFIGURE_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCPU.bat)
        set(BUILD_COMMAND echo "Building tensorflow with bazel for CPU only" &&
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ &&
                bazel build --config=opt --jobs=4 //tensorflow:tensorflow_cc.dll
        )
    endif()
    ExternalProject_Add(tensorflow
            DEPENDS tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            CONFIGURE_COMMAND
                echo "Configuring TensorFlow..." COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ COMMAND
                ${CONFIGURE_SCRIPT} COMMAND
                echo "Done TF configure"
            BUILD_COMMAND
                ${BUILD_COMMAND}
            INSTALL_COMMAND
                echo "Installing tensorflow binary"  COMMAND
                ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/tensorflow_cc.dll.if.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/tensorflow_cc.lib COMMAND
                #${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/external/protobuf_archive/protobuf.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/protobuf.lib COMMAND
                ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/tensorflow_cc.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/tensorflow_cc.dll COMMAND
                echo "Installing tensorflow headers"  COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\tensorflow\\*.h" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\tensorflow\\" /syi COMMAND
                echo "Installing tensorflow generated headers" COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\bazel-bin\\tensorflow\\*.h" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\tensorflow\\" /syi COMMAND
                echo "Installing tensorflow third party headers"  COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\third_party\\" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\third_party\\" /syi  COMMAND
                echo "Installing protobuf headers"  COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\bazel-tensorflow_download\\external\\com_google_protobuf\\src\\google\\*.h" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\google\\" /syi COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\bazel-tensorflow_download\\external\\com_google_protobuf\\src\\google\\*.inc" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\google\\" /syi COMMAND
                echo "Installing absl headers"  COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\bazel-tensorflow_download\\external\\com_google_absl\\absl\\*.h" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\absl\\" /syi COMMAND
                xcopy "${FAST_EXTERNAL_BUILD_DIR_WIN}\\tensorflow\\src\\tensorflow_download\\bazel-tensorflow_download\\external\\com_google_absl\\absl\\*.inc" "${FAST_EXTERNAL_INSTALL_DIR_WIN}\\include\\absl\\" /syi
    )
else(WIN32)
    # Use bazel to build tensorflow on linux
    set(GIT_EXECUTABLE "git")
    if(FAST_BUILD_TensorFlow_CUDA)
        set(CONFIGURE_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCUDA.sh)
        set(BUILD_COMMAND echo "Building tensorflow with bazel and CUDA GPU support" &&
            cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ &&
            bazel build -c opt --config=cuda --copt=-mfpmath=both --copt=-march=core-avx2 --jobs=4 //tensorflow:libtensorflow_cc.so
        )
    else(FAST_BUILD_TensorFlow_CUDA)
        set(CONFIGURE_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/TensorflowConfigureCPU.sh)
        set(BUILD_COMMAND echo "Building tensorflow with bazel for CPU only" &&
            cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ &&
            bazel build -c opt --config=opt --copt=-mfpmath=both --copt=-march=core-avx2 --jobs=4 //tensorflow:libtensorflow_cc.so
        )
    endif()
    ExternalProject_Add(tensorflow
            DEPENDS tensorflow_download
            PREFIX ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/tensorflow
            DOWNLOAD_COMMAND ""
            UPDATE_COMMAND ""
            # Run TF configure in the form of a shell script. CUDA should be installed in /usr/local/cuda
            CONFIGURE_COMMAND
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/ && sh ${CONFIGURE_SCRIPT}
            # Build using bazel
            BUILD_COMMAND
                ${BUILD_COMMAND}
            INSTALL_COMMAND
                echo "Installing tensorflow binary" &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_cc.so.2.3.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/ &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_cc.so.2 ${FAST_EXTERNAL_INSTALL_DIR}/lib/ &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_framework.so.2.3.0 ${FAST_EXTERNAL_INSTALL_DIR}/lib/ &&
                cp -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/tensorflow/libtensorflow_framework.so.2 ${FAST_EXTERNAL_INSTALL_DIR}/lib/ &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc.so.2.3.0 &&
                chmod a+w ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework.so.2.3.0 &&
                strip -s ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_cc.so.2.3.0 &&
                strip -s ${FAST_EXTERNAL_INSTALL_DIR}/lib/libtensorflow_framework.so.2.3.0 &&
                echo "Installing tensorflow headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/tensorflow/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing tensorflow generated headers" &&
                cd ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-bin/ &&
                bash -c "find tensorflow/ -name '*.h' | xargs cp -f --parents -t ${FAST_EXTERNAL_INSTALL_DIR}/include/" &&
                echo "Installing tensorflow third_party headers" &&
                cp -rf ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/third_party/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ &&
                echo "Installing protobuf headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/com_google_protobuf/src/google/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf" &&
                echo "Installing absl headers" &&
                bash -c "cp $(readlink -f ${FAST_EXTERNAL_BUILD_DIR}/tensorflow/src/tensorflow_download/bazel-out/)/../../../external/com_google_absl/absl/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ -Rf"
    )
endif(WIN32)
