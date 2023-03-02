if(FAST_MODULE_ONNXRuntime)
    message("-- Enabling Microsoft ONNX runtime inference engine module")
    if(WIN32)
        fast_download_dependency(onnxruntime
              1.14.0
              28d0477bbc21efa87ea1c44c5f3b8b29bf3e272b22921126981d7ecdfac2b998
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
                22fdcae7e380e064e3c75ec8c585d11d05bc24983f08cf30a810914500bfeb9d
            )
        endif()
    else()
    endif()
endif()
