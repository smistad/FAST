# Download and build OpenVINO

include(${PROJECT_SOURCE_DIR}/cmake/Externals.cmake)

if(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
ExternalProject_Add(OpenVINO
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO
        GIT_REPOSITORY "https://github.com/openvinotoolkit/openvino.git"
        GIT_TAG "2021.1"
        UPDATE_COMMAND
            git submodule update --init --recursive
        CMAKE_ARGS
            -DENABLE_BEH_TESTS=OFF
            -DENABLE_C=OFF
            -DENABLE_FUNCTIONAL_TESTS=OFF
            -DENABLE_OPENCV:BOOL=OFF
            -DENABLE_PROFILING_ITT:BOOL=OFF
            -DENABLE_SAMPLES:BOOL=OFF
            -DENABLE_CPPCHECK:BOOL=OFF
            -DENABLE_CPPLINT:BOOL=OFF
            -DBUILD_TESTING:BOOL=OFF
            #-DBUILD_SHARED_LIBS:BOOL=ON
            -DENABLE_VPU:BOOL=ON
	    -DNGRAPH_UNIT_TEST_ENABLE:BOOL=OFF
	    -DNGRAPH_UNIT_TEST_OPENVINO_ENABLE:BOOL=OFF
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        BUILD_COMMAND
            ${CMAKE_COMMAND} --build . --config Release --target inference_engine COMMAND
            ${CMAKE_COMMAND} --build . --config Release --target clDNNPlugin COMMAND
            ${CMAKE_COMMAND} --build . --config Release --target myriadPlugin COMMAND
            ${CMAKE_COMMAND} --build . --config Release --target MKLDNNPlugin
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_legacy.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_legacy.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_transformations.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_transformations.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_lp_transformations.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_lp_transformations.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_ir_reader.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_ir_v7_reader.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/inference_engine_onnx_reader.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/onnx_importer.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/clDNNPlugin.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/MKLDNNPlugin.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/myriadPlugin.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/ngraph.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/ngraph.lib ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/plugins.xml ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/temp/tbb/bin/tbb.dll ${FAST_EXTERNAL_INSTALL_DIR}/bin/ COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/include/ ${FAST_EXTERNAL_INSTALL_DIR}/include/openvino/ COMMAND
			${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/ngraph/core/include/ngraph/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ngraph/
		)
else()
ExternalProject_Add(OpenVINO
        PREFIX ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO
        GIT_REPOSITORY "https://github.com/openvinotoolkit/openvino.git"
        GIT_TAG "2021.1"
        UPDATE_COMMAND
            git submodule update --init --recursive
        CMAKE_ARGS
            -DENABLE_BEH_TESTS=OFF
            -DENABLE_C=OFF
            -DENABLE_FUNCTIONAL_TESTS=OFF
            -DENABLE_OPENCV:BOOL=OFF
            -DENABLE_PROFILING_ITT:BOOL=OFF
            -DENABLE_SAMPLES:BOOL=OFF
            -DENABLE_CPPCHECK:BOOL=OFF
            -DENABLE_CPPLINT:BOOL=OFF
            -DBUILD_TESTING:BOOL=OFF
            #-DBUILD_SHARED_LIBS:BOOL=ON
            -DENABLE_VPU:BOOL=ON
	    -DNGRAPH_UNIT_TEST_ENABLE:BOOL=OFF
	    -DNGRAPH_UNIT_TEST_OPENVINO_ENABLE:BOOL=OFF
	    -DTREAT_WARNING_AS_ERROR:BOOL=OFF
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
            -DCMAKE_INSTALL_MESSAGE:BOOL=LAZY
            -DCMAKE_INSTALL_PREFIX:STRING=${FAST_EXTERNAL_INSTALL_DIR}
        BUILD_COMMAND
            make -j4 inference_engine clDNNPlugin myriadPlugin MKLDNNPlugin
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine_legacy.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine_transformations.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine_lp_transformations.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine_ir_reader.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine_ir_v7_reader.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libinference_engine_onnx_reader.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libonnx_importer.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
			${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libclDNNPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libMKLDNNPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libmyriadPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/libngraph.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/bin/intel64/Release/lib/plugins.xml ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            #${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libGNAPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            #${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/bin/intel64/Release/lib/libHeteroPlugin.so ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/temp/tbb/lib/libtbb.so.2 ${FAST_EXTERNAL_INSTALL_DIR}/lib/ COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/inference-engine/include/ ${FAST_EXTERNAL_INSTALL_DIR}/include/openvino/ COMMAND
			${CMAKE_COMMAND} -E copy_directory ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO/src/OpenVINO/ngraph/core/include/ngraph/ ${FAST_EXTERNAL_INSTALL_DIR}/include/ngraph/
		)
endif()
else(FAST_BUILD_ALL_DEPENDENCIES)
if(WIN32)
  set(FILENAME windows/openvino_2021.1_msvc14.2.tar.xz)
  set(SHA 3526f46aa835a6aca389052c6c43876210f06dcdf9454aac6dfcb72f26c4cc76)
else()
  set(FILENAME linux/openvino_2021.1_glibc2.27.tar.xz)
  set(SHA fdd445522248eeaf4d19a9ac4de84885d77dac9ba6ec365a73c2cf38458ab2d4)
endif()
	ExternalProject_Add(OpenVINO
		PREFIX ${FAST_EXTERNAL_BUILD_DIR}/OpenVINO
		URL ${FAST_PREBUILT_DEPENDENCY_DOWNLOAD_URL}/${FILENAME}
		URL_HASH SHA256=${SHA}
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		# On install: Copy contents of each subfolder to the build folder
		INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${FAST_EXTERNAL_INSTALL_DIR}/include COMMAND
						${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin ${FAST_EXTERNAL_INSTALL_DIR}/bin COMMAND
						${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib ${FAST_EXTERNAL_INSTALL_DIR}/lib COMMAND
						${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/licences ${FAST_EXTERNAL_INSTALL_DIR}/licences
	)
endif(FAST_BUILD_ALL_DEPENDENCIES)

list(APPEND FAST_INCLUDE_DIRS ${FAST_EXTERNAL_INSTALL_DIR}/include/openvino/)
