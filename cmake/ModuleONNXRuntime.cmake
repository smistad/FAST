if(FAST_MODULE_ONNXRuntime AND WIN32)
    message("-- Enabling Microsoft ONNX runtime inference engine module")
    fast_download_dependency(onnxruntime
          1.13.1
          ca1a7890391a641a283eb0de18280094f275a9ce77c39745b31484d70107fc47
    )
endif()
