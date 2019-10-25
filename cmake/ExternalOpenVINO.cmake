# Download and build OpenVINO

include(${PROJECT_SOURCE_DIR}/cmake/Externals.cmake)

if(WIN32)
else()
ExternalProject_Add(OpenVINO
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO
        BINARY_DIR ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/build
        GIT_REPOSITORY "https://github.com/opencv/dldt.git"
        GIT_TAG "2019_R1.1"
        SOURCE_SUBDIR inference-engine
        UPDATE_COMMAND
            git submodule init COMMAND git submodule update --recursive COMMAND mkdir -p inference-engine/build/
        CMAKE_ARGS
            -DENABLE_OPENCV:BOOL=OFF
            -DENABLE_OBJECT_DETECTION_TESTS:BOOL=OFF
            -DENABLE_PROFILING_ITT:BOOL=OFF
            -DENABLE_SAMPLES:BOOL=OFF
            -DENABLE_SAMPLES_CORE:BOOL=OFF
            -DENABLE_SEGMENTATION_TESTS:BOOL=OFF
            -DENABLE_CPPCHECK:BOOL=OFF
            -DENABLE_CPPLINT:BOOL=OFF
            -DBUILD_TESTING:BOOL=OFF
            #-DBUILD_SHARED_LIBS:BOOL=ON
            -DENABLE_VPU:BOOL=ON
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        BUILD_COMMAND
            make -j4 inference_engine clDNNPlugin myriadPlugin MKLDNNPlugin ie_cpu_extension
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libinference_engine.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libclDNN64.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libclDNNPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libMKLDNNPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libmyriadPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libcpu_extension.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            #${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libGNAPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            #${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libHeteroPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/temp/tbb/lib/libtbb.so.2 ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/include/ ${FAST_EXTERNAL_INSTALL_DIR}/include/openvino/

        )
endif()

list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openvino/)
