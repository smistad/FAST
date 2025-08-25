if(FAST_MODULE_ONNXRuntime)
    message("-- Enabling Microsoft ONNX runtime inference engine module")
    if(WIN32)
        fast_download_dependency(onnxruntime
              1.22.1
              7236099edc47df10aaac418d4775e7e1f5681f39cdc1d335d825a2ac653f796e
        )
    elseif(APPLE)
        if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
            fast_download_dependency(onnxruntime
                    1.22.0
                    934c7b1d25729dea29045ce98ff1055696daa44c769343ffced4cad3d35768d5
            )
        else()
            fast_download_dependency(onnxruntime
                1.14.0
                22fdcae7e380e064e3c75ec8c585d11d05bc24983f08cf30a810914500bfeb9d
            )
        endif()
    else()
        fast_download_dependency(onnxruntime
                1.22.0
                e0f9d12a52373606c213b2c2785c9bf7425b1fc7cf9cc4102840b402e1bc720f
        )
    endif()
endif()
