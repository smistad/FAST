if(FAST_MODULE_ONNXRuntime AND WIN32)
    message("-- Enabling Microsoft ONNX runtime inference engine module")
    fast_download_dependency(onnxruntime
          1.13.1
          07ab6a1e875fbd336fd694cf71728535d935bf1a87c80cb1307e04758bce87d9
    )
endif()
