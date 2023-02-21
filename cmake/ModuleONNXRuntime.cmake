if(FAST_MODULE_ONNXRuntime)
    message("-- Enabling Microsoft ONNX runtime inference engine module")
    if(WIN32)
        fast_download_dependency(onnxruntime
              1.13.1
              07ab6a1e875fbd336fd694cf71728535d935bf1a87c80cb1307e04758bce87d9
        )
    elseif(APPLE)
        if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            fast_download_dependency(onnxruntime
                    1.14.0
                    06f80a2c3c36f7008594abd9651d0edf7903d82165d450350abebae37db2f4d0
            )
        else()
            fast_download_dependency(onnxruntime
                1.14.0
            )
        endif()
    else()
    endif()
endif()
